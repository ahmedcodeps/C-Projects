//
// Created by Ahmed Ibrahim on 5/4/26.
//

#include "Game.h"
#include <print>
#include <iostream>
#include "../Engine/Parser.h"

Game::Game() = default;

void Guide() {
    std::println("This is a text-based adventure game with no visuals, in games like this you use your imagination in order to create the world you explore.");
    std::println("Basic commands: ");
    std::println("go - type go *areaname* in order to travel to that exit");
    std::println("take, eat, drop - take an item, eat an item, drop an item (you can pick it back up again)");
    std::println("use - use an item in the correct context")
    ;
    std::println("inventory, look - you can type inventory to see what items you have at any time, look in order to see the contents of the room you're in again");
    std::println("stats - you can use this command to see your name, health, and the room you're currently in");
    std::println("quit - used to quit the game");
    std::println("Combat: ");
    std::println("battle - the battle command is used to enter battle with a monster, keep in mind some commands are not available when you are in battle mode, you will also gain some new commands");
    std::println("attack - use attack *itemname* in order to attack");
    std::println("run - use to run away from battle");
    std::println("you can use the command (guide) in order to see these commands again");
    std::println("PRESS ENTER TO BEGIN");
}

void Game::Init() {

    Guide();
    std::cin.get();


    std::print("Player name: ");
    std::string name;
    std::cin >> name;

    auto Cave_of_Awakening = std::make_unique<Room>("Cave of Awakening", "A strange dark cave, the sound of dripping water from stalactites fills your ears.. How did you get here?");
    auto Forest_of_Mystery = std::make_unique<Room>("Forest of Mystery", "A lush forest concealed by fog and mist, shadows lurk in the foggy cloak..");
    auto Rocky_Range = std::make_unique<Room>("Rocky Range", "A rocky hill with a small lake swelling underneath.");
    auto Abandoned_Settlement = std::make_unique<Room>("Abandoned Settlement", "Ruins of a small village, there's a dried up well and torn down brick houses.");
    auto Brick_House = std::make_unique<Room>("Abandoned brick house", "The only house left partially standing, you try to find anything of use.");
    auto Upstairs = std::make_unique<Room>("Upstairs the House", "You see a family portrait on the walls, there used to be a family here, where are they?");
    auto Village_Outskirts = std::make_unique<Room>("Village Outskirts", "There's nothing but empty land for miles, are you stuck here forever? You decide to give up...");

    Cave_of_Awakening->addExit("north", Forest_of_Mystery.get());
    Forest_of_Mystery->addExit("south", Cave_of_Awakening.get());
    Forest_of_Mystery->addExit("west", Rocky_Range.get());
    Rocky_Range->addExit("east", Forest_of_Mystery.get());
    Rocky_Range->addExit("north", Abandoned_Settlement.get());
    Abandoned_Settlement->addExit("south", Rocky_Range.get());


    Abandoned_Settlement->addExit("brick house", Brick_House.get());
    Abandoned_Settlement->addExit("locked gate", Village_Outskirts.get());
    Brick_House->addExit("outside", Abandoned_Settlement.get());
    Brick_House->addExit("upstairs", Upstairs.get());
    Upstairs->addExit("downstairs", Brick_House.get());


    auto Chipped_Sword = std::make_unique<Item>("Chipped Sword", "Badly battered sword, eroded from years of use.", 10);
    auto Mysterious_Seed = std::make_unique<Item>("Seed", "Mysterious seed found on a dug up piece of soil", 0);
    auto Hearty_Bass = std::make_unique<Item>("Bass", "Hearty fish that's sure to fill you up!", 0);
    auto Silver_Sword = std::make_unique<Item>("Silver Sword", "Of very fine craftsmanship, apt at slaying beasts.", 15);
    auto Rotting_Meat = std::make_unique<Item>("Rotten Meat", "It has a putrid smell.. eat at your own risk.",0);
    auto Moldy_Bread = std::make_unique<Item>("Moldy Bread", "The bread is old, its growing fur like an animal.", 0);
    auto Key = std::make_unique<Item>("Key", "A peculiar key, rusting and corroded with a strange insignia carved onto the back",1);

    auto Orca = std::make_unique<Enemy>("Orca", "He towers over you, salivating at the idea of his next meal, battle scars are imprinted all over his body.", 15, 5);
    auto Ghost = std::make_unique<Enemy>("Ghost", "The ghost howls at you, it has no physical composition, it cannot be hurt or hit.", 10, 5);
    auto Grey_Wolf = std::make_unique<Enemy>("Grey Wolf", "A grey wolf searching for scraps, she dosen't want to fight, but will if she has to.", 25, 10);
    auto Zombie = std::make_unique<Enemy>("Zombie", "The green and fleshy monster turns towards you and stares, it's starving and too exhausted to fight", 1, 0);
    Cave_of_Awakening->addEnemy(std::move(Orca));
    Forest_of_Mystery->addEnemy(std::move(Ghost));
    Abandoned_Settlement->addEnemy(std::move(Grey_Wolf));
    Upstairs->addEnemy(std::move(Zombie));

    Cave_of_Awakening->addItem(std::move(Chipped_Sword));
    Forest_of_Mystery->addItem(std::move((Mysterious_Seed)));
    Rocky_Range->addItem(std::move(Hearty_Bass));
    Abandoned_Settlement->addItem(std::move(Silver_Sword));
    Abandoned_Settlement->addItem(std::move(Rotting_Meat));
    Brick_House->addItem(std::move(Moldy_Bread));
    Upstairs->addItem(std::move(Key));

    auto player = std::make_unique<Player>(name, Cave_of_Awakening.get(), 10);

    m_player = std::move(player);

    m_rooms.push_back(std::move(Cave_of_Awakening));
    m_rooms.push_back(std::move(Forest_of_Mystery));
    m_rooms.push_back(std::move(Rocky_Range));
    m_rooms.push_back(std::move(Abandoned_Settlement));
    m_rooms.push_back(std::move(Brick_House));
    m_rooms.push_back(std::move(Upstairs));
    m_rooms.push_back(std::move(Village_Outskirts));
}

void Game::Run() {
    m_isRunning = true;
    m_player->GetRoom()->print_contents();


    while (m_isRunning) {

        std::println(">");

        std::string prompt;
        std::getline(std::cin, prompt);

        Command command = Parser::Parse(prompt);
        ProcessCommands(command);

        // check things after an event has happened

        if (m_player->isDead()) {
            std::println("{} has overwhelmed you and you have been defeated.", m_player->GetRoom()->getEnemy()->getName());
            m_isRunning = false;
        }

    }
}

void Game::ProcessCommands(const Command &cmd) {
    if (m_player->isFighting() == false) {
        if (cmd.action == "go") {
            m_player->Move(cmd.name);
            m_player->GetRoom()->print_contents();
        }
        else if (cmd.action == "take")
            m_player->Take(cmd.name);
        else if (cmd.action == "eat")
            m_player->Eat(cmd.name);

        else if (cmd.action == "drop")
            m_player->Drop(cmd.name);

        else if (cmd.action == "use")
            m_player->Use(cmd.name);

        else if (cmd.action == "inventory")
            m_player->Inventory();

        else if (cmd.action == "look")
            m_player->GetRoom()->print_contents();

        else if (cmd.action == "stats")
            m_player->PrintStats();

        else if (cmd.action == "quit")
            m_isRunning = false;

        else if (cmd.action == "battle")
            m_player->Battle(cmd.name);

        else if (cmd.action == "guide")
            Guide();


    }
    else if (m_player->isFighting() == true) {

        if (cmd.action == "attack") {
            m_player->Attack(cmd.name);
            m_player->TakeDamage(m_player->GetRoom()->getEnemy());
        }
        else if (cmd.action == "run")
            m_player->ExitFight();

        else if (cmd.action == "stats")
            m_player->PrintStats();

        else if (cmd.action == "inventory")
            m_player->Inventory();

        else if (cmd.action == "guide")
            Guide();


    }
}