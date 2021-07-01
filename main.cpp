//
// Created by koen on 26-02-21.
//

#include <chrono>
#include "VariabilityParityGames/VPGame.h"
#include "Algorithms/VPG_PP.h"
#include "Algorithms/VPG_PM.h"
#include "Algorithms/VPG_SCC.h"

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
    bool detect_loops = false;
    if (argc > 3) {
        if (*argv[3] == 'L') { // Enable self-loop elimination
            detect_loops = true;
        }
    }
    /* Parse the Variability Parity Game from input and, optionally, collect
     * vertices with a self-loop in the vector. */
    game.parseVPGFromFile(argv[1]);

    long elimination_time;
    if (detect_loops) {
        auto start = std::chrono::high_resolution_clock::now();
        game.elimateSelfLoops(); // Solve self-loops.
        auto end = std::chrono::high_resolution_clock::now();
        elimination_time =
                std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
                        .count();
    }
    bool sort = false;
    auto start = std::chrono::high_resolution_clock::now();
    cout << "Vertices: " << game.n_nodes << endl;
    cout << "Edges: " << game.edge_guards.size() << endl;
    /** Select solver we are running */
    if (*argv[2] == 'M') {
        VPG_PM solver (&game);
        solver.run();
    } else if (*argv[2] == 'P') {
        sort = true;
        game.sort();
        VPG_PP solver(&game);
        solver.run();
    } else if (*argv[2] == 'S') {
        VPG_SCC solver(&game);
        solver.run();
    }
    auto end = std::chrono::high_resolution_clock::now();
    long running_time =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
                    .count();
    if (sort) game.permute(game.mapping);
    cout << "Solving time: " << running_time << " ns" << std::endl;
    if (detect_loops) cout << "=<0>=:" << elimination_time << std::endl;
    cout << "*-----------------------------------*" << endl;
    printSolution(game.winning_0, 0);
    printSolution(game.winning_1, 1);
}