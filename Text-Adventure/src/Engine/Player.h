//
// Created by Ahmed Ibrahim on 5/3/26.
//

#ifndef UNTITLED_PLAYER_H
#define UNTITLED_PLAYER_H

#include "Item.h"
#include "Room.h"

class Player {
public:
    Player(const std::string& name, Room* room, int health);

    bool isFighting() const;

    void Inventory() const;

    void PrintStats() const;

    Room* GetRoom() const;

    void Move(const std::string& move_direction); // changes rooms
    void Take(const std::string& item_name);  // calls the give item on the room and assigns it
    void Drop(const std::string& item_name); // removes the item
    void Eat(const std::string& item_name);
    void Use(const std::string& item_name);
    void Battle(const std::string& enemy_name);
    void Attack(const std::string& item_name);
    void TakeDamage(const Enemy* enemy);

    void ExitFight();

    bool isDead() const;



private:

    void EnterFight();

    std::string m_name;
    Room* m_room;
    std::vector<std::unique_ptr<Item>> m_items;
    int m_health;
    bool m_isFighting;
};

#endif //UNTITLED_PLAYER_H
