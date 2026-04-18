#include "core/Board.hpp"
#include "core/ChanceTile.hpp"
#include "core/CommunityChestTile.hpp"
#include "core/FestivalTile.hpp"
#include "core/FreeParkingTile.hpp"
#include "core/GoTile.hpp"
#include "core/GoToJailTile.hpp"
#include "core/JailTile.hpp"
#include "core/PBMTaxTile.hpp"
#include "core/PPHTaxTile.hpp"
#include "core/PropertyTile.hpp"
#include "core/Tile.hpp"
#include "models/Card.hpp"
#include "models/CardDeck.hpp"
#include "models/Configuration.hpp"
#include "models/Property.hpp"

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
      BoardInitializedProperty(const std::string &code, const std::string &name, int buy, int detail, int mortgage, const std::string &type)
        : Property(code, name, static_cast<PropertyStatus>(0), mortgage, nullptr), buyPrice(buy), propertyDetail(detail), propertyType(type) {}

      int getBuyPrice() const override { return buyPrice; }
      int getPropertyDetail() const override { return propertyDetail; }
      void printTitle() const override {}
      void demolish() override {
        setOwner(nullptr);
        setStatus(static_cast<PropertyStatus>(0));
      }
      std::string getType() const override { return propertyType; }
    };

  std::string toLower(const std::string &text) {
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
  }

  CardDeck<Card> chanceDeck;
  CardDeck<Card> communityChestDeck;

  template <typename T, typename = void>
  struct HasGetChanceCards : std::false_type {};

  template <typename T>
  struct HasGetChanceCards<T, std::void_t<decltype(std::declval<T &>().getChanceCards())>> : std::true_type {};

  template <typename T, typename = void>
  struct HasGetCommunityChestCards : std::false_type {};

  template <typename T>
  struct HasGetCommunityChestCards<T, std::void_t<decltype(std::declval<T &>().getCommunityChestCards())>> : std::true_type {};

  template <typename T, typename = void>
  struct HasSetDeck : std::false_type {};

  template <typename T>
  struct HasSetDeck<T, std::void_t<decltype(std::declval<T &>().setDeck(std::declval<CardDeck<Card> *>()))>> : std::true_type {};

  template <typename T, typename = void>
  struct HasSetCardDeck : std::false_type {};

  template <typename T>
  struct HasSetCardDeck<T, std::void_t<decltype(std::declval<T &>().setCardDeck(std::declval<CardDeck<Card> *>()))>> : std::true_type {};

  template <typename T, typename = void>
  struct HasNameField : std::false_type {};

  template <typename T>
  struct HasNameField<T, std::void_t<decltype(std::declval<T>().name)>> : std::true_type {};

  template <typename T, typename = void>
  struct HasMortgageValueField : std::false_type {};

  template <typename T>
  struct HasMortgageValueField<T, std::void_t<decltype(std::declval<T>().mortgageValue)>> : std::true_type {};

  template <typename T, typename = void>
  struct HasBuyPriceField : std::false_type {};

  template <typename T>
  struct HasBuyPriceField<T, std::void_t<decltype(std::declval<T>().buyPrice)>> : std::true_type {};

  template <typename T, typename = void>
  struct HasPriceField : std::false_type {};

  template <typename T>
  struct HasPriceField<T, std::void_t<decltype(std::declval<T>().price)>> : std::true_type {};

  template <typename T, typename = void>
  struct HasPropertyDetailField : std::false_type {};

  template <typename T>
  struct HasPropertyDetailField<T, std::void_t<decltype(std::declval<T>().propertyDetail)>> : std::true_type {};

  template <typename T, typename = void>
  struct HasPropertyTypeField : std::false_type {};

  template <typename T>
  struct HasPropertyTypeField<T, std::void_t<decltype(std::declval<T>().propertyType)>> : std::true_type {};

  template <typename T, typename = void>
  struct HasTypeField : std::false_type {};

  template <typename T>
  struct HasTypeField<T, std::void_t<decltype(std::declval<T>().type)>> : std::true_type {};

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

  template <typename ConfigType>
  void initializeCardDecks(ConfigType &config) {
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
  std::string getPropertyNameFromConfig(const ConfigType &cfg, const std::string &fallback) {
    if constexpr (HasNameField<ConfigType>::value) {
      if (!cfg.name.empty()) {
        return cfg.name;
      }
    }
    return fallback;
  }

  template <typename ConfigType>
  Property *buildPropertyFromConfig(const std::string &code,
                                    const std::string &fallbackName,
                                    const ConfigType &cfg) {
    return new BoardInitializedProperty(
        code,
        getPropertyNameFromConfig(cfg, fallbackName),
        getBuyPriceFromConfig(cfg),
        getPropertyDetailFromConfig(cfg),
        getMortgageValueFromConfig(cfg),
        getPropertyTypeFromConfig(cfg));
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
      } 
      else if (category == 'P') {
        target = &propertyCount;
      } 
      else if (category == 'T') {
        target = &taxCount;
      } 
      else if (category == 'C') {
        target = &cardCount;
      } 
      else if (category == 'F') {
        target = &festivalCount;
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
}

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

  int goCount = 0;
  int jailCount = 0;
  for (const BoardTileConfig &entry : layout) {
    const std::string type = toLower(entry.type);
    if (type == "go") goCount++;
    if (type == "jail") jailCount++;

    if (type == "property") {
      const std::string refCode = entry.propertyCode.empty() ? entry.code : entry.propertyCode;
    }
  }

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
      tiles.push_back( new GoTile(TileInitTracker::nextId('A'), entry.name, pos, goSalary));
      continue;
    }
    if (type == "jail") {
      tiles.push_back(new JailTile(TileInitTracker::nextId('A'), entry.name, pos));
      continue;
    }
    if (type == "go_to_jail") {
      tiles.push_back(new GoToJailTile(TileInitTracker::nextId('A'), entry.name, pos));
      continue;
    }
    if (type == "free_parking") {
      tiles.push_back(new FreeParkingTile(TileInitTracker::nextId('A'), entry.name, pos));
      continue;
    }
    if (type == "chance") {
      ChanceTile *chanceTile = new ChanceTile(TileInitTracker::nextId('C'), entry.name, pos);
      attachDeckIfSupported(*chanceTile, &chanceDeck);
      tiles.push_back(chanceTile);
      continue;
    }
    if (type == "community_chest") {
      CommunityChestTile *communityChestTile = new CommunityChestTile(TileInitTracker::nextId('C'), entry.name, pos);
      attachDeckIfSupported(*communityChestTile, &communityChestDeck);
      tiles.push_back(communityChestTile);
      continue;
    }
    if (type == "festival") {
      tiles.push_back(new FestivalTile(TileInitTracker::nextId('F'), entry.name, pos));
      continue;
    }
    if (type == "pph_tax" || type == "pph") {
      tiles.push_back(new PPHTaxTile(TileInitTracker::nextId('T'), entry.name, pos, tax.pphFlat, tax.pphPercentage));
      continue;
    }
    if (type == "pbm_tax" || type == "pbm") {
      tiles.push_back(new PBMTaxTile(TileInitTracker::nextId('T'), entry.name, pos, tax.pbmFlat));
      continue;
    }
    if (type == "property") {
      const std::string refCode = entry.propertyCode.empty() ? entry.code : entry.propertyCode;
      PropertyConfig *propConfig = config.getPropertyConfig(refCode);
      Property *property = nullptr;

      if (propConfig != nullptr) {
        property = buildPropertyFromConfig(refCode, entry.name, *propConfig);
      } else {
        BoardInitializedProperty *fallbackProperty = new BoardInitializedProperty(
            refCode,
            entry.name,
            0,
            0,
            0,
            "property");
        property = fallbackProperty;
      }

      const std::string tileName = (propConfig != nullptr)
                                       ? getPropertyNameFromConfig(*propConfig, entry.name)
                                       : entry.name;
      tiles.push_back(new PropertyTile(TileInitTracker::nextId('P'), tileName, pos, property));
      continue;
    }
  }
  tileCount = static_cast<int>(tiles.size());
}

Tile &Board::getTile(int pos) { return *tiles[pos]; }

Tile *Board::getTilebyCode(const std::string &code) {
  for (Tile *tile : tiles) {
    if (tile->getCode() == code) {
      return tile;
    }
  }
  return nullptr;
}

int Board::getTileCount() const { return tileCount; }
