#include <gtest/gtest.h>
#include "../../Conf.h"
#include "../../VariabilityParityGames/VPGame.h"

class VPGame_test :
        public testing::TestWithParam<std::tuple<std::string, std::string>> {
public:
    ~VPGame_test() {
        // Reset the BDD library after each test.
        bdd_done();
    }
};

class VPG_test {
public:
    static void enableCaching() {
        bdd_gbc();
        // enable cache after parsing
        bdd_setcacheratio(200);
        bdd_gbc();
    }

    static void parseGame(VPGame &game, const std::string& filename) {
        // Parse a VPG from file
        auto *loops = new vector<int>();
        game.parseVPGFromFile(filename, loops);
    }

    static void verifySolution(VPGame *game, VPGame *solution) {
        // Take the game and solution and verify outputs for each vertex.
        for (int i = 0; i < game->n_nodes; i++) {
            EXPECT_EQ(game->winning_0[i], solution->winning_0[i]);
            EXPECT_EQ(game->winning_1[i], solution->winning_1[i]);
        }
    }
};
