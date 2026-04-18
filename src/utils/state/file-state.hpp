#ifndef FILE_STATE_HPP
#define FILE_STATE_HPP

#include <string>
#include <fstream>
#include "GameState.hpp"

// GameSaver class, responsible for saving and loading game state to/from files.
// NOTE: Uses GameState's serialize and deserialize methods, this class only handles file I/O.
class GameSaver {
    public:
        static bool saveGame(const GameState& state, const string& filename);
        static bool loadGame(GameState& state, const string& filename);
};

#endif // FILE_STATE_HPP