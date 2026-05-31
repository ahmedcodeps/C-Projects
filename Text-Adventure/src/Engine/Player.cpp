#include "Player.h"
#include "Objects.h"
#include <print>


std::string ToLowerCopy(std::string str) {
    for (int i = 0; i < str.length(); i++) {
        str[i] = std::tolower(static_cast<unsigned char>(str[i]));
    }
    return str;
}

bool isThere(const std::string& object_name, auto& items, auto& iterator) {
    for (auto it = items.begin(); it != items.end(); ++it) {
        if (ToLowerCopy(it->get()->getName()) == object_name) {
            iterator = it;
            return true;
        }
    }
    return false;
}


Player::Player(const std::string& name, Room* room, int health) :
m_name(name), m_room(room), m_health(health), m_isFighting(false) {}

bool Player::isFighting() const {
    return m_isFighting;
}

Room* Player::GetRoom() const {
    return m_room;
}

void Player::Inventory() const {
    for (const auto& item : m_items) {
        std::print("{}; ", item->getName());
    }
    std::println();
}

void Player::PrintStats() const {
    std::println("Name is: {}", m_name);
    std::println("Health is: {}", m_health);
    std::println("Room is: {}", m_room->getName());
}

void Player::Move(const std::string &move_direction) {
    for (const auto& [direction, room] : m_room->getExits() ) {
        if (direction == move_direction) {

            if (move_direction == Exits::LOCKED_GATE) {
                std::println("The gate is locked, could it be opened with a key?");
                return;
            }

            std::println("Moving {}...", move_direction);
            m_room = (room) ? room : m_room;
            return;
        }
    }
    std::println("Unable to move {}.", move_direction);
}

void Player::Take(const std::string &item_name) {
    auto new_item = m_room->giveItem(item_name);

    if (new_item != nullptr) {
        std::println("Taking {}...", new_item->getName());
        std::println("{}", new_item->getDescription());
        m_items.push_back(std::move(new_item));
        return;
    }
    std::println("{} could not be found or is already within the inventory.", item_name);
}

void Player::Drop(const std::string &item_name) {
    auto it = m_items.begin();
    if (isThere(item_name, m_items, it)) {
        std::println("Dropping {}...", it->get()->getName());
        auto dropped_item = std::move(*it);
        m_room->addItem(std::move(dropped_item));
        m_items.erase(it);
        return;
    }
    std::println("{} is not within the inventory.", item_name);
}

void Player::Eat(const std::string &item_name) {
    auto it = m_items.begin();
    if (isThere(item_name, m_items, it) && it->get()->getIsFood()) {
        std::println("Eating {}...", it->get()->getName());

        if (it->get()->getHeal() > 0) {
            m_health += it->get()->getHeal();
            std::println("{} healed you for {} health, you are now at {} health.", it->get()->getName(), it->get()->getHeal(), m_health);
        }
        
        m_items.erase(it);
        return;
    }
    std::println("{} is not in the inventory or cannot be eaten.", item_name);
}

void Player::Use(const std::string &item_name) {
    auto it = m_items.begin();
    if (isThere(item_name, m_items, it) && ToLowerCopy(item_name) == Items::KEY) {
        if (m_room->getName() == Locations::ABANDONED_SETTLEMENT) {
            std::println("You used the key to open the gate, the gate door swings open and you continue forward");
            for (auto& [direction, room] : m_room->getExits()) {
                if (direction == Exits::LOCKED_GATE) {
                    m_room = room;
                    m_room->printContents();
                    return;
                }
            }
        }
        else {
            std::println("Cannot use a key here");
            return;
        }
    }
    std::println("{} cannot be used", item_name);
}

void Player::EnterFight() {
    m_isFighting = true;
    std::print("Choose your weapon: ");
    Inventory();
}

void Player::ExitFight() {
    m_isFighting = false;
    std::println("You ran away in cowardice.");
}

void Player::Battle(const std::string &enemy_name) {
    if (m_room->getEnemy()) {
        std::println("Entering battle with {}: {}.", m_room->getEnemy()->getName(), m_room->getEnemy()->getDescription());
        EnterFight();
        return;
    }

    std::println("Could not enter battle with {}.", enemy_name);
}

void Player::Attack(const std::string &item_name) {
    auto it = m_items.begin();
        if (isThere(item_name, m_items,it) && m_room->getEnemy()) {

            if (it->get()->getIsFood() == true) {
                std::println("Cannot damage enemy with food.");
                return;
            }

            if (m_room->getEnemy()->getName() == ENEMIES::GHOST) {
                std::println("You swing with all your might but the Ghost remains unaffected.");
                return;
            }

            m_room->getEnemy()->TakeDamage(it->get()->getDamage());
            std::string enemy_name = m_room->getEnemy()->getName();
            int enemy_health = m_room->getEnemy()->getHealth();
            std::println("You swing the {} at the {} with all your might and do {} damage, the {} has {} health remaining.",
            it->get()->getName(), enemy_name, it->get()->getDamage(), enemy_name, enemy_health);


            if (m_room->removeEnemy()) {
                m_isFighting = false;
            }
            return;
        }
    std::println("{} is not within the inventory.", item_name);
}

void Player::TakeDamage(const Enemy* enemy) {
    if (enemy) {
        m_health -= enemy->getDamage();
        if (enemy->getName() == ENEMIES::ZOMBIE)
            std::println("The zombie continues to stare... it chooses not to fight.");
        else
        std::println("{} lunges at you and hits you for {} damage, you have {} health remaining.", enemy->getName(), enemy->getDamage(), m_health);
    }
}

bool Player::isDead() const {
    return m_health <= 0;
}
