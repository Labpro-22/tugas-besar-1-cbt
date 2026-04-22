#ifndef DATA_GAME_SESSION_PERSISTENCE_HPP
#define DATA_GAME_SESSION_PERSISTENCE_HPP

#include <string>

class GameSession;

class GameSessionPersistence {
private:
    static bool parseInteger(const std::string& token, int& value);

public:
    bool save(const GameSession& session, const std::string& filename) const;
    bool load(GameSession& session, const std::string& filename) const;
};

#endif
