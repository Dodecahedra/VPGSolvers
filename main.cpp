//
// Created by koen on 26-02-21.
//

#include <chrono>
#include "VPGame.h"
#include "VPG_PP.h"
#include "VPG_PM.h"

unordered_map<string, string> winningmap;
string vertex;
void printFull(char* varset, int size, string prod){
    if(size == 0){
        winningmap[prod].append(vertex + ",");
        return;
    }

    if(*varset < 0){
        string prod2 = prod;
        prod.append("0");
        prod2.append("1");
        printFull(varset + 1, size -1, prod);
        printFull(varset + 1, size -1, prod2);
    } else {
        if(*varset == 0){
            prod.append("0");
        } else {
            prod.append("1");
        }
        printFull(varset +1, size -1, prod);
    }
}
void allsatPrintHandler(char* varset, int size)
{
    string p = "";
    printFull(varset, size, p);
}

void printSolution(vector<ConfSet> winning, int player) {
    winningmap = unordered_map<string, string>();
    cout << "W" << player << ": \n";
    for (int i = 0 ; i < winning.size(); i++) {
        vertex = to_string(i);
        bdd_allsat(winning[i], allsatPrintHandler);
    }
    for (auto& i: winningmap) {
        cout << "For product " << i.first << " the following vertices are in: " << i.second << std::endl;
    }
}

int main(int argc, char** argv) {
    VPGame game;
    // Read in the game
    game.parseVPGFromFile(argv[1]);
    // Make sure the game is sorted
    auto start = std::chrono::high_resolution_clock::now();
    game.sort();
//    VPGPPSolver solver(&game);
//    solver.run(); // Run the solver, should finish and write solution to VPGame
    VPG_PM solver(&game);
    solver.run();
    auto end = std::chrono::high_resolution_clock::now();
    auto running_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    game.permute(game.mapping);
    printSolution(game.winning_0, 0);
    printSolution(game.winning_1, 1);
    cout << "Solving took: " << running_time.count() << " ns" << std::endl;
//    cout << "Using: " << solver.promotions << " promotions" << std::endl;
}