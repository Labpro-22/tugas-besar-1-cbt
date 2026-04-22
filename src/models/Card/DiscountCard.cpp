#include "../../../include/models/Card/DiscountCard.hpp"
#include <algorithm>

// ctor
DiscountCard::DiscountCard() : SkillCard(), discountPercent(0), remainingDuration(0) {}

// custom ctor
DiscountCard::DiscountCard(int cardId, int discountPercent, int remainingDuration)
: SkillCard(cardId), discountPercent(discountPercent), remainingDuration(remainingDuration) {}

// dtor
DiscountCard::~DiscountCard() {}

// Getter discount
int DiscountCard::getDiscountPercent() const {
    return discountPercent;
}

// Getter remaining duration
int DiscountCard::getRemainingDuration() const {
    return remainingDuration;
}

// Must implement functions
std::string DiscountCard::getDescription() const {
    return "Discount " + std::to_string(discountPercent) + "% for " + std::to_string(remainingDuration) + " turns";
}

std::string DiscountCard::getType() const {
    return "DiscountCard";
}

int DiscountCard::getValue() const {
    return discountPercent; 
}

// apply discount
int DiscountCard::applyDiscount(int amount) {
    if (amount <= 0) {
        return 0;
    }

    int effectiveDiscount = std::clamp(discountPercent, 0, 100);
    return amount - (amount * effectiveDiscount / 100);
}

void DiscountCard::use(Player* p, GameManager* gm) {
    if (p == nullptr || gm == nullptr) {
        return;
    }

    if (!p->canUseAbility()) {
        gm->addLogEntry(p->getUsername() + " gagal menggunakan DiscountCard (ability sudah dipakai)");
        return;
    }

    p->applyDiscount(discountPercent, remainingDuration);

    markAsUsed();
    p->setUsedAbility();
    p->removeCard(this);
    gm->addLogEntry("Diskon " + std::to_string(discountPercent) + "% aktif");
}
