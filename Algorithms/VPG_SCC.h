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
