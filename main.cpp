//
// Created by koen on 26-02-21.
//

#include "VPGame.h"


int main(int argc, char** argv)
{
    VPGame game;
    game.parseVPGFromFile(argv[1]);
    game.sort();
}