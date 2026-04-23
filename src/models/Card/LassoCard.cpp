#include "models/Card/LassoCard.hpp"
#include "models/GameManager/GameManager.hpp"
#include "core/Board-Tiles/Board.hpp"
#include "core/Board-Tiles/Tile.hpp"
#include "views/InputHandler.hpp"

// ctor
LassoCard::LassoCard() : SkillCard() {}

// ctor with id
LassoCard::LassoCard(int cardId) : SkillCard(cardId) {}

// dtor
LassoCard::~LassoCard() {}

// Must implement functions
std::string LassoCard::getDescription() const {
    return "Pulls a target player to your tile.";
}

std::string LassoCard::getType() const { return "LassoCard"; }

void LassoCard::use(Player *p, GameManager *gm) {
    if (p == nullptr || gm == nullptr) {
        throw InternalGameException("LassoCard::use menerima konteks yang tidak valid.");
    }

    if (!p->canUseAbility()) {
        throw AbilityAlreadyUsedException();
    }

    const int boardSize = gm->getBoardSize();
    const int currentLap = p->getLapCount();
    std::vector<Player *> candidates;
    for (Player &player : gm->getPlayers()) {
        if (&player == p) {
            continue;
        }
        if (player.getStatus() == BANKRUPT || player.getStatus() == JAILED) {
            continue;
        }
        const int targetLap = player.getLapCount();
        const int distance = (player.getPosition() - p->getPosition() + boardSize) % boardSize;
        const bool isAheadByLap = targetLap > currentLap;
        const bool isAheadOnSameLap = targetLap == currentLap && distance > 0;
        if (isAheadByLap || isAheadOnSameLap) {
            candidates.push_back(&player);
        }
    }

    if (candidates.empty()) {
        throw AbilityTargetException("Tidak ada pemain lawan yang berada di depanmu.");
    }

    std::string targetListLog = "LassoCard - pilih target:";
    std::cout << "Pilih target LassoCard:\n";
    for (size_t i = 0; i < candidates.size(); ++i) {
        const Player *target = candidates[i];
        const Tile &targetTile = gm->getBoard().getTile(target->getPosition());
        targetListLog += " " + std::to_string(i + 1) + ". " + target->getUsername() + "(" + targetTile.getCode() + ");";
        std::cout << (i + 1) << ". " << target->getUsername() << " (" << targetTile.getCode() << " - " << targetTile.getName() << ")\n";
    }
    gm->getLogger().log(gm->getCurrentTurn(), p->getUsername(), "KARTU", targetListLog);
    gm->pushSnapshot();
    InputHandler input;
    const int choice = input.readChoice(1, static_cast<int>(candidates.size()), "Pilih target LassoCard: ");

    Player *target = candidates[static_cast<std::size_t>(choice - 1)];
    target->setPosition(p->getPosition());
    Tile &landingTile = gm->getBoard().getTile(target->getPosition());
    std::cout << target->getUsername() << " ditarik ke petak " << landingTile.getName() << ".\n";

    markAsUsed();
    p->setUsedAbility();
    p->removeCard(this);

    gm->getLogger().log(gm->getCurrentTurn(), p->getUsername(), "KARTU", "LassoCard: Menarik " + target->getUsername() + " ke petak " + landingTile.getName());
    landingTile.onLanded(*target, *gm);
}
