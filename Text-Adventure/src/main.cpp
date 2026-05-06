#include <print>
#include <sstream>
#include "Engine/Item.h"
#include "Engine/Room.h"
#include "Engine/Player.h"
#include "Engine/Command.h"
#include "Game/Game.h"
#include "Engine/Parser.h"
#include <iostream>


// have to add some type of begining that tells the player all the commands and how to refer back to them

int main() {
    Game game;
    game.Init();
    game.Run();
}

