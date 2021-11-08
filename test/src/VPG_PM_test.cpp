#include <gtest/gtest.h>
#include "../../Algorithms/VPG_PM.h"
#include "VPG_test.cpp"
#include <chrono>
#include <thread>

/**
 * Run the solver on a game and verify it against a pre-computed solution file.
 * input: <gameFile, solutionFile>
 */
TEST_P(VPGame_test, PM_test_output) {
    std::tuple<std::string, std::string> params = GetParam();
    // Create VPGame and solution from the given input files.
    VPGame game;
    VPG_test::parseGame(game, get<0>(params));
    VPGame solution;
    solution.bm_vars = game.bm_vars;
    solution.parseSolutionFromFile(get<1>(params), game.bigC);
    VPG_test::enableCaching();
    // Solve the game and verify the output.
    game.sort();
    VPG_PM solver(&game);
    solver.run();
    VPG_test::verifySolution(&game, &solution);

    using namespace std::chrono; // nanoseconds, system_clock, seconds
    std::this_thread::sleep_until(system_clock::now() + seconds(1));
}