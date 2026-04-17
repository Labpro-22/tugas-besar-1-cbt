#pragma once
#include "ActionCard.hpp"
#include "CommunityCardType.hpp"

class CommunityCard : public ActionCard {
    private:
        CommunityCardType communityType;
    public:
        // ctor
        CommunityCard(int cardId, CommunityCardType type);

        // dtor
        ~CommunityCard();

        // Must implement functions
        std::string getDescription() const override;
        std::string getType() const override;
        void execute(Player* p, GameManager* gm) override;
        int getValue() const override {
            return 0;
        }
};