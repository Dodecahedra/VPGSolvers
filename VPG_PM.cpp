//
// Created by koen on 16-03-21.
//

#include <queue>
#include <set>
#include "VPG_PM.h"

/**
 * Implementation of the Progress Measures algorithm. Unlike the original algorithm as described in
 * `Featured Games`, we only reconsider a vertex if one of its out-neighbours has been updated. At the
 * start of the algorithm the queue Q contains all the vertices in the VPG.
 */
VPG_PM::VPG_PM(VPGame *game)
    :game(game) {
    U = vector<map<ProgM, ConfSet, progmcomparator>>(game->n_nodes);
    // Initialize U
    l = game->priority[0];
    for (int i = 0; i < game->n_nodes; i++) {
        auto m =  map<ProgM, ConfSet, progmcomparator>();
        vector<int> n = vector<int>(l+1);
        m[n] = game->bigC;
        /* We initialise `U` where U(0)=C. We set the progress measure as key to allow us to
         * efficiently update only progress measures smaller/bigger than a given value. */
        U[i] = m;
    }
    M = vector<int>(l+1);
    // Compute the progress measure M.
    for (int i = 0; i < game->n_nodes; i++) {
        int pr = game->priority[i];
        if (pr%2!=0) {
            M[pr]++;
            T[pr]++;
        }
    }
    // We set T to be one higher than M.
    T[l]++;
}

/**
 *
 * @param k
 * @param phi
 * @param m
 */
void VPG_PM::minProg(int k, pair<ProgM, ConfSet> phi, ProgM &m) {
    ProgM u = get<0>(phi);
    bool keep = (k+1)%2;
    if (u[l] == T[l]) { // If progress measure u=T, then set m=T and return.
        setTop(m);
        return;
    }
    /* We only go over the odd places in m.
     * TODO: only store odd indices of m.*/
    for (int i = 1; i < k; i += 2) {
        if (k%2==0) { // k is even
            m[i] = u[i];
        } else { // k is odd
            if (keep) {
                m[i] = u[i];
            } else if (u[i] < M[i]) {
                m[i] = u[i] + 1;
                keep = true;
            }
        }
    }
    if (!keep) {
        setTop(m);
    }
}

void VPG_PM::setTop(vector<int> &m) { for (int i = 1; i < T.size(); i+=2) m[i] = T[i]; }

/**
 *
 * @param V
 * @param b
 */
void VPG_PM::MIN(map<ProgM, ConfSet> &V, bool &b) {
    /* TODO:
     *  Update this function. Should return void and accept a reference to a bool and map. Function
     *  goes over U and S and take the MIN accordingly. If U has been updates, write to bool. */
}
/**
 *
 * @param V
 * @param b
 */
void VPG_PM::MAX(map<ProgM, ConfSet> &V, bool &b) {
    /* TODO:
     *  Update this function. Should return void and accept a reference to a bool and map. Function
     *  goes over U and S and take the MAX accordingly. If U has been updates, write to bool. */
}

void VPG_PM::writeResult() {
    /* TODO:
     *  After algorithm has finished, check the result in U and write winning vertices and configs
     *  to the game. */
}

/**
 *
 */
void VPG_PM::run() {
    // Initialise Q with all vertices in the game.
    queue<int> Q = queue<int>();
    set<int> Qset = set<int>();
    for (int i = 0; i < game->n_nodes; i++) { Q.emplace(i); Qset.emplace(i); }
    // ...
    while (!Q.empty()) {
        int s = Q.front(); Q.pop();
        map<ProgM, ConfSet> V = map<ProgM, ConfSet>();
        for (auto ss: game->out_edges[s]) {
            int target = get<0>(ss); int guard = get<1>(ss);
            for (auto& p : U[target]) {
                bdd psi = (p.second & game->edge_guards[guard]);
                if (psi != emptyset) {
                    ProgM m = vector<int>(l);
                    minProg(game->priority[s], p, m);
                    V[m] = p.second;
                }
            }
            bool updated = false;
            if (game->owner[s]) { // Owner is odd
                MAX(V, updated);
            } else { // Owner is even
                MIN(V, updated);
            }
            if (updated) {
                for (auto tii : game->in_edges[s]) {
                    int sii = get<0>(tii);
                    if (Qset.count(sii) != 0) {
                        Q.emplace(sii);
                        Qset.emplace(sii);
                    }
                }
            }
        }
    }
    writeResult();
}