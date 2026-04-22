#include "models/Card/DiscountCard.hpp"
#include <algorithm>
#include "exception/NimonspoliExceptions.hpp"

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

int DiscountCard::getDuration() const {
    return remainingDuration;
}

// apply discount
int DiscountCard::applyDiscount(int amount) {
    if (amount <= 0) {
        throw InvalidInputException("Jumlah yang akan didiskon harus positif.");
    }

    int effectiveDiscount = std::clamp(discountPercent, 0, 100);
    return amount - (amount * effectiveDiscount / 100);
}

void DiscountCard::use(Player* p, GameManager* gm) {
    if (p == nullptr || gm == nullptr) {
        throw InternalGameException("DiscountCard::use menerima konteks yang tidak valid.");
    }

    if (!p->canUseAbility()) {
        gm->addLogEntry(p->getUsername() + " gagal menggunakan DiscountCard (ability sudah dipakai)");
        return;
    }

    p->applyDiscount(discountPercent, remainingDuration);
    std::cout << "DiscountCard diaktifkan! Diskon " << discountPercent
              << "% aktif untuk giliran ini.\n";

    markAsUsed();
    p->setUsedAbility();
    p->removeCard(this);
    gm->addLogEntry("Diskon " + std::to_string(discountPercent) + "% aktif");
}
