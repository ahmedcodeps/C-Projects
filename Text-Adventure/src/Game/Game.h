//
// Created by Ahmed Ibrahim on 5/4/26.
//

#ifndef UNTITLED_GAME_H
#define UNTITLED_GAME_H
#include <memory>
#include <vector>

#include "../Engine/Item.h"
#include "../Engine/Room.h"
#include "../Engine/Player.h"
#include "../Engine/Command.h"

class Game {
public:
    Game();

    void Init();

    void Run();


private:

    void ProcessCommands(const Command& cmd);

    std::vector<std::unique_ptr<Room>> m_rooms;
    std::unique_ptr<Player> m_player;

    bool m_isRunning = false;
};


#endif //UNTITLED_GAME_H
