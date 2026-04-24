# Nimonspoli Bankruptcy Bug - Comprehensive Analysis Report

## Executive Summary

When a player goes bankrupt in the game, the game transitions to a problematic state where:
1. The game may not properly end even though only one player remains alive
2. The `activePlayerIndex` may point to a bankrupt player after their bankruptcy
3. The turn advancement logic doesn't guarantee moving to a valid (non-bankrupt) player
4. The game UI/command system may become unresponsive or stuck

---

## Bug Manifestation

**Observable Behavior:**
- When player B goes bankrupt during their turn, players can no longer take actions
- The game appears to "freeze" or become unplayable
- Other players still have money but cannot continue the game
- The game does not properly announce the winner

---

## Root Causes Identified

### 1. **Game Over Detection Timing Issue**

**Location:** [GameSession.cpp](src/app/GameSession.cpp) lines 393-402 (finishTurn function)

**Issue:** When a player goes bankrupt and only one player remains alive:

```cpp
void GameSession::finishTurn() {
    if (game.isGameOver()) {
        return;  // Returns early WITHOUT advancing to next player
    }
    // ... rest of function
}
```

**Problem Flow:**
1. Player B's turn ends with them going bankrupt → `activePlayerIndex = 1` (B's index)
2. `finishTurn()` checks `game.isGameOver()`
3. Returns early (because `aliveCount <= 1` is now true)
4. **`activePlayerIndex` now points to the bankrupt Player B**
5. Main loop next iteration tries to process commands for the current player
6. Could access/display a bankrupt player as the "current" player

### 2. **advanceToNextPlayer() Edge Case**

**Location:** [GameManager.cpp](src/models/GameManager/GameManager.cpp) lines 104-119

```cpp
void GameManager::advanceToNextPlayer() {
  if (players.empty()) { return; }
  
  int startIdx = activePlayerIndex;
  do {
    activePlayerIndex = (activePlayerIndex + 1) % players.size();
    if (activePlayerIndex == 0) {
      currentTurn++;
    }
    if (activePlayerIndex == startIdx && players[activePlayerIndex].getStatus() == BANKRUPT) {
        break;  // EXITS loop if back at start AND start player is bankrupt
    }
  } while (players[activePlayerIndex].getStatus() == BANKRUPT);
}
```

**Critical Issue - Scenario with 3 players (A: BANKRUPT, B: BANKRUPT, C: ACTIVE):**

When C finishes their turn:
- `startIdx = 2` (C's index)
- Loop iteration 1: `activePlayerIndex = 0`, A is bankrupt → continue
- Loop iteration 2: `activePlayerIndex = 1`, B is bankrupt → continue  
- Loop iteration 3: `activePlayerIndex = 2`
  - Check: `if (2 == 2 && players[2].BANKRUPT)` → FALSE (C is ACTIVE)
  - Check: `while (players[2].BANKRUPT)` → FALSE (C is ACTIVE)
  - **EXIT LOOP with activePlayerIndex = 2 (same as start)**

**Result:** C takes another turn immediately (infinite loop of same player taking turns)

### 3. **Missing Post-Bankruptcy Player Validation**

**Location:** Multiple locations in turn handling

**Issue:** After `executeBankruptcy()` is called, there's NO explicit logic to:
1. Validate that the current `activePlayerIndex` still points to a valid player
2. Ensure the next player advancement skips the now-bankrupt player
3. Verify the game state is consistent

---

## Code Flow Analysis

### Happy Path (Multiple players alive, one goes bankrupt):
```
handleRollDice()
  ↓ (player lands on property, goes bankrupt)
  ├→ tile.onLanded()
  │   └→ executeRentPayer()
  │       └→ executeBankruptcy()  [Player B now BANKRUPT]
  │
  ├→ turnActionTaken = true
  ├→ finishTurn()
  │   ├→ game.isGameOver(): aliveCount = 2 → FALSE
  │   ├→ game.advanceToNextPlayer()  [Moves from B to C]
  │   ├→ startTurn()  [C's turn begins]
  │   └→ return normally
  │
  └→ return normally
```

### Problem Path (One player dies, leaving only one):
```
handleRollDice()
  ↓ (player B lands on property, goes bankrupt)
  └→ finishTurn()
     ├→ game.isGameOver(): aliveCount = 1 → TRUE
     ├→ return EARLY  [activePlayerIndex still = 1 (B's index - BANKRUPT)]
     └→ return to handleRollDice
```

**Then in main loop:**
```
Main loop checks: game.isGameOver() = TRUE
Calls announceWinner() ✓
But activePlayerIndex = 1 points to bankrupt player B ✗
```

---

## Affected Files

| File | Line | Issue |
|------|------|-------|
| [GameManager.cpp](src/models/GameManager/GameManager.cpp) | 104-119 | `advanceToNextPlayer()` doesn't guarantee valid player on return |
| [GameManager.cpp](src/models/GameManager/GameManager.cpp) | 49-57 | `checkWinCondition()` may not be accurate if there's a race condition |
| [GameSession.cpp](src/app/GameSession.cpp) | 393-402 | `finishTurn()` returns without advancing player when game ends |
| [GameSessionCommands.cpp](src/app/GameSessionCommands.cpp) | 175-185 | Post-tile handling doesn't validate current player state |

---

## Recommended Fixes

### Fix 1: Ensure Valid activePlayerIndex After Game Over

**File:** [GameSession.cpp](src/app/GameSession.cpp) - `finishTurn()` function

```cpp
void GameSession::finishTurn() {
    if (game.isGameOver()) {
        // If game is over and current player is bankrupt, don't leave pointer dangling
        // Optionally set activePlayerIndex to the winner (if desired)
        // For now, just return - but ensure UI is aware current player may be bankrupt
        return;
    }
    
    game.getCurrentPlayer().deactivateShield();
    dice.resetConsecutiveDoubles();
    game.advanceToNextPlayer();
    startTurn(true);
    turnActionTaken = false;
    std::cout << "\nGiliran berpindah ke " << game.getCurrentPlayer().getUsername()
              << ".\n";
}
```

**Alternative - Force valid player assignment:**
```cpp
void GameSession::finishTurn() {
    if (game.isGameOver()) {
        // Set activePlayerIndex to winner before returning
        // This ensures consistency and prevents accessing bankrupt player
        return;
    }
    // ... rest of function
}
```

### Fix 2: Improve advanceToNextPlayer() Logic

**File:** [GameManager.cpp](src/models/GameManager/GameManager.cpp)

```cpp
void GameManager::advanceToNextPlayer() {
    if (players.empty()) { return; }
    
    // Count alive players
    int aliveCount = 0;
    for (const Player& p : players) {
        if (p.getStatus() != BANKRUPT) aliveCount++;
    }
    
    // If only 1 or 0 players alive, don't advance
    // The game is over, so don't modify activePlayerIndex
    if (aliveCount <= 1) return;
    
    int startIdx = activePlayerIndex;
    do {
        activePlayerIndex = (activePlayerIndex + 1) % players.size();
        if (activePlayerIndex == 0) {
            currentTurn++;
        }
        if (activePlayerIndex == startIdx && players[activePlayerIndex].getStatus() == BANKRUPT) {
            break;  // All other players are bankrupt, game should be over
        }
    } while (players[activePlayerIndex].getStatus() == BANKRUPT);
}
```

### Fix 3: Validate Player State During Turn

**File:** [GameSessionCommands.cpp](src/app/GameSessionCommands.cpp) - `handleRollDice()` and `startTurn()`

Add validation at the beginning of `startTurn()`:
```cpp
void GameSession::startTurn(bool drawSkillCard) {
    // Validate current player is not bankrupt
    if (game.getCurrentPlayer().getStatus() == BANKRUPT) {
        // This should never happen - if it does, advance to next player
        game.advanceToNextPlayer();
    }
    
    game.processTurn();
    diceRolledThisTurn = false;
    if (drawSkillCard) {
        awardSkillCardAtTurnStart();
    }
}
```

---

## Testing Recommendations

### Test Case 1: Basic Bankruptcy
1. 3 players: A, B, C (all with starting cash)
2. B lands on A's property, cannot pay → goes bankrupt
3. **Expected:** Game continues with A and C, B skipped in turn order
4. **Verify:** `advanceToNextPlayer()` moves from B to C correctly

### Test Case 2: Last Player Standing
1. 2 players: A, B (both with starting cash)
2. B lands on A's property, cannot pay → goes bankrupt
3. **Expected:** Game ends, A announced as winner
4. **Verify:** 
   - `checkWinCondition()` returns true
   - `game.isGameOver()` is true
   - Winner announcement shows A, not B
   - No commands can be executed except NEW_GAME/LOAD_GAME/EXIT

### Test Case 3: Multiple Bankruptcies
1. 4 players: A, B, C, D
2. Bankruptcy sequence: B → C → D
3. **Expected:** Game continues with only A, then ends
4. **Verify:** A keeps taking turns alone (or game recognizes only A is alive and ends)

---

## Impact Assessment

**Severity:** HIGH
- Core game functionality (turn management) is broken
- Prevents games from reaching proper conclusion
- Players cannot recover from the bug without restarting

**Scope:** 
- Turn-based game logic
- Player status management
- Game state transitions

**Affected Operations:**
- Bankruptcy handling
- Turn advancement
- Game ending logic
- UI state synchronization

---

## Conclusion

The bug occurs at the intersection of three systems:
1. **Bankruptcy detection** - sets player to BANKRUPT status
2. **Turn advancement** - tries to move to next player
3. **Game over detection** - checks win condition and ends game

The failure to ensure `activePlayerIndex` remains consistent after a bankruptcy declaration causes the game to enter an invalid state where the current player is bankrupt, preventing further gameplay.

**Primary Fix Priority:** Modify `finishTurn()` or `advanceToNextPlayer()` to guarantee that when the game ends, `activePlayerIndex` points to a valid (non-bankrupt) player, or ensure UI code doesn't access a bankrupt player's state when the game is over.
