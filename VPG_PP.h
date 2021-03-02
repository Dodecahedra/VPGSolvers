//
// Created by koen on 24-02-21.
//

#ifndef VPGSOLVERS_VPG_PP_H
#define VPGSOLVERS_VPG_PP_H
#include "VPGame.h"
#include "Conf.h"

#endif //VPGSOLVERS_VPG_PP_H

class VPGPPSolver
{
public:
    VPGPPSolver(VPGame *game);
    ~VPGPPSolver();
    void run();

    int promotions;

protected:
    /** Current game we are solving */
    VPGame *game;
    /** Current subgame we're solving */
    VertexSetZlnk *V;
    /** Current configurations of subgame */
    vector<ConfSet> *C;
    static VertexSetZlnk emptyvertexset;

    int *inverse;
    int max_prio;

    std::vector<std::tuple<VertexSetZlnk, std::vector<ConfSet>>> regions;
    std::vector<std::tuple<ConfSet, int>> *strategy;
    std::vector<std::tuple<ConfSet, int>> *region;

    void attract(int player, VertexSetZlnk *subV, std::vector<ConfSet> *vc);
    void promote(int from, int to);
    void resetRegion(int p);
    bool setupRegion(VertexSetZlnk *bigV, int p, std::vector<ConfSet> *vc);
    void setDominion(int p);
    int getRegionStatus(int i, int p);
    void reportRegion();
    void printState();

    void attractQueue(int player, VectorBoolOptimized *bigA, vector<bdd> *ac) const;

    void removeFromBigV(VectorBoolOptimized *bigA, vector<bdd> *ac) const;

    int closedSubgame(int p, const vector<bdd> &region_confs, int j);

    int findLowestRegion(int p, const vector<bdd> &region_confs, int lowest_region, int j);
};