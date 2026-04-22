#ifndef PROPERTY_VIEW_HPP
#define PROPERTY_VIEW_HPP

#include <map>
#include <string>
#include <vector>

class Player;
class Property;
class InputHandler;

class PropertyView {
private:
    int cardWidth;

public:
    PropertyView(int cardWidth = 32);

    void showDeed(Property* property) const;
    void showPlayerProperties(Player* player) const;
    bool showBuyPrompt(Property* property, int playerCash) const;
    bool showBuyPrompt(Property* property, int playerCash, InputHandler& input) const;
    void showMortgageOptions(const std::vector<Property*>& properties) const;
    void showRedeemOptions(const std::vector<Property*>& mortgaged) const;
    void showBuildOptions(const std::map<std::string, std::vector<Property*>>& buildableGroups) const;
};

#endif
