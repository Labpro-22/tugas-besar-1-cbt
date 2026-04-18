#include "file-state.hpp"
#include <iostream>

using namespace std;

bool GameSaver::saveGame(const GameState& state, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file for writing: " << filename << endl;
        return false;
    }

    // Use GameState's serialize method to get the string representation of the state
    file << state.serialize();
    file.close();
    return true;
}

bool GameSaver::loadGame(GameState& state, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file for reading: " << filename << endl;
        return false;
    }

    string data((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    // Use GameState's deserialize method to populate the state
    state.deserialize(data);
    return true;
}