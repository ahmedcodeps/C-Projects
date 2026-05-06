#include "Room.h"
#include <print>
#include <ranges>


Room::Room(const std::string& name, const std::string& description) :
m_name(name), m_description(description) {}

const std::string& Room::getName() const {
    return m_name;
}

const std::string& Room::getDescription() const {
    return m_description;
}

const std::unordered_map<std::string, Room *>& Room::getExits() const {
    return m_exits;
}

const std::vector<std::unique_ptr<Item> >& Room::getItems() const {
    return m_items;
}

Enemy* Room::getEnemy() const {
    return m_enemy.get();
}

void Room::print_contents() const {
    std::println("{}", m_name);
    std::println("{}", m_description);

    if (m_items.empty() == false) {
        std::print("Items: ");
        for (const auto& item : m_items) {
            std::print("{}; ",item->getName());
        }

        std::println();
    }

    std::print("Exits: ");
    for (const auto &direction: m_exits | std::views::keys) {
        std::print("{}; ",direction);
    }
    std::println();
    if (m_enemy) {
        std::print("Enemies: {}", m_enemy->getName());
        std::println();
    }
}

void Room::addItem(std::unique_ptr<Item> item) {
    m_items.push_back(std::move(item));
}

void Room::addExit(const std::string &direction, Room *room) {
    m_exits[direction] = room;
}

void Room::addEnemy(std::unique_ptr<Enemy> enemy) {
    m_enemy = std::move(enemy);
}

bool Room::removeEnemy() {
    if (m_enemy->isDead() == true) {
        if (m_enemy->getName() == "Zombie") {
            std::println("{} has been put out of its misery...", m_enemy->getName());
        }
        else {
            std::println("{} has been slain.", m_enemy->getName());
        }
        auto remove_ptr = std::move(m_enemy);
        return true;
    }
    return false;
}

std::unique_ptr<Item> Room::giveItem(const std::string &item_name) {

    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        std::string namecopy = it->get()->getName();

        for (int i = 0; i < namecopy.length(); i++) {
            namecopy[i] = tolower(static_cast<unsigned char>(namecopy[i]));
        }

        if (namecopy == item_name) {
            std::unique_ptr<Item> item_ptr = std::move(*it);
            m_items.erase(it);
            return (item_ptr);
        }
    }
    return nullptr;
}
