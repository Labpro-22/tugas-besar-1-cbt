#include "models/Card/CommunityCard.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"
#include "exception/NimonspoliExceptions.hpp"

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
    throw InternalGameException("CommunityCard::execute menerima konteks yang tidak valid.");
  }

  if (communityType != CommunityCardType::BIRTHDAY && p->hasShieldActive()) {
    gm->getLogger().log(gm->getCurrentTurn(), p->getUsername(), "SHIELD", "ShieldCard melindungi dari efek Dana Umum");
    return;
  }

  if (communityType == CommunityCardType::BIRTHDAY) {
    const int giftAmount = 100;

    for (Player &other : gm->getPlayers()) {
      if (&other == p || other.getStatus() == BANKRUPT) {
        continue;
      }

      try {
        if (!other.canPay(giftAmount)) {
          throw InsufficientFundsException(other.getUsername(), giftAmount, other.getCash());
        }
        other.reduceCash(giftAmount);
        p->addCash(giftAmount);
      } catch (const InsufficientFundsException &) {
        gm->executeBankruptcy(other, p, giftAmount);
      }
    }
    gm->getLogger().log(gm->getCurrentTurn(), p->getUsername(), "KARTU", "Dana Umum: Menerima hadiah ulang tahun M100 dari setiap pemain");
    return;
  }

  if (communityType == CommunityCardType::DOCTOR_FEE) {
    const int doctorFee = 700;
    try {
      if (!p->canPay(doctorFee)) {
        throw InsufficientFundsException(p->getUsername(), doctorFee, p->getCash());
      }
      p->reduceCash(doctorFee);
      std::cout << "Kamu membayar M" << doctorFee << " ke Bank." << " Sisa Uang = M" << p->getCash() << ".\n";
      gm->getLogger().log(gm->getCurrentTurn(), p->getUsername(), "PAJAK", "Dana Umum: Membayar biaya dokter M" + std::to_string(doctorFee));
    } catch (const InsufficientFundsException &) {
      std::cout << "Kamu tidak mampu membayar biaya dokter! (M" << doctorFee << ")\n";
      std::cout << "Uang kamu saat ini: M" << p->getCash() << "\n";
      gm->executeBankruptcy(*p, nullptr, doctorFee);
    }
    return;
  }

  if (communityType == CommunityCardType::CAMPAIGN_FEE) {
    const int campaignFee = 200;
    for (Player &other : gm->getPlayers()) {
      if (&other == p || other.getStatus() == BANKRUPT) {
        continue;
      }

      try {
        if (!p->canPay(campaignFee)) {
          throw InsufficientFundsException(p->getUsername(), campaignFee, p->getCash());
        }
        p->reduceCash(campaignFee);
        other.addCash(campaignFee);
      } catch (const InsufficientFundsException &) {
        gm->executeBankruptcy(*p, &other, campaignFee);
        break;
      }
    }
    gm->getLogger().log(gm->getCurrentTurn(), p->getUsername(), "PAJAK", "Dana Umum: Membayar campaign fee M200 ke setiap pemain");
    return;
  }

  throw InvalidCardException("CommunityCard");
}
