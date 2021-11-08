#include <gtest/gtest.h>
#include "../../Algorithms/VPG_PP.h"
#include "VPG_test.cpp"
#include <chrono>
#include <thread>


INSTANTIATE_TEST_SUITE_P(VPG_tests,
                         VPGame_test,
                         testing::Values(
                                 std::tuple<std::string, std::string>("/home/koen/Code/VPGSolvers/test/resources/0VPG",
                                                                      "/home/koen/Code/VPGSolvers/test/resources/0SOL"),
                                 std::tuple<std::string, std::string>("/home/koen/Code/VPGSolvers/test/resources/1VPG",
                                                                      "/home/koen/Code/VPGSolvers/test/resources/1SOL")
                         ));

/**
 * Run the solver on a game and verify it against a pre-computed solution file.
 * input: <gameFile, solutionFile>
 */
TEST_P(VPGame_test, PP_test_output) {
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
    VPG_PP solver(&game);
    solver.run();
    game.permute(game.mapping);
    VPG_test::verifySolution(&game, &solution);

    using namespace std::chrono; // nanoseconds, system_clock, seconds
    std::this_thread::sleep_until(system_clock::now() + seconds(1));
}