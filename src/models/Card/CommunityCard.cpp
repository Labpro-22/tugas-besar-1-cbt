#include "CommunityCard.hpp"
#include "../GameManager/GameManager.hpp"

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
    if (p == nullptr) {
        return;
    }

    // Kartu lainnya merugikan (gunakan shield jika ada)
    if (communityType != CommunityCardType::BIRTHDAY && p->hasShieldActive()) {
        p->deactivateShield();
        return;
    }

    switch (communityType) {
        case CommunityCardType::BIRTHDAY:
            if (gm == nullptr) {
                return;
            }

            for (Player& other : gm->getPlayers()) {
                // skip player yang bangkrut
                if (&other == p || other.getStatus() == BANKRUPT) {
                    continue;
                }

                // Ambil 100 dari pemain lain (jika kurang, ambil seadanya)
                int contribution = other.getCash() < 100 ? other.getCash() : 100;
                if (contribution > 0) {
                    other.reduceCash(contribution);
                    p->addCash(contribution);
                }
            }

            break;

        case CommunityCardType::DOCTOR_FEE:
            p->reduceCash(700);
            break;
            
        case CommunityCardType::CAMPAIGN_FEE:

            // Kasih 100 ke semua pemain
            for (Player& other : gm->getPlayers()) {
                // skip player yang bangkrut
                if (&other == p || other.getStatus() == BANKRUPT) {
                    continue;
                }
            
                if (p->getCash() > 100) {
                    other.addCash(100);
                    p->reduceCash(100);
                } else if (p->getCash() > 0 && p->getCash() < 100){
                    other.addCash(p->getCash());
                    p->reduceCash(p->getCash());
                } 
                
                // do nothing kalau uang habis
            }

            break;
    }
}