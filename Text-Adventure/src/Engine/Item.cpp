#include "Item.h"

Item::Item(const std::string& name,const std::string& description,const int damage, const int heal) :
m_name(name), m_description(description), m_damage(damage), m_isFood(false), m_heal(heal) {
    if (m_damage == 0)
        m_isFood = true;
}

const std::string& Item::getName() const {
    return m_name;
}
const std::string& Item::getDescription() const {
    return m_description;
}

int Item::getDamage() const {
    return m_damage;
}

int Item::getHeal() const {
    return m_heal;
}

bool Item::getIsFood() const {
    return m_isFood;
}

