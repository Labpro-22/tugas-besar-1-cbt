#include "core/Board-Tiles/Board.hpp"
#include "core/Board-Tiles/ChanceTile.hpp"
#include "core/Board-Tiles/ActionTile.hpp"
#include "core/Board-Tiles/CommunityChestTile.hpp"
#include "core/Board-Tiles/FestivalTile.hpp"
#include "core/Board-Tiles/FreeParkingTile.hpp"
#include "core/Board-Tiles/GoTile.hpp"
#include "core/Board-Tiles/GoToJailTile.hpp"
#include "core/Board-Tiles/JailTile.hpp"
#include "core/Board-Tiles/PBMTaxTile.hpp"
#include "core/Board-Tiles/PPHTaxTile.hpp"
#include "core/Board-Tiles/PropertyTile.hpp"
#include "core/Board-Tiles/Tile.hpp"
#include "models/Card/Card.hpp"
#include "models/Card/CardDeck.hpp"
#include "models/Property/Property.hpp"
#include "models/Property/Railroad.hpp"
#include "models/Property/Street.hpp"
#include "models/Property/Utility.hpp"
#include "placeholder/Configuration.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace {
class BoardInitializedProperty final : public Property {
private:
  int buyPrice;
  int propertyDetail;
  std::string propertyType;

public:
  BoardInitializedProperty(const std::string &code, const std::string &name,
                           int buy, int detail, int mortgage,
                           const std::string &type)
      : Property(code, name, static_cast<PropertyStatus>(0), mortgage, nullptr),
        buyPrice(buy), propertyDetail(detail), propertyType(type) {}

  int getBuyPrice() const override { return buyPrice; }
  int getPropertyDetail() const override { return propertyDetail; }
  void printTitle() const override {}
  void demolish() override {
    Property::setOwner(nullptr);
    Property::setStatusStr("BANK");
  }
  std::string getType() const override { return propertyType; }
};

std::string toLower(const std::string &text) {
  std::string result = text;
  std::transform(
      result.begin(), result.end(), result.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return result;
}

std::string toUpper(const std::string &text) {
  std::string result = text;
  std::transform(
      result.begin(), result.end(), result.begin(),
      [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
  return result;
}

ColorGroup parseColorGroup(const std::string &value) {
  const std::string normalized = toUpper(value);

  if (normalized == "COKLAT")
    return ColorGroup::COKLAT;
  if (normalized == "BIRU_MUDA")
    return ColorGroup::BIRU_MUDA;
  if (normalized == "MERAH_MUDA" || normalized == "PINK")
    return ColorGroup::MERAH_MUDA;
  if (normalized == "ORANGE")
    return ColorGroup::ORANGE;
  if (normalized == "MERAH")
    return ColorGroup::MERAH;
  if (normalized == "KUNING")
    return ColorGroup::KUNING;
  if (normalized == "HIJAU")
    return ColorGroup::HIJAU;
  if (normalized == "BIRU_TUA")
    return ColorGroup::BIRU_TUA;
  return ColorGroup::ABU_ABU;
}

CardDeck<Card> chanceDeck;
CardDeck<Card> communityChestDeck;

template <typename T, typename = void>
struct HasGetChanceCards : std::false_type {};

template <typename T>
struct HasGetChanceCards<
    T, std::void_t<decltype(std::declval<T &>().getChanceCards())>>
    : std::true_type {};

template <typename T, typename = void>
struct HasGetCommunityChestCards : std::false_type {};

template <typename T>
struct HasGetCommunityChestCards<
    T, std::void_t<decltype(std::declval<T &>().getCommunityChestCards())>>
    : std::true_type {};

template <typename T, typename = void> struct HasSetDeck : std::false_type {};

template <typename T>
struct HasSetDeck<T, std::void_t<decltype(std::declval<T &>().setDeck(
                         std::declval<CardDeck<Card> *>()))>> : std::true_type {
};

template <typename T, typename = void>
struct HasSetCardDeck : std::false_type {};

template <typename T>
struct HasSetCardDeck<T, std::void_t<decltype(std::declval<T &>().setCardDeck(
                             std::declval<CardDeck<Card> *>()))>>
    : std::true_type {};

template <typename T, typename = void> struct HasNameField : std::false_type {};

template <typename T>
struct HasNameField<T, std::void_t<decltype(std::declval<T>().name)>>
    : std::true_type {};

template <typename T, typename = void>
struct HasMortgageValueField : std::false_type {};

template <typename T>
struct HasMortgageValueField<
    T, std::void_t<decltype(std::declval<T>().mortgageValue)>>
    : std::true_type {};

template <typename T, typename = void>
struct HasBuyPriceField : std::false_type {};

template <typename T>
struct HasBuyPriceField<T, std::void_t<decltype(std::declval<T>().buyPrice)>>
    : std::true_type {};

template <typename T, typename = void>
struct HasPriceField : std::false_type {};

template <typename T>
struct HasPriceField<T, std::void_t<decltype(std::declval<T>().price)>>
    : std::true_type {};

template <typename T, typename = void>
struct HasPropertyDetailField : std::false_type {};

template <typename T>
struct HasPropertyDetailField<
    T, std::void_t<decltype(std::declval<T>().propertyDetail)>>
    : std::true_type {};

template <typename T, typename = void>
struct HasPropertyTypeField : std::false_type {};

template <typename T>
struct HasPropertyTypeField<
    T, std::void_t<decltype(std::declval<T>().propertyType)>> : std::true_type {
};

template <typename T, typename = void> struct HasTypeField : std::false_type {};

template <typename T>
struct HasTypeField<T, std::void_t<decltype(std::declval<T>().type)>>
    : std::true_type {};

template <typename TileType>
void attachDeckIfSupported(TileType &tile, CardDeck<Card> *deck) {
  if constexpr (HasSetDeck<TileType>::value) {
    tile.setDeck(deck);
  } else if constexpr (HasSetCardDeck<TileType>::value) {
    tile.setCardDeck(deck);
  }
}

template <typename DeckType, typename CardContainer>
void loadDeck(DeckType &deck, const CardContainer &cards) {
  for (const auto &card : cards) {
    deck.addCard(card);
  }
  deck.shuffle();
}

template <typename ConfigType> void initializeCardDecks(ConfigType &config) {
  chanceDeck = CardDeck<Card>();
  communityChestDeck = CardDeck<Card>();

  if constexpr (HasGetChanceCards<ConfigType>::value) {
    loadDeck(chanceDeck, config.getChanceCards());
  }

  if constexpr (HasGetCommunityChestCards<ConfigType>::value) {
    loadDeck(communityChestDeck, config.getCommunityChestCards());
  }
}

template <typename ConfigType>
int getMortgageValueFromConfig(const ConfigType &cfg) {
  if constexpr (HasMortgageValueField<ConfigType>::value) {
    return cfg.mortgageValue;
  }
  return 0;
}

template <typename ConfigType>
int getBuyPriceFromConfig(const ConfigType &cfg) {
  if constexpr (HasBuyPriceField<ConfigType>::value) {
    return cfg.buyPrice;
  }
  if constexpr (HasPriceField<ConfigType>::value) {
    return cfg.price;
  }
  return 0;
}

template <typename ConfigType>
int getPropertyDetailFromConfig(const ConfigType &cfg) {
  if constexpr (HasPropertyDetailField<ConfigType>::value) {
    return cfg.propertyDetail;
  }
  return 0;
}

template <typename ConfigType>
std::string getPropertyTypeFromConfig(const ConfigType &cfg) {
  if constexpr (HasPropertyTypeField<ConfigType>::value) {
    return cfg.propertyType;
  }
  if constexpr (HasTypeField<ConfigType>::value) {
    return cfg.type;
  }
  return "property";
}

template <typename ConfigType>
std::string getPropertyNameFromConfig(const ConfigType &cfg,
                                      const std::string &fallback) {
  if constexpr (HasNameField<ConfigType>::value) {
    if (!cfg.name.empty()) {
      return cfg.name;
    }
  }
  return fallback;
}

Property *buildPropertyFromConfig(const PropertyConfig &cfg,
                                  const Configuration &configuration,
                                  const std::string &fallbackName) {
  const std::string propertyType = toUpper(cfg.propertyType);

  if (propertyType == "STREET") {
    Street *street = new Street(
        cfg.buyPrice, parseColorGroup(cfg.colorGroup), cfg.rentLevels,
        cfg.houseUpgradeCost, cfg.hotelUpgradeCost, BuildingLevel::EMPTY, 1);
    street->setCode(cfg.code);
    street->setName(cfg.name.empty() ? fallbackName : cfg.name);
    street->setMortgageValue(cfg.mortgageValue);
    street->setStatusStr("BANK");
    return street;
  }

  if (propertyType == "RAILROAD") {
    Railroad *railroad = new Railroad(cfg.buyPrice,
                                      configuration.getRailroadRentTable());
    railroad->setCode(cfg.code);
    railroad->setName(cfg.name.empty() ? fallbackName : cfg.name);
    railroad->setMortgageValue(cfg.mortgageValue);
    railroad->setStatusStr("BANK");
    return railroad;
  }

  if (propertyType == "UTILITY") {
    Utility *utility =
        new Utility(cfg.buyPrice, configuration.getUtilityMultiplierTable());
    utility->setCode(cfg.code);
    utility->setName(cfg.name.empty() ? fallbackName : cfg.name);
    utility->setMortgageValue(cfg.mortgageValue);
    utility->setStatusStr("BANK");
    return utility;
  }

  return new BoardInitializedProperty(
      cfg.code, cfg.name.empty() ? fallbackName : cfg.name,
      getBuyPriceFromConfig(cfg), getPropertyDetailFromConfig(cfg),
      getMortgageValueFromConfig(cfg), getPropertyTypeFromConfig(cfg));
}

struct TileInitTracker {
  static int actionCount;
  static int propertyCount;
  static int taxCount;
  static int cardCount;
  static int festivalCount;

  static void reset() {
    actionCount = 0;
    propertyCount = 0;
    taxCount = 0;
    cardCount = 0;
    festivalCount = 0;
  }

  static std::string nextId(char category) {
    int *target = nullptr;
    if (category == 'A') {
      target = &actionCount;
    } else if (category == 'P') {
      target = &propertyCount;
    } else if (category == 'T') {
      target = &taxCount;
    } else if (category == 'C') {
      target = &cardCount;
    } else if (category == 'F') {
      target = &festivalCount;
    }

    if (target == nullptr) {
      throw std::runtime_error("Unknown tile category for id generation.");
    }

    (*target)++;
    std::ostringstream oss;
    oss << category << std::setw(2) << std::setfill('0') << *target;
    return oss.str();
  }
};

int TileInitTracker::actionCount = 0;
int TileInitTracker::propertyCount = 0;
int TileInitTracker::taxCount = 0;
int TileInitTracker::cardCount = 0;
int TileInitTracker::festivalCount = 0;
} // namespace

Board::Board() : tileCount(0) {}

Board::~Board() {
  for (Tile *tile : tiles) {
    delete tile;
  }
  tiles.clear();
}

void Board::initialize(Configuration &config) {
  for (Tile *tile : tiles) {
    delete tile;
  }
  tiles.clear();

  const std::vector<BoardTileConfig> &layout = config.getBoardLayout();

  initializeCardDecks(config);

  const TaxConfig &tax = config.getTaxConfig();
  const int goSalary = config.getGoSalary();
  TileInitTracker::reset();

  tiles.reserve(layout.size());
  for (std::size_t i = 0; i < layout.size(); ++i) {
    const BoardTileConfig &entry = layout[i];
    const std::string type = toLower(entry.type);
    const int pos = static_cast<int>(i);

    if (type == "go") {
      tiles.push_back(
          new GoTile(TileInitTracker::nextId('A'), entry.name, pos, goSalary));
      continue;
    }
    if (type == "jail") {
      tiles.push_back(
          new JailTile(TileInitTracker::nextId('A'), entry.name, pos));
      continue;
    }
    if (type == "go_to_jail") {
      tiles.push_back(
          new GoToJailTile(TileInitTracker::nextId('A'), entry.name, pos));
      continue;
    }
    if (type == "free_parking") {
      tiles.push_back(
          new FreeParkingTile(TileInitTracker::nextId('A'), entry.name, pos));
      continue;
    }
    if (type == "chance") {
      ChanceTile *chanceTile =
          new ChanceTile(TileInitTracker::nextId('C'), entry.name, pos);
      attachDeckIfSupported(*chanceTile, &chanceDeck);
      tiles.push_back(chanceTile);
      continue;
    }
    if (type == "community_chest") {
      CommunityChestTile *communityChestTile =
          new CommunityChestTile(TileInitTracker::nextId('C'), entry.name, pos);
      attachDeckIfSupported(*communityChestTile, &communityChestDeck);
      tiles.push_back(communityChestTile);
      continue;
    }
    if (type == "festival") {
      tiles.push_back(
          new FestivalTile(TileInitTracker::nextId('F'), entry.name, pos));
      continue;
    }
    if (type == "pph_tax" || type == "pph") {
      tiles.push_back(new PPHTaxTile(TileInitTracker::nextId('T'), entry.name,
                                     pos, tax.pphFlat, tax.pphPercentage));
      continue;
    }
    if (type == "pbm_tax" || type == "pbm") {
      tiles.push_back(new PBMTaxTile(TileInitTracker::nextId('T'), entry.name,
                                     pos, tax.pbmFlat));
      continue;
    }
    if (type == "property") {
      const std::string refCode =
          entry.propertyCode.empty() ? entry.code : entry.propertyCode;
      PropertyConfig *propConfig = config.getPropertyConfig(refCode);
      Property *property = nullptr;

      if (propConfig != nullptr) {
        property = buildPropertyFromConfig(*propConfig, config, entry.name);
      } else {
        BoardInitializedProperty *fallbackProperty =
            new BoardInitializedProperty(refCode, entry.name, 0, 0, 0,
                                         "property");
        property = fallbackProperty;
      }

      const std::string tileName =
          (propConfig != nullptr)
              ? getPropertyNameFromConfig(*propConfig, entry.name)
              : entry.name;
      tiles.push_back(new PropertyTile(TileInitTracker::nextId('P'), tileName,
                                       pos, property));
      continue;
    }

    throw std::runtime_error("Unknown board tile type: " + entry.type);
  }
  tileCount = static_cast<int>(tiles.size());
}

void Board::setTiles(std::vector<Tile *> newTiles) {
  for (Tile *tile : tiles) {
    delete tile;
  }

  tiles = std::move(newTiles);
  tileCount = static_cast<int>(tiles.size());
}

Tile &Board::getTile(int pos) {
  if (pos < 0 || pos >= static_cast<int>(tiles.size())) {
    throw std::out_of_range("Board::getTile position out of range");
  }
  return *tiles[static_cast<std::size_t>(pos)];
}

const Tile &Board::getTile(int pos) const {
  if (pos < 0 || pos >= static_cast<int>(tiles.size())) {
    throw std::out_of_range("Board::getTile position out of range");
  }
  return *tiles[static_cast<std::size_t>(pos)];
}

Tile *Board::getTilebyCode(const std::string &code) {
  for (Tile *tile : tiles) {
    if (tile->getCode() == code) {
      return tile;
    }
  }
  return nullptr;
}

int Board::getTileCount() const { return tileCount; }

int Board::findNearestStation(int currentPos) const {
  int bestPos = -1;
  int firstStation = -1;
  for (int i = 0; i < tileCount; ++i) {
    if (tiles[i] == nullptr)
      continue;

    const PropertyTile *propertyTile = dynamic_cast<const PropertyTile *>(tiles[i]);
    if (propertyTile == nullptr) {
      continue;
    }

    if (toUpper(propertyTile->getProperty().getType()) == "RAILROAD") {
      if (firstStation < 0)
        firstStation = i;
      if (i > currentPos && bestPos < 0) {
        bestPos = i;
      }
    }
  }
  // If no station ahead, wrap around to first station
  if (bestPos < 0)
    bestPos = firstStation;
  // If no stations at all, stay in place
  if (bestPos < 0)
    bestPos = currentPos;
  return bestPos;
}

int Board::findJailPosition() const {
  for (int i = 0; i < tileCount; ++i) {
    if (tiles[i] == nullptr)
      continue;
    const std::string tileType = toLower(tiles[i]->getType());
    const std::string tileCode = toLower(tiles[i]->getCode());
    const std::string tileName = toLower(tiles[i]->getName());

    if (tileType == "jail" || tileType == "penjara" || tileCode == "pen" ||
        tileName == "penjara") {
      return i;
    }

    const auto *actionTile = dynamic_cast<const ActionTile *>(tiles[i]);
    if (actionTile != nullptr &&
        toLower(actionTile->getActionType()) == "jail") {
      return i;
    }
  }
  return 10; // Fallback default
}
