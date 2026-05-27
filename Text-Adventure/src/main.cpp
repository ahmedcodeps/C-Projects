#include "Engine/Item.h"
#include "Engine/Room.h"
#include "Engine/Player.h"
#include "Engine/Command.h"
#include "Game/Game.h"
#include "Engine/Parser.h"


int main() {
    Game game;
    game.Init();
    game.Run();
}

