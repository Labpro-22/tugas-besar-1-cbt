#include "CommunityCard.hpp"
#include "../GameManager/GameManager.hpp"
#include "../GameManager/Player.hpp"

// ctor
CommunityCard::CommunityCard(int cardId, CommunityCardType type)
    : ActionCard(cardId), communityType(type) {}

// dtor
CommunityCard::~CommunityCard() {}

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

std::string CommunityCard::getType() const { return "CommunityCard"; }

void CommunityCard::execute(Player *p, GameManager *gm) {
  if (p == nullptr || gm == nullptr) {
    return;
  }

  if (communityType != CommunityCardType::BIRTHDAY && p->hasShieldActive()) {
    gm->addLogEntry(p->getUsername() +
                    " terlindungi shield dari efek kartu community chest");
    return;
  }

  if (communityType == CommunityCardType::BIRTHDAY) {
    // Spesifikasi: "Dapatkan M100 dari setiap pemain"
    const int giftAmount = 100;

    for (Player &other : gm->getPlayers()) {
      if (&other == p || other.getStatus() == BANKRUPT) {
        continue;
      }

      if (other.canPay(giftAmount)) {
        other.reduceCash(giftAmount);
        p->addCash(giftAmount);
      } else {
        gm->executeBankruptcy(other, p, giftAmount);
      }
    }
    gm->addLogEntry(p->getUsername() + " menerima hadiah ulang tahun");
    return;
  }

  if (communityType == CommunityCardType::DOCTOR_FEE) {
    // Spesifikasi: "Biaya dokter. Bayar M700"
    const int doctorFee = 700;
    gm->executeTaxPayment(*p, doctorFee, true);
    gm->addLogEntry(p->getUsername() + " membayar doctor fee");
    return;
  }

  if (communityType == CommunityCardType::CAMPAIGN_FEE) {
    // Spesifikasi: "Bayar M200 kepada setiap pemain"
    const int campaignFee = 200;
    for (Player &other : gm->getPlayers()) {
      if (&other == p || other.getStatus() == BANKRUPT) {
        continue;
      }

      if (p->canPay(campaignFee)) {
        p->reduceCash(campaignFee);
        other.addCash(campaignFee);
      } else {
        gm->executeBankruptcy(*p, &other, campaignFee);
        break;
      }
    }
    gm->addLogEntry(p->getUsername() + " membayar campaign fee ke pemain lain");
  }
}
