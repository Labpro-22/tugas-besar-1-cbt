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

void GameSession::handlePrintBoard() {
  std::vector<Player *> playerPointers;
  playerPointers.reserve(game.getPlayers().size());
  for (Player &player : game.getPlayers()) {
    playerPointers.push_back(&player);
  }

  std::cout << boardRenderer.renderBoard(
      &board, playerPointers, game.getCurrentTurn(), game.getMaxTurn());
}

void GameSession::handleRollDice(bool manual, int d1, int d2) {
  if (diceRolledThisTurn && !dice.checkDouble()) {
    throw InvalidTurnStateException(
        "Kamu sudah melempar dadu pada giliran ini!");
  }

  Player &currentPlayer = game.getCurrentPlayer();
  const bool startedTurnInJail = currentPlayer.getStatus() == JAILED;
  int jailAttempts = 0;
  bool releasedFromJailByDouble = false;

  if (startedTurnInJail) {
    const int jailFine = configuration.getJailFine();
    jailAttempts = getJailAttemptCount(currentPlayer);

    std::cout << currentPlayer.getUsername() << " sedang berada di Penjara.\n";

    if (jailAttempts >= 3) {
      std::cout << "Ini adalah giliran ke-4 di penjara. "
                << currentPlayer.getUsername() << " wajib membayar denda M"
                << jailFine << " sebelum melempar dadu.\n";
      game.executeTaxPayment(currentPlayer, jailFine, true);
      if (currentPlayer.getStatus() == BANKRUPT) {
        turnActionTaken = true;
        finishTurn();
        return;
      }

      releasePlayerFromJail(currentPlayer);
      std::cout << currentPlayer.getUsername()
                << " berhasil keluar dari Penjara setelah membayar denda.\n";
    } else {
      std::cout << "Pilih cara keluar:\n";
      std::cout << "1. Bayar denda M" << jailFine << "\n";
      std::cout << "2. Gunakan Kartu \"Bebas dari Penjara\"\n";
      std::cout << "3. Coba lempar dadu (harus double)\n";

      const int choice =
          cli.getInputHandler().readChoice(1, 3, "Pilihan (1/2/3): ");
      if (choice == 1) {
        game.executeTaxPayment(currentPlayer, jailFine, true);
        if (currentPlayer.getStatus() == BANKRUPT) {
          turnActionTaken = true;
          finishTurn();
          return;
        }

        releasePlayerFromJail(currentPlayer);
        std::cout << currentPlayer.getUsername()
                  << " keluar dari Penjara setelah membayar denda.\n";
      } else if (choice == 2) {
        Card *jailCard = nullptr;
        for (Card *card : currentPlayer.getHand()) {
          if (card->getDescription().find("Bebas dari Penjara") !=
              std::string::npos) {
            jailCard = card;
            break;
          }
        }

        if (jailCard == nullptr) {
          std::cout << "Kamu tidak memiliki kartu \"Bebas dari Penjara\".\n";
          std::cout << "Mencoba melempar dadu sebagai alternatif...\n";
        } else {
          currentPlayer.removeCard(static_cast<SkillCard *>(jailCard));
          releasePlayerFromJail(currentPlayer);
          std::cout << currentPlayer.getUsername()
                    << " menggunakan kartu dan keluar dari Penjara.\n";
        }
      } else if (choice == 3) {
        std::cout << "Mencoba melempar dadu untuk mendapatkan double...\n";
      }

      // Notifikasi snapshot agar GUI tahu pilihan sudah diproses
      notifySnapshotImmediate();
    }
  }

  if (manual) {
    if (d1 < 1 || d1 > 6 || d2 < 1 || d2 > 6) {
      throw InvalidDiceValueException(d1, d2);
    }

    dice.setValues(d1, d2);
    std::cout << "Dadu diatur secara manual.\n";
  } else {
    std::cout << "Mengocok dadu...\n";
    dice.roll();
  }

  diceRolledThisTurn = true;

  const int total = dice.getTotal();
  const bool rolledDouble = dice.checkDouble();
  game.setLastDiceTotal(total);

  game.getLogger().log(
      game.getCurrentTurn(), currentPlayer.getUsername(), "DADU",
      "Lempar: " + std::to_string(dice.getDie1()) + "+" +
          std::to_string(dice.getDie2()) + "=" + std::to_string(total));

  std::cout << "Hasil: " << dice.getDie1() << " + " << dice.getDie2() << " = "
            << total << "\n";

  if (startedTurnInJail && currentPlayer.getStatus() == JAILED) {
    if (!rolledDouble) {
      if (jailAttempts >= 2) {
        // Gagal di percobaan ke-3, wajib bayar denda dan jalan
        const int jailFine = configuration.getJailFine();
        std::cout << "Gagal mendapatkan double di percobaan ke-3.\n";
        std::cout << currentPlayer.getUsername() << " wajib membayar denda M"
                  << jailFine << " dan melanjutkan perjalanan.\n";
        game.executeTaxPayment(currentPlayer, jailFine, true);
        if (currentPlayer.getStatus() == BANKRUPT) {
          turnActionTaken = true;
          finishTurn();
          return;
        }
        releasePlayerFromJail(currentPlayer);
      } else {
        jailAttemptCounts[currentPlayer.getUsername()] = jailAttempts + 1;
        std::cout
            << "Kamu gagal mendapatkan double dan tetap berada di Penjara.\n";
        std::cout << "Percobaan keluar penjara: "
                  << jailAttemptCounts[currentPlayer.getUsername()] << " / 3\n";
        game.getLogger().log(
            game.getCurrentTurn(), currentPlayer.getUsername(), "PENJARA",
            "Gagal keluar dari penjara (percobaan " +
                std::to_string(jailAttemptCounts[currentPlayer.getUsername()]) +
                "/3)");
        notifySnapshotImmediate();
        turnActionTaken = true;
        finishTurn();
        return;
      }
    } else {
      releasePlayerFromJail(currentPlayer);
      releasedFromJailByDouble = true;
      std::cout << "Double! " << currentPlayer.getUsername()
                << " berhasil keluar dari Penjara.\n";
      game.getLogger().log(game.getCurrentTurn(), currentPlayer.getUsername(),
                           "PENJARA", "Keluar dari penjara karena double");
    }
  }

  if (dice.hasThreeConsecutiveDoubles()) {
    std::cout << "Double 3 kali berturut-turut! " << currentPlayer.getUsername()
              << " terkena sanksi masuk Penjara.\n";
    game.getLogger().log(game.getCurrentTurn(), currentPlayer.getUsername(),
                         "DOUBLE",
                         "Double ke-3 berturut-turut -> masuk penjara");
    game.goToJail(currentPlayer);
    if (currentPlayer.getStatus() == JAILED) {
      markPlayerJailed(currentPlayer);
    }
    notifySnapshotImmediate();
    turnActionTaken = true;
    finishTurn();
    return;
  }

  std::cout << "Memajukan Bidak " << currentPlayer.getUsername() << " sebanyak "
            << total << " petak...\n";

  game.moveCurrentPlayer(total);
  Tile &tile = board.getTile(currentPlayer.getPosition());
  std::cout << "Bidak mendarat di: " << tile.getName() << ".\n";
  notifySnapshotImmediate();
  tile.onLanded(currentPlayer, game);
  if (currentPlayer.getStatus() == JAILED) {
    markPlayerJailed(currentPlayer);
  }

  turnActionTaken = true;

  if (rolledDouble && currentPlayer.getStatus() == ACTIVE &&
      !releasedFromJailByDouble && !game.isGameOver()) {
    std::cout << currentPlayer.getUsername()
              << " mendapat double. Giliran tambahan!\n";
    game.getLogger().log(game.getCurrentTurn(), currentPlayer.getUsername(),
                         "DOUBLE", "Mendapat giliran tambahan");
    notifySnapshotImmediate();
    return;
  }

  finishTurn();
}

void GameSession::markPlayerJailed(Player &player) {
  if (player.getStatus() == JAILED) {
    jailAttemptCounts[player.getUsername()] = 0;
  }
}

void GameSession::releasePlayerFromJail(Player &player) {
  player.setActive();
  jailAttemptCounts.erase(player.getUsername());
}

int GameSession::getJailAttemptCount(const Player &player) const {
  const auto it = jailAttemptCounts.find(player.getUsername());
  if (it == jailAttemptCounts.end()) {
    return 0;
  }

  return it->second;
}

void GameSession::handlePrintDeed() {
  std::string code = GameSessionUtil::uppercase(
      cli.getInputHandler().readToken("Masukkan kode petak: "));

  Property *property = queries.findPropertyByCode(code);
  if (property == nullptr) {
    throw PropertyNotFoundException(code);
  }

  cli.getPropertyView().showDeed(property);
}

void GameSession::handlePrintProperties() {
  cli.getPropertyView().showPlayerProperties(&game.getCurrentPlayer());
}

void GameSession::handlePrintLogs(const Command &command) {
  std::vector<LogEntry> entries = game.getLogger().getEntries();
  if (command.args.empty()) {
    cli.getLogView().showLogs(entries.data(), entries.size());
    return;
  }

  cli.getLogView().showLastLogs(entries.data(), entries.size(),
                                std::stoi(command.args[0]));
}

void GameSession::handleMortgage() {
  Player &currentPlayer = game.getCurrentPlayer();
  std::vector<Property *> mortgageable = queries.getMortgageableProperties();
  cli.getPropertyView().showMortgageOptions(mortgageable);
  if (mortgageable.empty()) {
    throw PropertyOwnershipException(
        "-", "Kamu tidak memiliki aset yang dapat digadaikan.");
  }

  std::string selectionPrompt = "Pilih nomor properti (0 untuk batal):\n";
  for (size_t i = 0; i < mortgageable.size(); ++i) {
    selectionPrompt += std::to_string(i + 1) + ". [" +
                       mortgageable[i]->getCode() + "] " +
                       mortgageable[i]->getName() + "\n";
  }

  int choice = cli.getInputHandler().readChoice(
      0, static_cast<int>(mortgageable.size()), selectionPrompt);
  if (choice == 0) {
    return;
  }

  Property *property = mortgageable[static_cast<std::size_t>(choice - 1)];
  if (property->getType() == "Street") {
    Street *street = static_cast<Street *>(property);
    if (game.hasBuildingsInColorGroup(currentPlayer, street->getColorGroup())) {
      std::cout << property->getName()
                << " tidak dapat digadaikan langsung karena masih ada "
                   "bangunan di color group ["
                << GameSessionUtil::colorGroupLabel(street->getColorGroup())
                << "].\n";

      std::string sellPrompt =
          "Konfirmasi Jual Bangunan\n"
          "Masih terdapat bangunan di color group [" +
          GameSessionUtil::colorGroupLabel(street->getColorGroup()) +
          "].\n"
          "Jual semua bangunan color group ini? (y/n)";
      const bool sellBuildings = cli.getInputHandler().readYesNo(sellPrompt);
      if (!sellBuildings) {
        return;
      }

      const int soldValue = game.sellBuildingsInColorGroup(
          currentPlayer, street->getColorGroup());
      std::cout << "Bangunan di color group ["
                << GameSessionUtil::colorGroupLabel(street->getColorGroup())
                << "] terjual. Kamu menerima M" << soldValue << ".\n";

      std::string continuePrompt = "Konfirmasi Gadai\n"
                                   "Lanjut menggadaikan " +
                                   property->getName() + "? (y/n)";
      const bool continueMortgage =
          cli.getInputHandler().readYesNo(continuePrompt);
      if (!continueMortgage) {
        return;
      }
    }
  }

  // Final Mortgage Confirmation
  std::string mortgagePrompt =
      "Konfirmasi Gadai\n"
      "Apakah Anda yakin ingin menggadaikan aset ini?\n"
      "Anda akan menerima:\nM" +
      std::to_string(property->getMortgageValue()) +
      "\n"
      "Target Properti: " +
      property->getName();
  if (!cli.getInputHandler().readYesNo(mortgagePrompt)) {
    return;
  }

  const int received = game.executeMortgage(currentPlayer, *property);

  std::cout << property->getName() << " berhasil digadaikan.\n";
  std::cout << "Kamu menerima M" << received << " dari Bank.\n";
  std::cout << "Uang kamu sekarang: M" << currentPlayer.getCash() << "\n";
  turnActionTaken = true;
  notifySnapshotImmediate();
}

void GameSession::handleRedeem() {
  Player &currentPlayer = game.getCurrentPlayer();
  std::vector<Property *> redeemable = queries.getRedeemableProperties();
  cli.getPropertyView().showRedeemOptions(redeemable);
  if (redeemable.empty()) {
    throw PropertyRedeemException(
        "-", "Kamu tidak memiliki aset yang sedang digadaikan.");
  }

  std::string selectionPrompt = "Pilih nomor properti (0 untuk batal):\n";
  for (size_t i = 0; i < redeemable.size(); ++i) {
    selectionPrompt += std::to_string(i + 1) + ". [" +
                       redeemable[i]->getCode() + "] " +
                       redeemable[i]->getName() + "\n";
  }

  int choice = cli.getInputHandler().readChoice(
      0, static_cast<int>(redeemable.size()), selectionPrompt);
  if (choice == 0) {
    return;
  }

  Property *property = redeemable[static_cast<std::size_t>(choice - 1)];
  const int baseRedeemPrice = property->getRedeemPrice();
  const int redeemPrice = game.applyDiscount(currentPlayer, baseRedeemPrice);
  if (!currentPlayer.canPay(redeemPrice)) {
    throw PropertyRedeemException(
        property->getCode(), "Uang kamu tidak cukup. Butuh M" +
                                 std::to_string(redeemPrice) + ", tersedia M" +
                                 std::to_string(currentPlayer.getCash()) + ".");
  }

  // Final Redeem Confirmation
  std::string redeemPrompt =
      "Konfirmasi Tebus\n"
      "Apakah Anda yakin ingin menebus kembali aset ini?\n"
      "BIAYA TEBUS:\nM" +
      std::to_string(redeemPrice);
  if (redeemPrice != baseRedeemPrice) {
    redeemPrompt += " (setelah diskon)";
  }
  redeemPrompt += "\nTarget Properti: " + property->getName();

  if (!cli.getInputHandler().readYesNo(redeemPrompt)) {
    return;
  }

  const int paid = game.executeRedeem(currentPlayer, *property);

  std::cout << property->getName() << " berhasil ditebus!\n";
  std::cout << "Kamu membayar M" << baseRedeemPrice;
  if (paid != baseRedeemPrice) {
    std::cout << " -> M" << paid << " setelah diskon";
  }
  std::cout << " ke Bank.\n";
  std::cout << "Uang kamu sekarang: M" << currentPlayer.getCash() << "\n";
  turnActionTaken = true;
  notifySnapshotImmediate();
}

void GameSession::handleBuild() {
  Player &currentPlayer = game.getCurrentPlayer();
  std::map<std::string, std::vector<Street *>> groupedStreets;

  for (Street *street : queries.getBuildableStreets()) {
    if (street == nullptr)
      continue;
    groupedStreets[GameSessionUtil::colorGroupLabel(street->getColorGroup())]
        .push_back(street);
  }

  if (groupedStreets.empty()) {
    throw PropertyBuildException(
        "-", "Tidak ada color group yang memenuhi syarat untuk dibangun.");
  }

  std::vector<std::string> orderedGroups;
  for (const auto &entry : groupedStreets)
    orderedGroups.push_back(entry.first);

  // Color Group Selection
  std::string groupPrompt = "BANGUN (Pilih Grup Warna)\n"
                            "Uang Kamu Saat Ini: M" +
                            std::to_string(currentPlayer.getCash()) + "\n";
  for (size_t i = 0; i < orderedGroups.size(); ++i) {
    groupPrompt += std::to_string(i + 1) + ". [" + orderedGroups[i] + "]\n";
    for (Street *s : groupedStreets[orderedGroups[i]]) {
      groupPrompt += "   - " + s->getName() + " (" + s->getCode() +
                     "): " + s->getBuildingLabel() + " (Harga: M" +
                     std::to_string(s->getNextBuildCost()) + ")\n";
    }
  }

  int groupChoice = cli.getInputHandler().readChoice(
      0, static_cast<int>(orderedGroups.size()), groupPrompt);
  if (groupChoice == 0)
    return;

  std::vector<Street *> &streets =
      groupedStreets[orderedGroups[static_cast<std::size_t>(groupChoice - 1)]];
  std::vector<Street *> eligible = game.getEligibleBuildTargets(streets);

  if (eligible.empty()) {
    std::cout << "Tidak ada petak yang bisa dibangun saat ini.\n";
    return;
  }

  // Property Selection
  std::string propertyPrompt =
      "BANGUN (Pilih Petak)\n"
      "Color Group [" +
      orderedGroups[static_cast<std::size_t>(groupChoice - 1)] + "]\n";
  for (size_t i = 0; i < eligible.size(); ++i) {
    propertyPrompt += std::to_string(i + 1) + ". " + eligible[i]->getName() +
                      " (" + eligible[i]->getCode() +
                      "): " + eligible[i]->getBuildingLabel() + " (Biaya: M" +
                      std::to_string(eligible[i]->getNextBuildCost()) + ")\n";
  }

  int propertyChoice = cli.getInputHandler().readChoice(
      0, static_cast<int>(eligible.size()), propertyPrompt);
  if (propertyChoice == 0)
    return;

  Street *target = eligible[static_cast<std::size_t>(propertyChoice - 1)];
  const bool hotelUpgrade = target->isNextBuildHotel();
  const int baseCost = target->getNextBuildCost();
  const int cost = game.applyDiscount(currentPlayer, baseCost);

  if (!currentPlayer.canPay(cost)) {
    throw PropertyBuildException(
        target->getCode(), "Uang kamu tidak cukup. Butuh M" +
                               std::to_string(cost) + ", tersedia M" +
                               std::to_string(currentPlayer.getCash()) + ".");
  }

  // Optional (upgrade to hotel)
  if (hotelUpgrade) {
    std::string confirmPrompt =
        "UPGRADE HOTEL\n"
        "Seluruh color group [" +
        orderedGroups[static_cast<std::size_t>(groupChoice - 1)] +
        "] sudah memiliki 4 rumah.\n"
        "Upgrade " +
        target->getName() +
        " ke Hotel?\n"
        "Biaya: M" +
        std::to_string(cost);
    if (!cli.getInputHandler().readYesNo(confirmPrompt))
      return;
  }

  const int paid = game.executeBuild(currentPlayer, *target);

  std::cout << (hotelUpgrade ? target->getName() + " di-upgrade ke Hotel!"
                             : "Kamu membangun 1 rumah di " + target->getName())
            << ". Biaya: M" << paid << "\n";
  std::cout << "Uang tersisa: M" << currentPlayer.getCash() << "\n";
  turnActionTaken = true;
  notifySnapshotImmediate();
}

void GameSession::handleUseAbility() {
  Player &currentPlayer = game.getCurrentPlayer();
  std::vector<Card *> &hand = currentPlayer.getHand();

  if (hand.empty()) {
    throw CardException("Kamu tidak memiliki kartu kemampuan.");
  }

  if (diceRolledThisTurn) {
    throw AbilityTimingException(
        "Kartu kemampuan hanya bisa digunakan SEBELUM melempar dadu.");
  }
  if (!currentPlayer.canUseAbility()) {
    throw AbilityAlreadyUsedException();
  }

  std::cout << "Daftar Kartu Kemampuan Spesial Anda:\n";
  for (std::size_t i = 0; i < hand.size(); ++i) {
    std::cout << (i + 1) << ". " << hand[i]->getType() << " - "
              << hand[i]->getDescription() << "\n";
  }
  std::cout << "0. Batal\n";

  std::string usePrompt = "Pilih kartu yang ingin digunakan (0 untuk batal):\n";
  for (std::size_t i = 0; i < hand.size(); ++i) {
    usePrompt += std::to_string(i + 1) + ". " + hand[i]->getType();
    if (hand[i]->getValue() != 0)
      usePrompt += " (" + std::to_string(hand[i]->getValue()) + ")";
    usePrompt += "\n";
  }
  int choice = cli.getInputHandler().readChoice(
      0, static_cast<int>(hand.size()), usePrompt);
  if (choice == 0) {
    return;
  }

  SkillCard *selected =
      static_cast<SkillCard *>(hand[static_cast<std::size_t>(choice - 1)]);
  if (selected == nullptr) {
    throw InvalidCardException("Pilihan kartu");
  }

  if (!executeSkillCard(currentPlayer, selected)) {
    return;
  }

  turnActionTaken = true;

  if (currentPlayer.getStatus() == JAILED ||
      currentPlayer.getStatus() == BANKRUPT) {
    finishTurn();
  }
}

void GameSession::handleSave(const Command &command) {
  if (turnActionTaken) {
    throw SaveGameException(
        "SIMPAN hanya dapat dipanggil di awal giliran (sebelum ada aksi).");
  }

  if (saveToFile(command.args[0])) {
    std::cout << "Permainan berhasil disimpan ke: " << command.args[0] << "\n";
    game.getLogger().log(game.getCurrentTurn(),
                         game.getCurrentPlayer().getUsername(), "SAVE",
                         "Simpan ke " + command.args[0]);
    return;
  }

  throw SaveGameException(
      "Gagal menyimpan file! Pastikan direktori dapat ditulis.");
}

void GameSession::handleLoad(const Command &) const {
  throw CommandNotAllowedException(
      "MUAT", "MUAT hanya dapat dilakukan dari menu awal program.");
}
