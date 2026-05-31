#include <print>
#include <sstream>
#include "Engine/Item.h"
#include "Engine/Room.h"
#include "Engine/Player.h"
#include "Engine/Command.h"
#include "Game/Game.h"
#include "Engine/Parser.h"
#include <iostream>


int main() {
    Game game;
    game.Init();
    game.Run();
}

