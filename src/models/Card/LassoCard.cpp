#include "LassoCard.hpp"
#include "../GameManager/GameManager.hpp"

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

std::string LassoCard::getType() const {
    return "LassoCard";
}

void LassoCard::use(Player* p, GameManager* gm) {
    if (p == nullptr || gm == nullptr) {
        return;
    }

    vector<Player>& players = gm->getPlayers();

    // sendiri (kartu tidak berguna)
    if (players.size() < 2) {
        markAsUsed();
        return;
    }

    int currentIndex = -1;
    for (int i = 0; i < static_cast<int>(players.size()); ++i) {
        if (&players[i] == p) {
            currentIndex = i;
            break;
        }
    }

    // mencari target player di depan dan tarik ke posisi p sekarang
    if (currentIndex >= 0) {
        int targetIndex = (currentIndex + 1) % static_cast<int>(players.size());
        if (targetIndex != currentIndex) {
            players[targetIndex].setPosition(p->getPosition());
        }
    }

    markAsUsed();
}
