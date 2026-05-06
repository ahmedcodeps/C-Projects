#include "Item.h"

Item::Item(const std::string& name,const std::string& description,const int damage) :
m_name(name), m_description(description), m_damage(damage) {
    if (m_damage == 0) {
        m_isfood = true;
    }
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

const bool Item::getIsFood() const {
    return m_isfood;
}

