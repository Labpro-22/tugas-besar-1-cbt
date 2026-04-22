#include "../../../include/app/GameSession.hpp"
#include "../../../include/app/GameSessionUtil.hpp"

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

#include "../../../include/core/Board-Tiles/PropertyTile.hpp"
#include "../../../include/core/Board-Tiles/Tile.hpp"
#include "../../../include/models/Card/DemolitionCard.hpp"
#include "../../../include/models/Card/DiscountCard.hpp"
#include "../../../include/models/Card/LassoCard.hpp"
#include "../../../include/models/Card/MoveCard.hpp"
#include "../../../include/models/Card/ShieldCard.hpp"
#include "../../../include/models/Card/SkillCard.hpp"
#include "../../../include/models/Card/TeleportCard.hpp"
#include "../../../include/models/GameManager/LogEntry.hpp"
#include "../../../include/models/GameManager/Player.hpp"
#include "../../../include/models/Property/Railroad.hpp"
#include "../../../include/models/Property/Street.hpp"
#include "../../../include/models/Property/Utility.hpp"

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
        dynamic_cast<SkillCard*>(player.getHand()[static_cast<std::size_t>(discardChoice - 1)]);
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

    const std::string type = uppercase(card->getType());
    if (type == "MOVECARD") {
        return executeMoveCard(player, *card);
    }
    if (type == "DISCOUNTCARD") {
        return executeDiscountCard(player, *card);
    }
    if (type == "SHIELDCARD") {
        return executeShieldCard(player, *card);
    }
    if (type == "TELEPORTCARD") {
        return executeTeleportCard(player, *card);
    }
    if (type == "LASSOCARD") {
        return executeLassoCard(player, *card);
    }
    if (type == "DEMOLITIONCARD") {
        return executeDemolitionCard(player, *card);
    }

    std::cout << "Kartu kemampuan belum didukung.\n";
    return false;
}

bool GameSession::executeMoveCard(Player& player, SkillCard& card) {
    MoveCard* moveCard = dynamic_cast<MoveCard*>(&card);
    if (moveCard == nullptr) {
        return false;
    }

    const int steps = moveCard->getSteps();
    std::cout << "MoveCard diaktifkan! Bergerak maju " << steps << " petak.\n";
    game.moveCurrentPlayer(steps);
    std::cout << "Bidak mendarat di: "
              << board.getTile(player.getPosition()).getName() << ".\n";
    game.getLogger().log(game.getCurrentTurn(), player.getUsername(), "KARTU",
                         "MoveCard -> maju " + std::to_string(steps) + " petak");
    resolveLandingAfterAbility(player, true);
    return true;
}

bool GameSession::executeDiscountCard(Player& player, SkillCard& card) {
    DiscountCard* discountCard = dynamic_cast<DiscountCard*>(&card);
    if (discountCard == nullptr) {
        return false;
    }

    player.applyDiscount(discountCard->getDiscountPercent(),
                         discountCard->getRemainingDuration());
    std::cout << "DiscountCard diaktifkan! Diskon "
              << discountCard->getDiscountPercent()
              << "% aktif untuk giliran ini.\n";
    game.getLogger().log(
        game.getCurrentTurn(), player.getUsername(), "KARTU",
        "DiscountCard aktif " + std::to_string(discountCard->getDiscountPercent()) +
            "%");
    return true;
}

bool GameSession::executeShieldCard(Player& player, SkillCard&) {
    player.activateShield();
    std::cout << "ShieldCard diaktifkan! Kamu kebal terhadap tagihan atau sanksi"
                 " selama giliran ini.\n";
    game.getLogger().log(game.getCurrentTurn(), player.getUsername(), "KARTU",
                         "ShieldCard aktif");
    return true;
}

bool GameSession::executeTeleportCard(Player& player, SkillCard&) {
    int targetPosition = -1;
    while (targetPosition < 0 || targetPosition >= board.getTileCount()) {
        std::string token = uppercase(trim(cli.getInputHandler().readPromptLine(
            "Masukkan kode petak tujuan teleport: ", "Teleport")));
        const bool numeric =
            !token.empty() &&
            std::all_of(token.begin(), token.end(), [](unsigned char ch) {
                return std::isdigit(ch) != 0;
            });

        targetPosition = numeric ? std::stoi(token)
                                 : queries.findTilePositionByCode(token);
        if (targetPosition < 0 || targetPosition >= board.getTileCount()) {
            std::cout << "Kode petak tidak valid.\n";
        }
    }

    game.movePlayerTo(player, targetPosition, true);
    std::cout << "TeleportCard diaktifkan! Bidak dipindahkan ke "
              << board.getTile(player.getPosition()).getName() << ".\n";
    game.getLogger().log(game.getCurrentTurn(), player.getUsername(), "KARTU",
                         "TeleportCard -> " +
                             board.getTile(player.getPosition()).getCode());
    resolveLandingAfterAbility(player, false);
    return true;
}

bool GameSession::executeLassoCard(Player& player, SkillCard&) {
    const int boardSize = game.getBoardSize();
    std::vector<Player*> candidates;
    for (Player& other : game.getPlayers()) {
        if (&other == &player || other.getStatus() == BANKRUPT) {
            continue;
        }

        const int distance =
            (other.getPosition() - player.getPosition() + boardSize) % boardSize;
        if (distance > 0) {
            candidates.push_back(&other);
        }
    }

    if (candidates.empty()) {
        std::cout << "Tidak ada pemain lawan yang berada di depanmu.\n";
        return false;
    }

    std::cout << "Pilih target LassoCard:\n";
    for (std::size_t i = 0; i < candidates.size(); ++i) {
        const Player* target = candidates[i];
        std::cout << (i + 1) << ". " << target->getUsername() << " ("
                  << board.getTile(target->getPosition()).getCode() << " - "
                  << board.getTile(target->getPosition()).getName() << ")\n";
    }

    const int choice = cli.getInputHandler().readChoice(
        1, static_cast<int>(candidates.size()), "Pilih target LassoCard: ");
    Player* target = candidates[static_cast<std::size_t>(choice - 1)];
    target->setPosition(player.getPosition());
    std::cout << target->getUsername() << " ditarik ke petak "
              << board.getTile(player.getPosition()).getName() << ".\n";
    game.getLogger().log(game.getCurrentTurn(), player.getUsername(), "KARTU",
                         "LassoCard -> tarik " + target->getUsername());
    resolveLandingAfterAbility(*target, false);
    return true;
}

bool GameSession::executeDemolitionCard(Player& player, SkillCard&) {
    std::vector<Property*> targets;
    for (Player& other : game.getPlayers()) {
        if (&other == &player || other.getStatus() == BANKRUPT) {
            continue;
        }

        for (Property* property : other.getProperties()) {
            if (property != nullptr) {
                targets.push_back(property);
            }
        }
    }

    if (targets.empty()) {
        std::cout << "Tidak ada properti lawan yang bisa dihancurkan.\n";
        return false;
    }

    std::cout << "Pilih properti target DemolitionCard:\n";
    for (std::size_t i = 0; i < targets.size(); ++i) {
        Property* target = targets[i];
        std::cout << (i + 1) << ". " << target->getCode() << " | "
                  << target->getName() << " | pemilik: "
                  << (target->getOwner() ? target->getOwner()->getUsername() : "BANK");
        Street* street = dynamic_cast<Street*>(target);
        if (street != nullptr) {
            std::cout << " | " << buildingLabel(street);
        }
        std::cout << "\n";
    }

    const int choice = cli.getInputHandler().readChoice(
        1, static_cast<int>(targets.size()),
        "Pilih properti untuk DemolitionCard: ");
    Property* target = targets[static_cast<std::size_t>(choice - 1)];
    Player* owner = target->getOwner();
    if (owner != nullptr) {
        owner->removeProperty(target);
    }
    target->setOwner(nullptr);
    target->setStatusStr("BANK");
    target->setFestival(1, 0);
    target->setBuildingCount(0);

    std::cout << target->getName()
              << " dihancurkan dan kembali menjadi milik Bank.\n";
    game.getLogger().log(game.getCurrentTurn(), player.getUsername(), "KARTU",
                         "DemolitionCard -> " + target->getCode());
    return true;
}

void GameSession::resolveLandingAfterAbility(Player& player, bool /*grantGoSalary*/) {
    Tile& tile = board.getTile(player.getPosition());
    tile.onLanded(player, game);
    if (player.getStatus() == JAILED) {
        markPlayerJailed(player);
    }
}

int GameSession::applyDiscountToAmount(const Player& player, int amount) const {
    if (!player.hasDiscount() || amount <= 0) {
        return amount;
    }

    const int effectiveDiscount =
        std::clamp(player.getDiscountPercentage(), 0, 100);
    return amount - (amount * effectiveDiscount / 100);
}

