#include "Player.h"
#include <print>
#include <ranges>

void tolower(std::string& str) {

    for (int i = 0; i < str.length(); i++) {
        str[i] = tolower(static_cast<unsigned char>(str[i]));
    }
}


Player::Player(const std::string& name, Room* room, int health) :
m_name(name), m_room(room), m_health(health) {}

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

            if (move_direction == "locked gate") {
                std::println("the gate is locked, could it be opened with a key?");
                return;
            }

            std::println("Moving {}...", move_direction);
            m_room = room;
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
    std::println("{} could not be found.", item_name);
}

void Player::Drop(const std::string &item_name) {
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        std::string namecopy = it->get()->getName();
        tolower(namecopy);

        if (namecopy == item_name) {
            println("Dropping {}...", it->get()->getName());
            auto dropped_item = std::move(*it);
            m_room->addItem(std::move(dropped_item));
            m_items.erase(it);
            return;
        }
    }
    std::println("{} is not within the inventory.", item_name);
}

void Player::Eat(const std::string &item_name) {
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        std::string namecopy = it->get()->getName();

        tolower(namecopy);

        if (namecopy == item_name && it->get()->getIsFood()) {
            println("Eating {}...", it->get()->getName());

            if (it->get()->getName() == "Seed") {
                m_health += 3;
                std::println("{} healed {} of your health, you are now at {} health.", it->get()->getName(), 3, m_health);
            }
            else if (it->get()->getName() == "Bass") {
                m_health += 7;
                std::println("{} healed {} of your health, you are now at {} health.", it->get()->getName(), 7, m_health);
            }
            else if (it->get()->getName() == "Rotten Meat") {
                m_health -= 3;
                std::println("{} took {} of your health, you are now at {} health.", it->get()->getName(), 3, m_health);
            }
            else if (it->get()->getName() == "Moldy Bread") {
                m_health += 5;
                std::println("{} healed {} of your health, you are now at {} health.", it->get()->getName(), 5, m_health);

            }

            auto eaten_item = std::move(*it);
            m_items.erase(it);

            return;
        }
    }
    std::println("{} cannot be eaten.", item_name);
}

void Player::Use(const std::string &item_name) {
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        std::string namecopy = it->get()->getName();
        tolower(namecopy);

        if (namecopy == item_name && item_name == "key") {
            if (m_room->getName() == "Abandoned Settlement") {
                std::println("You used the key to open the gate, the gate door swings open and you continue forward");
                for (auto& [direction, room] : m_room->getExits()) {
                    if (direction == "locked gate") {
                        m_room = room;
                        m_room->print_contents();
                        return;
                    }
                }
            }
            else {
                std::println("Cannot use a key here");
                return;
            }
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
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        std::string namecopy = it->get()->getName();
        tolower(namecopy);

        if (namecopy == item_name) {

            if (it->get()->getIsFood() == true) {
                std::println("Cannot damage enemy with food.");
                return;
            }

            if (m_room->getEnemy()->getName() == "Ghost") {
                std::println("You swing with all you're might but the Ghost remains unaffected.");
                return;
            }



            m_room->getEnemy()->TakeDamage(it->get()->getDamage());

            std::println("You swing the {} at the {} with all your might and do {} damage, the {} has {} health remaining.",
           it->get()->getName(), m_room->getEnemy()->getName(), it->get()->getDamage(), m_room->getEnemy()->getName(), m_room->getEnemy()->getHealth());



            if (m_room->removeEnemy()) {
                m_isFighting = false;
            }

            return;
        }
    }
    std::println("{} is not within the inventory.", item_name);
}

void Player::TakeDamage(const Enemy* enemy) {
    if (enemy) {
        m_health -= enemy->getDamage();
        if (enemy->getName() == "Zombie")
            std::println("The zombie continues to stare... it chooses not to fight.");
        else
        std::println("{} lunges at you and hits you for {} damage, you have {} health remaining.", enemy->getName(), enemy->getDamage(), m_health);
    }
}

bool Player::isDead() const {
    return m_health <= 0;
}
