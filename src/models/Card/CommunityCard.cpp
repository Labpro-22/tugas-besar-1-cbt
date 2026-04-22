#include "../../../include/models/Card/CommunityCard.hpp"
#include "../../../include/models/GameManager/GameManager.hpp"
#include "../../../include/models/GameManager/Player.hpp"

// ctor
CommunityCard::CommunityCard(int cardId, CommunityCardType type)
    : ActionCard(cardId), communityType(type) {}

// dtor
CommunityCard::~CommunityCard() {}

// Must implement functions
std::string CommunityCard::getDescription() const {
  switch (communityType) {
  case CommunityCardType::BIRTHDAY:
    return "Ini adalah hari ulang tahun Anda. Dapatkan M100 dari setiap pemain.";
  case CommunityCardType::DOCTOR_FEE:
    return "Biaya dokter. Bayar M700.";
  case CommunityCardType::CAMPAIGN_FEE:
    return "Anda mau nyaleg. Bayar M200 kepada setiap pemain.";
  default:
    return "Kartu dana umum tidak dikenal.";
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
    const int doctorFee = 700;
    if (p->canPay(doctorFee)) {
      p->reduceCash(doctorFee);
      std::cout << "Kamu membayar M" << doctorFee << " ke Bank."
                << " Sisa Uang = M" << p->getCash() << ".\n";
      gm->addLogEntry(p->getUsername() + " membayar biaya dokter M" + std::to_string(doctorFee));
    } else {
      std::cout << "Kamu tidak mampu membayar biaya dokter! (M" << doctorFee << ")\n";
      std::cout << "Uang kamu saat ini: M" << p->getCash() << "\n";
      gm->executeBankruptcy(*p, nullptr, doctorFee);
    }
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
