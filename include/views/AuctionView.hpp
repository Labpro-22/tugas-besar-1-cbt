#ifndef AUCTION_VIEW_HPP
#define AUCTION_VIEW_HPP

#include <string>
#include <vector>

#include "models/GameManager/Player.hpp"
#include "models/Property/Property.hpp"
#include "models/Command.hpp"

class AuctionView {
private:
    int highestBidShown;
    std::string currentPropertyCode;

public:
    AuctionView();

    void showAuctionStart(Property* property, const std::vector<Player*>& order);
    Command promptAuctionAction(Player* player, int currentBid);
    void showBidUpdate(Player* bidder, int amount);
    void showAuctionWinner(Player* winner, int amount, Property* property);
};

#endif