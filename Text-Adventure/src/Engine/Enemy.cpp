//
// Created by Ahmed Ibrahim on 5/4/26.
//

#include "Enemy.h"

Enemy::Enemy(const std::string &name, const std::string &description, int health, int damage) :
m_name(name), m_description(description), m_health(health), m_damage(damage) {}

const std::string &Enemy::getName() const {
    return m_name;
}

const std::string &Enemy::getDescription() const {
    return m_description;
}

int Enemy::getDamage() const {
    return m_damage;
}

int Enemy::getHealth() {
    if (m_health <= 0)
        m_health = 0;
    return m_health;
}

bool Enemy::isDead() const {
    return m_health <= 0;
}

void Enemy::TakeDamage(int damage) {
    m_health -= damage;
}
