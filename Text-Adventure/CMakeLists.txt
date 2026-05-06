cmake_minimum_required(VERSION 4.2)
project(untitled)

set(CMAKE_CXX_STANDARD 26)

add_executable(untitled src/main.cpp
        src/Engine/Item.h
        src/Engine/Item.cpp
        src/Engine/Room.cpp
        src/Engine/Room.h
        src/Engine/Player.cpp
        src/Engine/Player.h
        src/Game/Game.cpp
        src/Game/Game.h
        src/Engine/Command.h
        src/Engine/Parser.h
        src/Engine/Enemy.cpp
        src/Engine/Enemy.h)
