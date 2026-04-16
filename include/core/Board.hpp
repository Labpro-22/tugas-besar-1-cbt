#ifndef BOARD_HPP
#define BOARD_HPP

#include <string>
#include <vector>

// Deklarasi terlebih dahulu
class Tile;
class Configuration;

class Board {
private:
  // Atribut kelas
  std::vector<Tile *> tiles;
  int tileCount;

public:
  // Konstruktor dan destruktor
  Board();
  ~Board();

  // Method inisiasi data board
  void initialize(Configuration &config);

  // Getter tile
  Tile &getTile(int pos);
  Tile *getTilebyCode(const std::string &code);
  int getTileCount() const;
};

#endif
