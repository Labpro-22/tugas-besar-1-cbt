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
#include "models/Property/Property.hpp"
#include "models/Property/Railroad.hpp"
#include "models/Property/Street.hpp"
#include "models/Property/Utility.hpp"
#include "data/Configuration.hpp"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {

std::string toUpper(std::string text) {
  std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
    return static_cast<char>(std::toupper(ch));
  });
  return text;
}

std::string toLower(std::string text) {
  std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return text;
}

ColorGroup parseColorGroup(const std::string &colorGroup) {
  const std::string color = toUpper(colorGroup);
  if (color == "COKLAT") return ColorGroup::COKLAT;
  if (color == "BIRU_MUDA") return ColorGroup::BIRU_MUDA;
  if (color == "MERAH_MUDA" || color == "PINK") return ColorGroup::MERAH_MUDA;
  if (color == "ORANGE") return ColorGroup::ORANGE;
  if (color == "MERAH") return ColorGroup::MERAH;
  if (color == "KUNING") return ColorGroup::KUNING;
  if (color == "HIJAU") return ColorGroup::HIJAU;
  if (color == "BIRU_TUA") return ColorGroup::BIRU_TUA;
  if (color == "ABU_ABU" || color == "ABU-ABU") return ColorGroup::ABU_ABU;
  return ColorGroup::ABU_ABU;
}

std::string getPropertyNameFromConfig(const PropertyConfig &cfg,
                                      const std::string &fallbackName) {
  return cfg.name.empty() ? fallbackName : cfg.name;
}

static int gActionCount = 0;
static int gPropertyCount = 0;
static int gTaxCount = 0;
static int gCardCount = 0;
static int gFestivalCount = 0;

static void resetTileCounters() {
  gActionCount = 0;
  gPropertyCount = 0;
  gTaxCount = 0;
  gCardCount = 0;
  gFestivalCount = 0;
}

static std::string nextTileId(char category) {
  int *target = nullptr;
  if (category == 'A')
    target = &gActionCount;
  else if (category == 'P')
    target = &gPropertyCount;
  else if (category == 'T')
    target = &gTaxCount;
  else if (category == 'C')
    target = &gCardCount;
  else if (category == 'F')
    target = &gFestivalCount;

  if (target == nullptr) {
    throw std::runtime_error("Unknown tile category for id generation.");
  }

  ++(*target);
  std::ostringstream oss;
  oss << category << std::setw(2) << std::setfill('0') << *target;
  return oss.str();
}

}  // namespace

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

  throw std::runtime_error("Unknown property type in configuration: " +
                           cfg.propertyType);
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

  const TaxConfig &tax = config.getTaxConfig();
  const int goSalary = config.getGoSalary();
  resetTileCounters();

  tiles.reserve(layout.size());
  for (std::size_t i = 0; i < layout.size(); ++i) {
    const BoardTileConfig &entry = layout[i];
    const std::string type = toLower(entry.type);
    const int pos = static_cast<int>(i);

    if (type == "go") {
      tiles.push_back(
          new GoTile(nextTileId('A'), entry.name, pos, goSalary));
      continue;
    }
    if (type == "jail") {
      tiles.push_back(
          new JailTile(nextTileId('A'), entry.name, pos));
      continue;
    }
    if (type == "go_to_jail") {
      tiles.push_back(
          new GoToJailTile(nextTileId('A'), entry.name, pos));
      continue;
    }
    if (type == "free_parking") {
      tiles.push_back(
          new FreeParkingTile(nextTileId('A'), entry.name, pos));
      continue;
    }
    if (type == "chance") {
      ChanceTile *chanceTile =
          new ChanceTile(nextTileId('C'), entry.name, pos);
      tiles.push_back(chanceTile);
      continue;
    }
    if (type == "community_chest") {
      CommunityChestTile *communityChestTile =
          new CommunityChestTile(nextTileId('C'), entry.name, pos);
      tiles.push_back(communityChestTile);
      continue;
    }
    if (type == "festival") {
      tiles.push_back(
          new FestivalTile(nextTileId('F'), entry.name, pos));
      continue;
    }
    if (type == "pph_tax" || type == "pph") {
      tiles.push_back(new PPHTaxTile(nextTileId('T'), entry.name,
                                     pos, tax.pphFlat, tax.pphPercentage));
      continue;
    }
    if (type == "pbm_tax" || type == "pbm") {
      tiles.push_back(new PBMTaxTile(nextTileId('T'), entry.name,
                                     pos, tax.pbmFlat));
      continue;
    }
    if (type == "property") {
      const std::string refCode =
          entry.propertyCode.empty() ? entry.code : entry.propertyCode;
      PropertyConfig *propConfig = config.getPropertyConfig(refCode);
      if (propConfig == nullptr) {
        throw std::runtime_error("Missing property config for code: " + refCode);
      }

      Property *property = buildPropertyFromConfig(*propConfig, config, entry.name);
      const std::string tileName = getPropertyNameFromConfig(*propConfig, entry.name);
      tiles.push_back(new PropertyTile(nextTileId('P'), tileName, pos, property));
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

int Board::findGoPosition() const {
  for (int i = 0; i < tileCount; ++i) {
    if (tiles[i] == nullptr) continue;
    if (toLower(tiles[i]->getType()) != "action") continue;

    const ActionTile *actionTile = static_cast<const ActionTile *>(tiles[i]);
    if (toLower(actionTile->getActionType()) == "go") {
      return i;
    }
  }
  throw std::runtime_error("GO tile not found in board layout.");
}

int Board::findNearestStation(int currentPos) const {
  int bestPos = -1;
  int firstStation = -1;

  for (int i = 0; i < tileCount; ++i) {
    if (tiles[i] == nullptr) continue;
    if (toLower(tiles[i]->getType()) != "property") continue;

    const PropertyTile *propertyTile = static_cast<const PropertyTile *>(tiles[i]);
    if (toUpper(propertyTile->getProperty().getType()) == "RAILROAD") {
      if (firstStation < 0) firstStation = i;
      if (i > currentPos && bestPos < 0) bestPos = i;
    }
  }

  if (bestPos < 0) bestPos = firstStation;
  if (bestPos < 0) {
    throw std::runtime_error("No station tile found in board layout.");
  }
  return bestPos;
}

int Board::findJailPosition() const {
  for (int i = 0; i < tileCount; ++i) {
    if (tiles[i] == nullptr) continue;

    const std::string tileType = toLower(tiles[i]->getType());
    const std::string tileCode = toLower(tiles[i]->getCode());
    const std::string tileName = toLower(tiles[i]->getName());

    if (tileType == "jail" || tileType == "penjara" || tileCode == "pen" || tileName == "penjara") {
      return i;
    }

    if (tileType == "action") {
      const ActionTile *actionTile = static_cast<const ActionTile *>(tiles[i]);
      if (toLower(actionTile->getActionType()) == "jail") {
        return i;
      }
    }
  }
  throw std::runtime_error("Jail tile not found in board layout.");
}
