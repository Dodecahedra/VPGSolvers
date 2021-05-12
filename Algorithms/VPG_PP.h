//
// Created by koen on 24-02-21.
//

#ifndef VPGSOLVERS_VPG_PP_H
#define VPGSOLVERS_VPG_PP_H
#include "../VPGame.h"
#include "../Conf.h"
#include "unordered_map"

#endif //VPGSOLVERS_VPG_PP_H

/**
 *
 */
class VPG_PP {
public:
    VPG_PP(VPGame *game);
    void run();

    int promotions;

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

    vector<VertexSetZlnk> regions;
    vector<unordered_map<int, int>> strategy;
    vector<unordered_map<int, ConfSet>> region;

    void attract(int p);
    void promote(int from, int to);
    void resetRegion(int p);
    bool setupRegion(int p);
    void setDominion(int p);
    int getRegionStatus(int i, int p);
    void reportRegion();
    void printState();

    void attractQueue(int p);

    void removeFromBigV(int p);

    void setUpRegion();
};