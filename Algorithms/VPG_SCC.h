//
// Created by koen on 23-06-21.
//

#ifndef VPGSOLVERS_VPG_SCC_H
#define VPGSOLVERS_VPG_SCC_H

#include <stack>
#include <queue>
#include <list>
#include "../VariabilityParityGames/VPGame.h"
#include "../VariabilityParityGames/Algorithms/zlnkVPG.h"

class VPG_SCC {
public:
    explicit VPG_SCC(VPGame * game);
    explicit VPG_SCC(VPGame * game, VertexSetZlnk * bigV, vector<ConfSet> * vc);

    void solve(VectorBoolOptimized *W0bigV, vector<bdd> *W0vc, VectorBoolOptimized *W1bigV, vector<bdd> *W1vc);

    /**
     * Amount of times we called tarjan's algorithm to decompose the VPG into SCCs.
     */
    int tarjan_calls = 0;
    /**
     * Time (ns) spent on computing the SCCs of the VPG.
     */
    long tarjan_time = 0;
    /**
     * Time spend in attractor set calculation
     */
    long attracting = 0;

    int attractions = 0;

protected:
    VPGame *game;
    VertexSetZlnk *bigV;
    VertexSetZlnk emptyvertexset;
    vector<ConfSet> *vc;
    int* index;
    int* lowlink;
    bool* onstack;

    void tarjanTSCC(list<unordered_set<int>> *tscc_map);

    void strongconnect(int v, int *idx, std::stack<int> *S, list<unordered_set<int>> *tscc_map);

    bool is_terminal(unordered_set<int> &p);

    tuple<int, int> getHighLowPrio();

    void getVCWithPrio(VectorBoolOptimized *bigA, vector<bdd> *ac, int prio);

    void attrQueue(int player, VectorBoolOptimized *bigA, vector<bdd> *ac);

    void removeFromBigV(VectorBoolOptimized *bigA, vector<bdd> *ac);

    void removeFromBigV(int i, bdd c);

    void attr(int player, VectorBoolOptimized *bigA, vector<bdd> *ac);

    void addTobigV(VectorBoolOptimized *pOptimized, vector<bdd> *pVector);

    void unify(VectorBoolOptimized *bigA, vector<bdd> *ac, VectorBoolOptimized *bigB, vector<bdd> *bc);
};


#endif //VPGSOLVERS_VPG_SCC_H
