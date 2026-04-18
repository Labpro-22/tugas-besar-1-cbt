#ifndef STATUS_VIEW_HPP
#define STATUS_VIEW_HPP

#include <string>
#include <vector>

// Sesuaikan path ini dengan posisi header timmu
#include "models/GameManager/Player.hpp"
#include "models/Property/Property.hpp"
#include "models/Card/Card.hpp"

class StatusView {
public:
    void showDiceResult(int d1, int d2, const std::string& destination) const;
    void showRentPayment(Player* player, Player* owner, int amount, Property* property) const;
    void showTaxPayment(Player* player, int amount) const;
    void showFestivalEffect(Property* property, int multiplier, int duration) const;
    void showCardUsage(Player* player, Card* card) const;
    void showBankruptcy(Player* debtor, Player* creditor) const;
    void showVictory(const std::vector<Player*>& winners) const;
};

#endif