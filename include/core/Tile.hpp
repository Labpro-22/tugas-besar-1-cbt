#ifndef TILE_HPP
#define TILE_HPP

#include <string>
#include <vector>
#include <algorithm>

// Forward declarations
class Player;
class GameManager;

class Tile {
  protected:
    // Atribut kelas
    std::string id;
    std::string name;
    int position;
    std::string tile_type;
    std::vector<Player *> on_tile;

  public:
    // Konstruktor dan Destruktor
    Tile(const std::string &code, const std::string &name, int pos, const std::string &type = "base");
    virtual ~Tile() = default;

    // Getters
    std::string getCode() const;
    std::string getName() const;
    int getPosition() const;
    bool isLanded() const;
    std::string getType() const;
    std::vector<Player *> getPlayerList() const;

    // Handler untuk menambah dan mengurangi player pada tile
    void addPlayer(Player &player);
    void removePlayer(Player &player);

    // Handler untuk tiap tile
    virtual void onLanded(Player &player, GameManager &game) = 0;
};

#endif
