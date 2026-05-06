//
// Created by Ahmed Ibrahim on 5/4/26.
//

#ifndef UNTITLED_ENEMY_H
#define UNTITLED_ENEMY_H
#include <string>

class Enemy {
public:
    Enemy(const std::string& name, const std::string& description, int health, int damage);

    const std::string& getName() const;
    const std::string& getDescription() const;
    int getHealth();
    int getDamage() const;
    bool isDead() const;
    void TakeDamage(int damage);

private:
    std::string m_name;
    std::string m_description;
    int m_health;
    int m_damage;
};


#endif //UNTITLED_ENEMY_H
