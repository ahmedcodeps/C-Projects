//
// Created by Ahmed Ibrahim on 5/4/26.
//

#ifndef UNTITLED_PARSER_H
#define UNTITLED_PARSER_H
#include "Command.h"
#include <sstream>

class Parser {
public:
    static Command Parse(std::string& command) {

        for (int i = 0; i < command.length(); i++) {
            command[i] = tolower(static_cast<unsigned char>(command[i]));
        }

        std::istringstream ss(command);

        Command cmd;
        ss >> cmd.action;
        ss.ignore();
        std::getline(ss, cmd.name);


        return cmd;
    }
};

#endif //UNTITLED_PARSER_H
