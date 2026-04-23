#ifndef BOARD_HPP
#define BOARD_HPP

#include "data/Configuration.hpp"
#include "models/Property/ColorGroup.hpp"
#include "models/Property/Property.hpp"
#include <string>
#include <vector>

class Tile;

class Board {
private:
  // Atribut kelas
  std::vector<Tile *> tiles;
  int tileCount;
  static int actionCount;
  static int propertyCount;
  static int taxCount;
  static int cardCount;
  static int festivalCount;

  static std::string toUpper(std::string text);
  static std::string toLower(std::string text);
  static ColorGroup parseColorGroup(const std::string &colorGroup);
  static std::string getPropertyNameFromConfig(const PropertyConfig &cfg,
                                               const std::string &fallbackName);
  static Property *buildPropertyFromConfig(const PropertyConfig &cfg,
                                           const Configuration &configuration,
                                           const std::string &fallbackName);
  static void resetTileCounters();
  static std::string nextTileId(char category);

public:
  // Konstruktor dan destruktor
  Board();
  ~Board();

  // Method inisiasi data board
  void initialize(Configuration &config);
  void setTiles(std::vector<Tile *> newTiles);

  // Getter tile
  Tile &getTile(int pos);
  const Tile &getTile(int pos) const;
  Tile *getTilebyCode(const std::string &code);
  int getTileCount() const;

  // Dynamic board query helpers
  int findGoPosition() const;
  int findNearestStation(int currentPos) const;
  int findJailPosition() const;
};

#endif
