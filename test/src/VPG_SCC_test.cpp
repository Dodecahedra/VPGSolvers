#include <gtest/gtest.h>
#include "../../Algorithms/VPG_SCC.h"
#include "VPG_test.cpp"
#include <chrono>
#include <thread>

/**
 * Run the solver on a game and verify it against a pre-computed solution file.
 * input: <gameFile, solutionFile>
 */
TEST_P(VPGame_test, SCC_test_output) {
    std::tuple<std::string, std::string> params = GetParam();
    // Create VPGame and solution from the given input files.
    VPGame game;
    VPG_test::parseGame(game, get<0>(params));
    VPGame solution;
    solution.bm_vars = game.bm_vars;
    solution.parseSolutionFromFile(get<1>(params), game.bigC);
    VPG_test::enableCaching();
    // Solve the game and verify the output.
    VPG_SCC solver(&game);
    auto *W0bigV = new VertexSetZlnk(game.n_nodes);
    auto *W0vc = new vector<ConfSet>(game.n_nodes);
    auto *W1bigV = new VertexSetZlnk(game.n_nodes);
    auto *W1vc = new vector<ConfSet>(game.n_nodes);
    solver.solve(W0bigV, W0vc, W1bigV, W1vc);
    game.winning_0 = (*W0vc);
    game.winning_1 = (*W1vc);
    VPG_test::verifySolution(&game, &solution);

    using namespace std::chrono; // nanoseconds, system_clock, seconds
    std::this_thread::sleep_until(system_clock::now() + seconds(1));
}