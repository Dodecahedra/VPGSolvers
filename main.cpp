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
    if (string(argv[1]) == "-h") {
        cout << "            = VPGSolver version 1.0 =\n" <<
                "----------------------------------------------------\n" <<
                "Usage: VPGSolver [FILE] [SOLVER] [LOOPS]\n" <<
                " FILE: utf-8 encoded file containing a VPG\n" <<
                " SOLVER: Any of the following solvers:\n" <<
                "   P: Priority Promotion algorithm\n" <<
                "   S: SCC Decomposition algorithm\n" <<
                "   M: Small Progress Measures algorithm\n" <<
                " LOOPS: Add `L` option to eliminate self loops\n" <<
                "\n" <<
                "Example: `VPGSolver eVPG P` runs Priority Promotion on the file `eVPG`"
            << std::endl;
        return 0;
    }
    if (argc < 3) {
        cout << "Error: not enough arguments" << std::endl;
        return 1;
    }
    bool detect_loops = false;
    if (argc > 3) {
        if (*argv[3] == 'L') { // Enable self-loop elimination
            detect_loops = true;
        } else {
            cout << "Error: " << *argv[3] << " is not a valid command option" << std::endl;
            return 1;
        }
    }
    /* Parse the Variability Parity Game from input and, optionally, collect
     * vertices with a self-loop in the vector. */
    auto *loops = new vector<int>();
    game.parseVPGFromFile(argv[1], loops);

    bdd_gbc();
    // enable cache after parsing
    bdd_setcacheratio(200);
    bdd_gbc();
    auto start = std::chrono::high_resolution_clock::now();

    long elimination_time;
    if (detect_loops) {
        auto start_elim = std::chrono::high_resolution_clock::now();
        game.elimateSelfLoops(loops); // Solve self-loops.
        auto end_elim = std::chrono::high_resolution_clock::now();
        elimination_time =
                std::chrono::duration_cast<std::chrono::nanoseconds>(end_elim - start_elim)
                        .count();
    }
    bool sort = false;
    cout << "Vertices: " << game.n_nodes << endl;
    cout << "Edges: " << game.edge_guards.size() << endl;
    /** Select solver we are running */
    std::chrono::high_resolution_clock::time_point end;
    if (*argv[2] == 'M') {
        VPG_PM solver (&game);
        solver.run();
        end = std::chrono::high_resolution_clock::now();
        if (detect_loops) cout << "=<3>=:" << elimination_time << std::endl;
    } else if (*argv[2] == 'P') {
        sort = true;
        game.sort();
        VPG_PP solver(&game);
        solver.run();
        end = std::chrono::high_resolution_clock::now();
        if (detect_loops) cout << "=<6>=:" << elimination_time << std::endl;
    } else if (*argv[2] == 'S') {
        VPG_SCC solver(&game);
        auto *W0bigV = new VertexSetZlnk(game.n_nodes);
        auto *W0vc = new vector<ConfSet>(game.n_nodes);
        auto *W1bigV = new VertexSetZlnk(game.n_nodes);
        auto *W1vc = new vector<ConfSet>(game.n_nodes);
        solver.solve(W0bigV, W0vc, W1bigV, W1vc);
        end = std::chrono::high_resolution_clock::now();
       game.winning_0 = (*W0vc);
       game.winning_1 = (*W1vc);
       cout << "=<1>=:" << solver.tarjan_calls << std::endl;
       cout << "=<2>=:" << solver.tarjan_time<< std::endl;
       cout << "=<3>=:" << solver.attracting << std::endl;
        cout << "=<4>=:" << solver.attractions << std::endl;
        if (detect_loops) cout << "=<5>=:" << elimination_time << std::endl;
    } else {
        cout << "Error: invalid option: " << *argv[2] << std::endl;
        return 1;
    }
    long running_time =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
                    .count();
    if (sort) game.permute(game.mapping);
    cout << "Solving time: " << running_time << " ns" << std::endl;
    cout << "*-----------------------------------*" << endl;
    printSolution(game.winning_0, 0);
    printSolution(game.winning_1, 1);
}