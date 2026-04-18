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
    if (p == nullptr || gm == nullptr) {
        return;
    }

    if (communityType != CommunityCardType::BIRTHDAY && p->hasShieldActive()) {
        gm->addLogEntry(p->getUsername() + " terlindungi shield dari efek kartu community chest");
        return;
    }

    if (communityType == CommunityCardType::BIRTHDAY) {
        const int giftAmount = 100;

        for (Player& other : gm->getPlayers()) {
            if (&other == p || other.getStatus() != ACTIVE) {
                continue;
            }

            if (other.canPay(giftAmount)) {
                other.reduceCash(giftAmount);
                p->addCash(giftAmount);
            } else {
                gm->executeBankruptcy(other, *p, giftAmount);
            }
        }
        gm->addLogEntry(p->getUsername() + " menerima hadiah ulang tahun");
        return;
    }

    if (communityType == CommunityCardType::DOCTOR_FEE) {
        gm->executeTaxPayment(*p, 700, true);
        gm->addLogEntry(p->getUsername() + " membayar doctor fee");
        return;
    }

    if (communityType == CommunityCardType::CAMPAIGN_FEE) {
        const int campaignFee = 200;
        for (Player& other : gm->getPlayers()) {
            if (&other == p || other.getStatus() != ACTIVE) {
                continue;
            }

            if (p->canPay(campaignFee)) {
                p->reduceCash(campaignFee);
                other.addCash(campaignFee);
            } else {
                gm->executeBankruptcy(*p, other, campaignFee);
                break;
            }
        }
        gm->addLogEntry(p->getUsername() + " membayar campaign fee ke pemain lain");
    }
}