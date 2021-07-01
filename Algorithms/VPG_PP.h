//
// Created by koen on 24-02-21.
//

#ifndef VPGSOLVERS_VPG_PP_H
#define VPGSOLVERS_VPG_PP_H
#include "../VariabilityParityGames/VPGame.h"
#include "../Conf.h"
#include "unordered_map"

#endif //VPGSOLVERS_VPG_PP_H

/**
 *
 */
class VPG_PP {
public:
    VPG_PP(VPGame *game);
    VPG_PP(VPGame *game, VertexSetZlnk *subV, vector<ConfSet> *subC);
    void run();

    int promotions;
    int attractions;
    long attractor_time = 0;

protected:
    /** Current game we are solving */
    VPGame *game;
    /** Current subgame we're solving */
    VertexSetZlnk *V;
    /** Current configurations of subgame */
    vector<ConfSet> *C;
    VertexSetZlnk emptyvertexset;

    int *inverse;
    int max_prio;

    VertexSetZlnk *regions;
    unordered_map<int, int> *strategy;
    unordered_map<int, ConfSet> *region;

    void attract(int p);
    void promote(int from, int to);
    void resetRegion(int p);
    bool setupRegion(int p);
    void setDominion(int p);
    int getRegionStatus(int i, int p);

    void attractQueue(int priority);

    void removeFromBigV(int priority);

    void setUpRegions();

    int getEscapeSetStatus(int i, int p, const int a, const VectorBoolOptimized &region_set);

    int findPromotableRegion(int p, const int a, const VectorBoolOptimized &region_set);
};