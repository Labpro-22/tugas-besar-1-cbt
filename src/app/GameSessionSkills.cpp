#include "app/GameSession.hpp"
#include "app/GameSessionUtil.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "core/Board-Tiles/PropertyTile.hpp"
#include "core/Board-Tiles/Tile.hpp"
#include "models/Card/SkillCard.hpp"
#include "models/GameManager/LogEntry.hpp"
#include "models/GameManager/Player.hpp"
#include "models/Property/Railroad.hpp"
#include "models/Property/Street.hpp"
#include "models/Property/Utility.hpp"

using namespace app;

void GameSession::initializeSkillDeck() {
    skillDeck.clear();
    skillDiscard.clear();

    skillDeck.insert(skillDeck.end(), 4, "MoveCard");
    skillDeck.insert(skillDeck.end(), 3, "DiscountCard");
    skillDeck.insert(skillDeck.end(), 2, "ShieldCard");
    skillDeck.insert(skillDeck.end(), 2, "TeleportCard");
    skillDeck.insert(skillDeck.end(), 2, "LassoCard");
    skillDeck.insert(skillDeck.end(), 2, "DemolitionCard");
    shuffleSkillDeck();
}

void GameSession::shuffleSkillDeck() {
    std::shuffle(skillDeck.begin(), skillDeck.end(),
                 std::mt19937(std::random_device{}()));
}

void GameSession::ensureSkillDeckAvailable() {
    if (!skillDeck.empty() || skillDiscard.empty()) {
        return;
    }

    skillDeck = skillDiscard;
    skillDiscard.clear();
    shuffleSkillDeck();
    std::cout << "Deck kartu kemampuan habis. Mencampur ulang discard pile...\n";
}

SkillCard* GameSession::createSkillCardInstance(const std::string& type,
                                                int value,
                                                int duration) {
    return skillCardFactory.create(type, value, duration);
}

SkillCard* GameSession::drawSkillCard() {
    ensureSkillDeckAvailable();
    if (skillDeck.empty()) {
        return nullptr;
    }

    const std::string type = skillDeck.back();
    skillDeck.pop_back();
    return createSkillCardInstance(type);
}

void GameSession::awardSkillCardAtTurnStart() {
    if (!gameStarted || game.getPlayers().empty()) {
        return;
    }

    Player& player = game.getCurrentPlayer();
    SkillCard* drawnCard = drawSkillCard();
    if (drawnCard == nullptr) {
        std::cout << "Tidak ada kartu kemampuan yang tersedia untuk dibagikan.\n";
        return;
    }

    player.addCard(drawnCard);
    std::cout << "Kamu mendapatkan 1 kartu acak baru!\n";
    std::cout << "Kartu yang didapat: " << drawnCard->getType();
    if (drawnCard->getValue() != 0) {
        std::cout << " (" << drawnCard->getValue() << ")";
    }
    std::cout << ".\n";
    game.getLogger().log(game.getCurrentTurn(), player.getUsername(), "KARTU",
                         "Mendapat " + skillCardDisplayLabel(drawnCard));

    if (player.getCardCount() <= 3) {
        return;
    }

    std::cout << "PERINGATAN: Kamu sudah memiliki 3 kartu di tangan (maksimal 3).\n";
    std::cout << "Kamu diwajibkan membuang 1 kartu.\n";
    std::cout << "Daftar Kartu Kemampuan Anda:\n";
    for (std::size_t i = 0; i < player.getHand().size(); ++i) {
        Card* card = player.getHand()[i];
        std::cout << (i + 1) << ". " << card->getType() << " - "
                  << card->getDescription() << "\n";
    }

    const int discardChoice = cli.getInputHandler().readChoice(
        1, static_cast<int>(player.getHand().size()),
        "Pilih nomor kartu yang ingin dibuang: ");
    SkillCard* discardCard =
        static_cast<SkillCard*>(player.getHand()[static_cast<std::size_t>(discardChoice - 1)]);
    if (discardCard != nullptr) {
        std::cout << discardCard->getType() << " telah dibuang.\n";
        discardSkillCard(player, discardCard);
    }
}

void GameSession::discardSkillCard(Player& player, SkillCard* card) {
    if (card == nullptr) {
        return;
    }

    skillDiscard.push_back(card->getType());
    player.removeCard(card);
    skillCardFactory.release(card);
}

bool GameSession::executeSkillCard(Player& player, SkillCard* card) {
    if (card == nullptr) {
        return false;
    }

    card->use(&player, &game);
    if (!card->isUsed()) {
        return false;
    }

    for (Player& other : game.getPlayers()) {
        if (other.getStatus() == JAILED) {
            markPlayerJailed(other);
        }
    }

    discardSkillCard(player, card);
    return true;
}

