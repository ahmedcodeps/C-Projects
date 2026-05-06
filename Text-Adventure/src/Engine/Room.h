//
// Created by Ahmed Ibrahim on 5/3/26.
//

#ifndef UNTITLED_ROOM_H
#define UNTITLED_ROOM_H

#include <string>
#include <unordered_map>
#include <vector>
#include "Item.h"
#include "Enemy.h"

class Room {
public:

    // ADD DEATH FUNCTION TO ORCA

    Room(const std::string& name, const std::string& description);

    const std::string& getName() const;
    const std::string& getDescription() const;
    const std::unordered_map<std::string, Room*>& getExits() const;
    const std::vector<std::unique_ptr<Item>>& getItems() const;

    Enemy* getEnemy() const;

    void print_contents() const;

    void addItem(std::unique_ptr<Item> item);
    void addExit(const std::string& direction, Room* room);
    void addEnemy(std::unique_ptr<Enemy> enemy);
    bool removeEnemy();

    std::unique_ptr<Item> giveItem(const std::string& item_name);

private:
    std::string m_name;
    std::string m_description;

    std::unordered_map<std::string, Room*> m_exits;
    std::vector<std::unique_ptr<Item>> m_items;
    std::unique_ptr<Enemy> m_enemy;
};

#endif //UNTITLED_ROOM_H
