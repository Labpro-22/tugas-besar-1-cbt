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

void GameSession::initializeSkillDeck() {
    skillDeck.clear();

    for (int i = 0; i < 4; ++i) skillDeck.addCard("MoveCard");
    for (int i = 0; i < 3; ++i) skillDeck.addCard("DiscountCard");
    for (int i = 0; i < 2; ++i) skillDeck.addCard("ShieldCard");
    for (int i = 0; i < 2; ++i) skillDeck.addCard("TeleportCard");
    for (int i = 0; i < 2; ++i) skillDeck.addCard("LassoCard");
    for (int i = 0; i < 2; ++i) skillDeck.addCard("DemolitionCard");
    
    skillDeck.shuffle();
}

void GameSession::shuffleSkillDeck() {
    skillDeck.shuffle();
}

void GameSession::ensureSkillDeckAvailable() {
    if (skillDeck.isEmpty()) {
        skillDeck.reshuffleDiscard();
        std::cout << "Deck kartu kemampuan habis. Mencampur ulang discard pile...\n";
    }
}

SkillCard* GameSession::createSkillCardInstance(const std::string& type,
                                                int value,
                                                int duration) {
    return skillCardFactory.create(type, value, duration);
}

SkillCard* GameSession::drawSkillCard() {
    try {
        const std::string type = skillDeck.draw();
        return createSkillCardInstance(type);
    } catch (const EmptyDeckException&) {
        return nullptr;
    }
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
                         "Mendapat " + drawnCard->getDisplayLabel());

    if (player.getCardCount() <= 3) {
        return;
    }

    std::string cardListLog = "Tangan penuh! Wajib buang 1 kartu:";
    for (std::size_t i = 0; i < player.getHand().size(); ++i) {
        Card* card = player.getHand()[i];
        cardListLog += " " + std::to_string(i + 1) + ". " + card->getType();
        if (card->getValue() != 0) {
            cardListLog += "(" + std::to_string(card->getValue()) + ")";
        }
        cardListLog += ";";
    }
    game.getLogger().log(game.getCurrentTurn(), player.getUsername(), "KARTU", cardListLog);
    notifySnapshotImmediate();

    std::cout << "PERINGATAN: Kamu sudah memiliki 3 kartu di tangan (maksimal 3).\n";
    std::cout << "Kamu diwajibkan membuang 1 kartu.\n";
    std::cout << "Daftar Kartu Kemampuan Anda:\n";
    for (std::size_t i = 0; i < player.getHand().size(); ++i) {
        Card* card = player.getHand()[i];
        std::cout << (i + 1) << ". " << card->getType() << " - "
                  << card->getDescription() << "\n";
    }

    std::string discardPrompt = "Tangan penuh! Pilih kartu untuk dibuang:\n";
    for (std::size_t i = 0; i < player.getHand().size(); ++i) {
        Card* c = player.getHand()[i];
        discardPrompt += std::to_string(i + 1) + ". " + c->getType();
        if (c->getValue() != 0) discardPrompt += " (" + std::to_string(c->getValue()) + ")";
        discardPrompt += "\n";
    }
    const int discardChoice = cli.getInputHandler().readChoice(
        1, static_cast<int>(player.getHand().size()), discardPrompt);
    SkillCard* discardCard =
        static_cast<SkillCard*>(player.getHand()[static_cast<std::size_t>(discardChoice - 1)]);
    if (discardCard != nullptr) {
        game.getLogger().log(game.getCurrentTurn(), player.getUsername(), "KARTU",
                             "Membuang kartu " + std::to_string(discardChoice) + ": " + discardCard->getType());
        std::cout << discardCard->getType() << " telah dibuang.\n";
        discardSkillCard(player, discardCard);
    }
}

void GameSession::discardSkillCard(Player& player, SkillCard* card) {
    if (card == nullptr) {
        return;
    }

    skillDeck.discardCard(card->getType());
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
