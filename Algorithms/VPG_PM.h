//
// Created by koen on 16-03-21.
//

#ifndef VPGSOLVERS_VPG_PM_H
#define VPGSOLVERS_VPG_PM_H
#include "../VPGame.h"
#include "../Conf.h"
#define ProgM vector<int>
/**
 * Declaration of the Progress Measures algorithm for Variability Parity Games.
 * Algorithm uses the notion of Progress Measures to calculate which vertices and configurations
 * are winning for player 0/1. Based on the description of the Progress Measure algorithm as described
 * in `Featured Games` by Uli Fahrenberg and Axel Legay.
 */
class VPG_PM {
public:
    VPG_PM(VPGame *game);
    void run();

protected:
    /** Parity Game we are solving. */
    VPGame *game;

    struct progmcomparator {
        bool operator()(const ProgM& a, const ProgM& b) const {
            /* Do a reverse comparison of the two arrays. Last index stores the
             * most significant measure. */
            for (int i = 0; i < a.size(); i++) {
                if (a[i] != b[i]) {
                    return a[i] < b[i];
                }
            }
            return false;
        }
    };

    struct equiv {
        bool operator()(const ProgM& a, const ProgM& b) const {
            /* Do a reverse comparison of the two arrays. Last index stores the
             * most significant measure. */
            for (int i = 0; i < a.size(); i++) {
                if (a[i] != b[i]) {
                    return false;
                }
            }
            return true;
        }
    };
    /** Fixed-size array of size {@code game->n_nodes}. For each vertex we keep a mapping from ConfSet
     * to a progress measure*/
    vector<map<ProgM, ConfSet, progmcomparator>> U;
    vector<tuple<ConfSet, int>> strategy;
    /** Array containing the progress measure. */
    progmcomparator ProgMComp;
    equiv equiv;
    vector<int> M;
    vector<int> Z;
    vector<int> T;
    int l;

    /** Computes the minimum progress measure `m` such that `m ≽(k) U(t)(phi)`. */
    void minProg(int k, pair<ProgM, ConfSet> phi, ProgM &m);
    /** Compute the MIN of two mappings U: ConfSet -> ProgM. */
    void MIN(map<ProgM, ConfSet, progmcomparator> &W, map<ProgM, ConfSet, progmcomparator> &V);
    /** Compute the MAX of two mappings U: ConfSet -> ProgM. */
    void MAX(map<ProgM, ConfSet, progmcomparator> &W, map<ProgM, ConfSet, progmcomparator> &V);

    int findHighestPriority();

    void writeResult();

    void setTop(vector<int> &m);

    void updateU(map<ProgM, ConfSet, progmcomparator> &W, int s, bool &updated);

    void fillInW(map<ProgM, ConfSet, progmcomparator> &W);
};


#endif //VPGSOLVERS_VPG_PM_H
