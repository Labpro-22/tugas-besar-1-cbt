#include "exception/NimonspoliExceptions.hpp"

#include <string>

namespace {

std::string money(int amount) {
    return "M" + std::to_string(amount);
}

std::string quoted(const std::string& value) {
    return "\"" + value + "\"";
}

std::string namedOrFallback(const std::string& value,
                            const std::string& fallback) {
    return value.empty() ? fallback : value;
}

}  // namespace

NimonspoliException::NimonspoliException(const std::string& message,
                                         const std::string& errorCode)
    : std::runtime_error(message), errorCode(errorCode) {}

const std::string& NimonspoliException::getErrorCode() const noexcept {
    return errorCode;
}

InputException::InputException(const std::string& message,
                               const std::string& errorCode)
    : NimonspoliException(message, errorCode) {}

InputStreamClosedException::InputStreamClosedException()
    : InputException("Input stream ditutup.", "INPUT_STREAM_CLOSED") {}

InvalidInputException::InvalidInputException(const std::string& message,
                                             const std::string& errorCode)
    : InputException(message, errorCode) {}

InvalidChoiceException::InvalidChoiceException(int minimum, int maximum)
    : InvalidInputException("Pilihan harus berada di antara " +
                            std::to_string(minimum) + " dan " +
                            std::to_string(maximum) + ".",
                            "INVALID_CHOICE") {}

InvalidDiceValueException::InvalidDiceValueException(int die1, int die2)
    : InvalidInputException("Nilai dadu tidak valid: " + std::to_string(die1) +
                            " dan " + std::to_string(die2) +
                            ". Nilai dadu harus 1 sampai 6.",
                            "INVALID_DICE_VALUE") {}

CommandException::CommandException(const std::string& message,
                                   const std::string& errorCode)
    : NimonspoliException(message, errorCode) {}

UnknownCommandException::UnknownCommandException(const std::string& command)
    : CommandException("Command " + quoted(namedOrFallback(command, "<kosong>")) +
                           " tidak dikenali.",
                       "UNKNOWN_COMMAND") {}

InvalidCommandFormatException::InvalidCommandFormatException(
    const std::string& command)
    : CommandException("Format command " +
                           quoted(namedOrFallback(command, "<kosong>")) +
                           " tidak valid.",
                       "INVALID_COMMAND_FORMAT") {}

MissingCommandArgumentException::MissingCommandArgumentException(
    const std::string& command, const std::string& expectedArgument)
    : CommandException("Command " + quoted(namedOrFallback(command, "<kosong>")) +
                           " membutuhkan argumen " + expectedArgument + ".",
                       "MISSING_COMMAND_ARGUMENT") {}

CommandNotAllowedException::CommandNotAllowedException(
    const std::string& command, const std::string& reason)
    : CommandException("Command " + quoted(namedOrFallback(command, "<kosong>")) +
                           " tidak dapat digunakan. " + reason,
                       "COMMAND_NOT_ALLOWED") {}

ConfigurationException::ConfigurationException(const std::string& message,
                                               const std::string& errorCode)
    : NimonspoliException(message, errorCode) {}

MissingConfigurationFileException::MissingConfigurationFileException(
    const std::string& filename)
    : ConfigurationException("File konfigurasi " +
                                 quoted(namedOrFallback(filename, "<kosong>")) +
                                 " tidak ditemukan atau tidak dapat dibaca.",
                             "MISSING_CONFIGURATION_FILE") {}

InvalidConfigurationFormatException::InvalidConfigurationFormatException(
    const std::string& filename, const std::string& reason)
    : ConfigurationException("Format file konfigurasi " +
                                 quoted(namedOrFallback(filename, "<kosong>")) +
                                 " tidak valid. " + reason,
                             "INVALID_CONFIGURATION_FORMAT") {}

InvalidBoardConfigurationException::InvalidBoardConfigurationException(
    const std::string& reason)
    : ConfigurationException("Konfigurasi papan tidak valid. " + reason,
                             "INVALID_BOARD_CONFIGURATION") {}

PersistenceException::PersistenceException(const std::string& message,
                                           const std::string& errorCode)
    : NimonspoliException(message, errorCode) {}

SaveGameException::SaveGameException(const std::string& filename)
    : PersistenceException("Gagal menyimpan permainan ke " +
                               quoted(namedOrFallback(filename, "<kosong>")) +
                               ".",
                           "SAVE_GAME_FAILED") {}

LoadGameException::LoadGameException(const std::string& filename,
                                     const std::string& reason)
    : PersistenceException("Gagal memuat permainan dari " +
                               quoted(namedOrFallback(filename, "<kosong>")) +
                               ". " + reason,
                           "LOAD_GAME_FAILED") {}

GameStateException::GameStateException(const std::string& message,
                                       const std::string& errorCode)
    : NimonspoliException(message, errorCode) {}

GameNotStartedException::GameNotStartedException()
    : GameStateException("Permainan belum dimulai.", "GAME_NOT_STARTED") {}

GameAlreadyFinishedException::GameAlreadyFinishedException()
    : GameStateException("Permainan sudah selesai.", "GAME_ALREADY_FINISHED") {}

InvalidTurnStateException::InvalidTurnStateException(const std::string& reason)
    : GameStateException("State giliran tidak valid. " + reason,
                         "INVALID_TURN_STATE") {}

InvalidPlayerCountException::InvalidPlayerCountException(int playerCount)
    : GameStateException("Jumlah pemain tidak valid: " +
                             std::to_string(playerCount) +
                             ". Jumlah pemain harus 2 sampai 4.",
                         "INVALID_PLAYER_COUNT") {}

PlayerNotFoundException::PlayerNotFoundException(const std::string& username)
    : GameStateException("Pemain " +
                             quoted(namedOrFallback(username, "<kosong>")) +
                             " tidak ditemukan.",
                         "PLAYER_NOT_FOUND") {}

InvalidPlayerStateException::InvalidPlayerStateException(
    const std::string& username, const std::string& reason)
    : GameStateException("State pemain " +
                             quoted(namedOrFallback(username, "<kosong>")) +
                             " tidak valid. " + reason,
                         "INVALID_PLAYER_STATE") {}

JailRuleException::JailRuleException(const std::string& reason)
    : GameStateException("Aturan penjara tidak dapat dipenuhi. " + reason,
                         "JAIL_RULE_ERROR") {}

BoardException::BoardException(const std::string& message,
                               const std::string& errorCode)
    : NimonspoliException(message, errorCode) {}

InvalidBoardPositionException::InvalidBoardPositionException(int position,
                                                             int boardSize)
    : BoardException("Posisi papan tidak valid: " + std::to_string(position) +
                         ". Ukuran papan: " + std::to_string(boardSize) + ".",
                     "INVALID_BOARD_POSITION") {}

TileNotFoundException::TileNotFoundException(const std::string& tileCode)
    : BoardException("Petak " +
                         quoted(namedOrFallback(tileCode, "<kosong>")) +
                         " tidak ditemukan.",
                     "TILE_NOT_FOUND") {}

UnsupportedTileTypeException::UnsupportedTileTypeException(
    const std::string& tileType)
    : BoardException("Tipe petak " +
                         quoted(namedOrFallback(tileType, "<kosong>")) +
                         " tidak didukung.",
                     "UNSUPPORTED_TILE_TYPE") {}

PropertyException::PropertyException(const std::string& message,
                                     const std::string& errorCode)
    : NimonspoliException(message, errorCode) {}

PropertyNotFoundException::PropertyNotFoundException(
    const std::string& propertyCode)
    : PropertyException("Properti " +
                            quoted(namedOrFallback(propertyCode, "<kosong>")) +
                            " tidak ditemukan.",
                        "PROPERTY_NOT_FOUND") {}

PropertyUnavailableException::PropertyUnavailableException(
    const std::string& propertyCode)
    : PropertyException("Properti " +
                            quoted(namedOrFallback(propertyCode, "<kosong>")) +
                            " tidak tersedia.",
                        "PROPERTY_UNAVAILABLE") {}

PropertyOwnershipException::PropertyOwnershipException(
    const std::string& propertyCode, const std::string& reason)
    : PropertyException("Kepemilikan properti " +
                            quoted(namedOrFallback(propertyCode, "<kosong>")) +
                            " tidak valid. " + reason,
                        "PROPERTY_OWNERSHIP_ERROR") {}

PropertyMortgageException::PropertyMortgageException(
    const std::string& propertyCode, const std::string& reason)
    : PropertyException("Status gadai properti " +
                            quoted(namedOrFallback(propertyCode, "<kosong>")) +
                            " tidak valid. " + reason,
                        "PROPERTY_MORTGAGE_ERROR") {}

PropertyRedeemException::PropertyRedeemException(const std::string& propertyCode,
                                                 const std::string& reason)
    : PropertyException("Redeem properti " +
                            quoted(namedOrFallback(propertyCode, "<kosong>")) +
                            " tidak valid. " + reason,
                        "PROPERTY_REDEEM_ERROR") {}

InvalidPropertyStatusException::InvalidPropertyStatusException(
    const std::string& statusStr)
    : PropertyException("Status properti " +
                            quoted(namedOrFallback(statusStr, "<kosong>")) +
                            " tidak dikenali.",
                        "INVALID_PROPERTY_STATUS") {}

PropertyBuildException::PropertyBuildException(const std::string& propertyCode,
                                               const std::string& reason)
    : PropertyException("Pembangunan properti " +
                            quoted(namedOrFallback(propertyCode, "<kosong>")) +
                            " tidak dapat dilakukan. " + reason,
                        "PROPERTY_BUILD_ERROR") {}

EconomyException::EconomyException(const std::string& message,
                                   const std::string& errorCode)
    : NimonspoliException(message, errorCode) {}

InsufficientFundsException::InsufficientFundsException(
    const std::string& username, int required, int available)
    : EconomyException("Uang " +
                           quoted(namedOrFallback(username, "<pemain>")) +
                           " tidak cukup. Butuh " + money(required) +
                           ", tersedia " + money(available) + ".",
                       "INSUFFICIENT_FUNDS") {}

RentPaymentException::RentPaymentException(const std::string& username,
                                           int amount)
    : EconomyException("Pemain " +
                           quoted(namedOrFallback(username, "<pemain>")) +
                           " tidak mampu membayar sewa " + money(amount) + ".",
                       "RENT_PAYMENT_FAILED") {}

TaxPaymentException::TaxPaymentException(const std::string& username, int amount)
    : EconomyException("Pemain " +
                           quoted(namedOrFallback(username, "<pemain>")) +
                           " tidak mampu membayar pajak " + money(amount) + ".",
                       "TAX_PAYMENT_FAILED") {}

JailFinePaymentException::JailFinePaymentException(const std::string& username,
                                                   int amount)
    : EconomyException("Pemain " +
                           quoted(namedOrFallback(username, "<pemain>")) +
                           " tidak mampu membayar denda penjara " +
                           money(amount) + ".",
                       "JAIL_FINE_PAYMENT_FAILED") {}

BankruptcyException::BankruptcyException(const std::string& username, int debt)
    : EconomyException("Pemain " +
                           quoted(namedOrFallback(username, "<pemain>")) +
                           " bangkrut dengan kewajiban " + money(debt) + ".",
                       "BANKRUPTCY") {}

AuctionException::AuctionException(const std::string& message,
                                   const std::string& errorCode)
    : NimonspoliException(message, errorCode) {}

NoAuctionParticipantException::NoAuctionParticipantException()
    : AuctionException("Tidak ada peserta lelang.", "NO_AUCTION_PARTICIPANT") {}

InvalidBidException::InvalidBidException(const std::string& username, int bid,
                                         int minimumBid)
    : AuctionException("Tawaran " + money(bid) + " dari " +
                           quoted(namedOrFallback(username, "<pemain>")) +
                           " tidak valid. Minimal harus lebih besar dari " +
                           money(minimumBid) + ".",
                       "INVALID_BID") {}

AuctionTurnException::AuctionTurnException(const std::string& username,
                                           const std::string& expectedUsername)
    : AuctionException("Belum giliran " +
                           quoted(namedOrFallback(username, "<pemain>")) +
                           " untuk menawar. Giliran saat ini: " +
                           quoted(namedOrFallback(expectedUsername, "<pemain>")) +
                           ".",
                       "AUCTION_TURN_ERROR") {}

AuctionStateException::AuctionStateException(const std::string& reason)
    : AuctionException("State lelang tidak valid. " + reason,
                       "AUCTION_STATE_ERROR") {}

CardException::CardException(const std::string& message,
                             const std::string& errorCode)
    : NimonspoliException(message, errorCode) {}

EmptyDeckException::EmptyDeckException()
    : CardException("Tidak dapat mengambil kartu dari deck kosong.",
                    "EMPTY_DECK") {}

CardHandLimitException::CardHandLimitException(int limit)
    : CardException("Jumlah kartu kemampuan melebihi batas. Maksimal " +
                        std::to_string(limit) + " kartu.",
                    "CARD_HAND_LIMIT") {}

InvalidCardException::InvalidCardException(const std::string& cardName)
    : CardException("Kartu " +
                        quoted(namedOrFallback(cardName, "<kosong>")) +
                        " tidak valid atau tidak didukung.",
                    "INVALID_CARD") {}

AbilityAlreadyUsedException::AbilityAlreadyUsedException()
    : CardException("Kartu kemampuan sudah digunakan pada giliran ini.",
                    "ABILITY_ALREADY_USED") {}

AbilityTimingException::AbilityTimingException(const std::string& reason)
    : CardException("Kartu kemampuan tidak dapat digunakan pada waktu ini. " +
                        reason,
                    "ABILITY_TIMING_ERROR") {}

AbilityTargetException::AbilityTargetException(const std::string& reason)
    : CardException("Target kartu kemampuan tidak valid. " + reason,
                    "ABILITY_TARGET_ERROR") {}

InternalGameException::InternalGameException(const std::string& reason)
    : NimonspoliException("Kesalahan internal game. " + reason,
                          "INTERNAL_GAME_ERROR") {}
