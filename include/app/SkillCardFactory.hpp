#ifndef APP_SKILL_CARD_FACTORY_HPP
#define APP_SKILL_CARD_FACTORY_HPP

#include <memory>
#include <string>
#include <vector>

class SkillCard;

class SkillCardFactory {
public:
    SkillCard* create(const std::string& type, int value = 0, int duration = 0);
    void release(SkillCard* card);
    void clear();

private:
    std::vector<std::unique_ptr<SkillCard>> ownedCards;
};

#endif
