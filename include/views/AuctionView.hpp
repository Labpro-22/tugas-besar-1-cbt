#ifndef AUCTION_VIEW_HPP
#define AUCTION_VIEW_HPP

#include <string>
#include <vector>

#include "../models/Command.hpp"

class Player;
class Property;
class InputHandler;

class AuctionView {
private:
    int highestBidShown;
    std::string currentPropertyCode;

public:
    AuctionView();

    void showAuctionStart(Property* property, const std::vector<Player*>& order);
    Command promptAuctionAction(Player* player, int currentBid);
    Command promptAuctionAction(Player* player, int currentBid, InputHandler& input);
    void showBidUpdate(Player* bidder, int amount);
    void showAuctionWinner(Player* winner, int amount, Property* property);
};

#endif
