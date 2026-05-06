//
// Created by Ahmed Ibrahim on 5/3/26.
//

#ifndef UNTITLED_ITEM_H
#define UNTITLED_ITEM_H

#include <string>
class Item {
public:
    Item(const std::string& name,const std::string& description, int damage);
    const std::string& getName() const;
    const std::string& getDescription() const;
    const bool getIsFood() const;
    int getDamage() const;
private:
    std::string m_name;
    std::string m_description;
    int m_damage;
    bool m_isfood;
};

#endif //UNTITLED_ITEM_H
