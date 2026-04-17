#include "CommunityCard.hpp"

// ctor
CommunityCard::CommunityCard(int cardId, CommunityCardType type) : ActionCard(cardId), communityType(type) {}

// dtor
CommunityCard::~CommunityCard(){}

// Must implement functions
std::string CommunityCard::getDescription() const {
    switch (communityType) {
        case CommunityCardType::BIRTHDAY:
            return "It is your birthday. Collect money from each player.";
        case CommunityCardType::DOCTOR_FEE:
            return "Doctor's fee. Pay treatment costs.";
        case CommunityCardType::CAMPAIGN_FEE:
            return "Campaign fee is due. Pay campaign contribution.";
        default:
            return "Unknown community card.";
    }
}

std::string CommunityCard::getType() const {
    return "CommunityCard";
}

void CommunityCard::execute(Player* p, GameManager* gm) {
    (void)p;
    (void)gm;
    // implement effect through GameManager (later)
}