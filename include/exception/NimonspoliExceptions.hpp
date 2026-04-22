#ifndef APP_NIMONSPOLI_EXCEPTIONS_HPP
#define APP_NIMONSPOLI_EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

class NimonspoliException : public std::runtime_error {
public:
    explicit NimonspoliException(const std::string& message,
                                 const std::string& errorCode = "NIMONSPOLI_ERROR");

    const std::string& getErrorCode() const noexcept;

private:
    std::string errorCode;
};

class InputException : public NimonspoliException {
public:
    explicit InputException(const std::string& message,
                            const std::string& errorCode = "INPUT_ERROR");
};

class InputStreamClosedException final : public InputException {
public:
    InputStreamClosedException();
};

class InvalidInputException : public InputException {
public:
    explicit InvalidInputException(const std::string& message = "Input tidak valid.",
                                   const std::string& errorCode = "INVALID_INPUT");
};

class InvalidChoiceException final : public InvalidInputException {
public:
    InvalidChoiceException(int minimum, int maximum);
};

class InvalidDiceValueException final : public InvalidInputException {
public:
    InvalidDiceValueException(int die1, int die2);
};

class CommandException : public NimonspoliException {
public:
    explicit CommandException(const std::string& message,
                              const std::string& errorCode = "COMMAND_ERROR");
};

class UnknownCommandException final : public CommandException {
public:
    explicit UnknownCommandException(const std::string& command);
};

class InvalidCommandFormatException final : public CommandException {
public:
    explicit InvalidCommandFormatException(const std::string& command);
};

class MissingCommandArgumentException final : public CommandException {
public:
    MissingCommandArgumentException(const std::string& command,
                                    const std::string& expectedArgument);
};

class CommandNotAllowedException final : public CommandException {
public:
    CommandNotAllowedException(const std::string& command,
                               const std::string& reason);
};

class ConfigurationException : public NimonspoliException {
public:
    explicit ConfigurationException(const std::string& message,
                                    const std::string& errorCode =
                                        "CONFIGURATION_ERROR");
};

class MissingConfigurationFileException final : public ConfigurationException {
public:
    explicit MissingConfigurationFileException(const std::string& filename);
};

class InvalidConfigurationFormatException final : public ConfigurationException {
public:
    InvalidConfigurationFormatException(const std::string& filename,
                                        const std::string& reason);
};

class InvalidBoardConfigurationException final : public ConfigurationException {
public:
    explicit InvalidBoardConfigurationException(const std::string& reason);
};

class PersistenceException : public NimonspoliException {
public:
    explicit PersistenceException(const std::string& message,
                                  const std::string& errorCode =
                                      "PERSISTENCE_ERROR");
};

class SaveGameException final : public PersistenceException {
public:
    explicit SaveGameException(const std::string& filename);
};

class LoadGameException final : public PersistenceException {
public:
    LoadGameException(const std::string& filename, const std::string& reason);
};

class GameStateException : public NimonspoliException {
public:
    explicit GameStateException(const std::string& message,
                                const std::string& errorCode = "GAME_STATE_ERROR");
};

class GameNotStartedException final : public GameStateException {
public:
    GameNotStartedException();
};

class GameAlreadyFinishedException final : public GameStateException {
public:
    GameAlreadyFinishedException();
};

class InvalidTurnStateException final : public GameStateException {
public:
    explicit InvalidTurnStateException(const std::string& reason);
};

class InvalidPlayerCountException final : public GameStateException {
public:
    InvalidPlayerCountException(int playerCount);
};

class PlayerNotFoundException final : public GameStateException {
public:
    explicit PlayerNotFoundException(const std::string& username);
};

class InvalidPlayerStateException final : public GameStateException {
public:
    InvalidPlayerStateException(const std::string& username,
                                const std::string& reason);
};

class JailRuleException final : public GameStateException {
public:
    explicit JailRuleException(const std::string& reason);
};

class BoardException : public NimonspoliException {
public:
    explicit BoardException(const std::string& message,
                            const std::string& errorCode = "BOARD_ERROR");
};

class InvalidBoardPositionException final : public BoardException {
public:
    InvalidBoardPositionException(int position, int boardSize);
};

class TileNotFoundException final : public BoardException {
public:
    explicit TileNotFoundException(const std::string& tileCode);
};

class UnsupportedTileTypeException final : public BoardException {
public:
    explicit UnsupportedTileTypeException(const std::string& tileType);
};

class PropertyException : public NimonspoliException {
public:
    explicit PropertyException(const std::string& message,
                               const std::string& errorCode = "PROPERTY_ERROR");
};

class PropertyNotFoundException final : public PropertyException {
public:
    explicit PropertyNotFoundException(const std::string& propertyCode);
};

class PropertyUnavailableException final : public PropertyException {
public:
    explicit PropertyUnavailableException(const std::string& propertyCode);
};

class PropertyOwnershipException final : public PropertyException {
public:
    PropertyOwnershipException(const std::string& propertyCode,
                               const std::string& reason);
};

class PropertyMortgageException final : public PropertyException {
public:
    PropertyMortgageException(const std::string& propertyCode,
                              const std::string& reason);
};

class PropertyBuildException final : public PropertyException {
public:
    PropertyBuildException(const std::string& propertyCode,
                           const std::string& reason);
};

class EconomyException : public NimonspoliException {
public:
    explicit EconomyException(const std::string& message,
                              const std::string& errorCode = "ECONOMY_ERROR");
};

class InsufficientFundsException final : public EconomyException {
public:
    InsufficientFundsException(const std::string& username, int required,
                               int available);
};

class RentPaymentException final : public EconomyException {
public:
    RentPaymentException(const std::string& username, int amount);
};

class TaxPaymentException final : public EconomyException {
public:
    TaxPaymentException(const std::string& username, int amount);
};

class JailFinePaymentException final : public EconomyException {
public:
    JailFinePaymentException(const std::string& username, int amount);
};

class BankruptcyException final : public EconomyException {
public:
    BankruptcyException(const std::string& username, int debt);
};

class AuctionException : public NimonspoliException {
public:
    explicit AuctionException(const std::string& message,
                              const std::string& errorCode = "AUCTION_ERROR");
};

class NoAuctionParticipantException final : public AuctionException {
public:
    NoAuctionParticipantException();
};

class InvalidBidException final : public AuctionException {
public:
    InvalidBidException(const std::string& username, int bid, int minimumBid);
};

class AuctionTurnException final : public AuctionException {
public:
    AuctionTurnException(const std::string& username,
                         const std::string& expectedUsername);
};

class AuctionStateException final : public AuctionException {
public:
    explicit AuctionStateException(const std::string& reason);
};

class AuctionFailedException final : public AuctionException {
public:
    explicit AuctionFailedException(const std::string& reason);
};

class CardException : public NimonspoliException {
public:
    explicit CardException(const std::string& message,
                           const std::string& errorCode = "CARD_ERROR");
};

class EmptyDeckException final : public CardException {
public:
    EmptyDeckException();
};

class CardHandLimitException final : public CardException {
public:
    explicit CardHandLimitException(int limit);
};

class InvalidCardException final : public CardException {
public:
    explicit InvalidCardException(const std::string& cardName);
};

class AbilityAlreadyUsedException final : public CardException {
public:
    AbilityAlreadyUsedException();
};

class AbilityTimingException final : public CardException {
public:
    explicit AbilityTimingException(const std::string& reason);
};

class AbilityTargetException final : public CardException {
public:
    explicit AbilityTargetException(const std::string& reason);
};

class InternalGameException final : public NimonspoliException {
public:
    explicit InternalGameException(const std::string& reason);
};

#endif
