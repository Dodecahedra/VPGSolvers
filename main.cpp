//
// Created by koen on 26-02-21.
//

#include "VPGame.h"
#include "VPG_PP.h"

int main(int argc, char** argv)
{
    VPGame game;
    // Read in the game
    game.parseVPGFromFile(argv[1]);
    // Make sure the game is sorted
    game.sort();
    VPGPPSolver solver(&game);
    solver.run(); // Run the solver, should finish and write solution to VPGame
}