#include "models/Card/SkillCardFactory.hpp"

#include "models/Card/DemolitionCard.hpp"
#include "models/Card/DiscountCard.hpp"
#include "models/Card/LassoCard.hpp"
#include "models/Card/MoveCard.hpp"
#include "models/Card/ShieldCard.hpp"
#include "models/Card/SkillCard.hpp"
#include "models/Card/TeleportCard.hpp"

#include <algorithm>
#include <cctype>
#include <random>

namespace {

std::string uppercase(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return text;
}

int randomInt(int min, int max) {
    static std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}

}

SkillCard* SkillCardFactory::create(const std::string& type, int value,
                                    int duration) {
    const std::string normalized = uppercase(type);
    std::unique_ptr<SkillCard> card;

    if (normalized == "MOVECARD") {
        const int steps = value > 0 ? value : randomInt(1, 6);
        card = std::make_unique<MoveCard>(0, steps);
    } else if (normalized == "DISCOUNTCARD") {
        const int discount = value > 0 ? value : (randomInt(1, 5) * 10);
        const int effectiveDuration = duration > 0 ? duration : 1;
        card = std::make_unique<DiscountCard>(0, discount, effectiveDuration);
    } else if (normalized == "SHIELDCARD") {
        card = std::make_unique<ShieldCard>();
    } else if (normalized == "TELEPORTCARD") {
        card = std::make_unique<TeleportCard>();
    } else if (normalized == "LASSOCARD") {
        card = std::make_unique<LassoCard>();
    } else if (normalized == "DEMOLITIONCARD") {
        card = std::make_unique<DemolitionCard>();
    }

    if (!card) {
        return nullptr;
    }

    SkillCard* rawCard = card.get();
    ownedCards.push_back(std::move(card));
    return rawCard;
}

void SkillCardFactory::release(SkillCard* card) {
    if (card == nullptr) {
        return;
    }

    ownedCards.erase(
        std::remove_if(ownedCards.begin(), ownedCards.end(),
                       [card](const std::unique_ptr<SkillCard>& ownedCard) {
                           return ownedCard.get() == card;
                       }),
        ownedCards.end());
}

void SkillCardFactory::clear() {
    ownedCards.clear();
}
