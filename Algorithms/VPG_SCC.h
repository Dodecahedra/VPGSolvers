//
// Created by koen on 23-06-21.
//

#ifndef VPGSOLVERS_VPG_SCC_H
#define VPGSOLVERS_VPG_SCC_H

#include <stack>
#include <queue>
#include "../VariabilityParityGames/VPGame.h"
#include "../VariabilityParityGames/Algorithms/zlnkVPG.h"

class VPG_SCC {
public:
    explicit VPG_SCC(VPGame * game);
    void run();

    /**
     * Amount of times we called tarjan's algorithm to decompose the VPG into SCCs.
     */
    int tarjan_calls = 0;
    /**
     * Amount of subgames we have solved using Zielonka's algorithm.
     */
    int subgames_solved = 0;
    /**
     * Amount of attractor sets we computed (total so including those from zielonka's).
     */
    int attractors = 0;
    /**
     * Time (ns) spent on computing the SCCs of the VPG.
     */
    long tarjan_time = 0;
    /**
     * Time (ns) spent on solving the subgames and computing the attractor set.
     */
    long solving_time = 0;

protected:
    VPGame *game;
    VertexSetZlnk *V;
    VertexSetZlnk emptyvertexset;
    vector<ConfSet> *C;
    int* index;
    int* lowlink;
    bool* onstack;

    void tarjanTSCC(vector<unordered_set<int>> *tscc_map);

    void strongconnect(int v, int *idx, stack<int> *S, vector<unordered_set<int>> *tscc_map);

    bool is_terminal(unordered_set<int> &p);
};


#endif //VPGSOLVERS_VPG_SCC_H
