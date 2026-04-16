#include "core/Board.hpp"
#include "core/Tile.hpp"

Board::Board() : tileCount(0) {}

Board::~Board() {
  for (Tile *tile : tiles) {
    delete tile;
  }
  tiles.clear();
}

void Board::initialize(Configuration &config) {
  // Implementasi menyesuaikan config nantinya
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
