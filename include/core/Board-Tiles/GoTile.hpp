#ifndef GO_TILE_HPP
#define GO_TILE_HPP

#include "ActionTile.hpp"

class GoTile : public ActionTile {
    private:
        // Atribut
        int salary;

    public:
        // Konstruktor dan destruktor
        GoTile(const std::string &code, const std::string &name, int pos, int salary);
        ~GoTile() = default;

        // Handler untuk kasus apabila player melewati atau berdiam di petak go
        void onLanded(Player &player, GameManager &game) override;
        void onPassed(Player &player, GameManager &game);

        // Mengambil besar duit yang diterima player
        int getSalary() const;
};

#endif
