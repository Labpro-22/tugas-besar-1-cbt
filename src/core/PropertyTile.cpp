#include "core/PropertyTile.hpp"

PropertyTile::PropertyTile(const std::string &code, const std::string &name,
                           int pos, Property *prop)
    : Tile(code, name, pos), property(prop), festivalMultiplier(1),
      festivalDuration(0) {
  tile_type = "property";
}

void PropertyTile::applyFestivalEffect(int mult, int dur) {
  festivalMultiplier = mult;
  festivalDuration = dur;
}

void PropertyTile::checkFestivalEffect() {
  if (festivalDuration > 0) {
    festivalDuration--;
    if (festivalDuration == 0) {
      festivalMultiplier = 1;
    }
  }
}

Property &PropertyTile::getProperty() { return *property; }

void PropertyTile::onLanded(Player & /*player*/, GameManager & /*game*/) {
  // Implementasi override fungsi onLanded
}
