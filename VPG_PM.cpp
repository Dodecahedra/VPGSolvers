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
 * Finds the progress measure m s.t. `m ≽(k) U(t)(phi)`.
 * @param k the index we need to compare up to.
 * @param phi pair of <ProgM, ConfSet> that belongs to U(t)(phi).
 * @param m Progress Measure that we write the result to.
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
 * Updates the progress measures of U[s] using the new mapping V. Updates the mapping if
 * V < U[s].
 * @param V mapping containing the progress measure we just computed for `s`.
 * @param s the vertex we are updating.
 * @param b boolean variable which we set to true if we updated one of the progress measures in U.
 */
void VPG_PM::MIN(map<ProgM, ConfSet> &V, int s, bool &b) {
    for (const auto& t : V) {
        /* Do a reverse search through U[s] and break out of the loop if we find that the progress
         * measure has become smaller or equal to V[phi]. */
        for (auto rit = U[s].rbegin(); rit != U[s].rend(); ++rit) {
            if ((t.second & rit->second) != emptyset) {
                rit->second = (rit->second - t.second);
                U[s][t.first] |= (rit->second & t.second);
                b = true;
            }
            if (!ProgMComp(rit->first, t.first)) break;
        }
    }
}
/**
 *Updates the progress measures of U[s] using the new mapping V. Updates the mapping if
 * V > U[s].
 * @param V mapping containing the progress measure we just computed for `s`.
 * @param s the vertex we are updating.
 * @param b boolean variable which we set to true if we updated one of the progress measures in U.
 */
void VPG_PM::MAX(map<ProgM, ConfSet> &V, int s, bool &b) {
    for (const auto& t : V) {
        /* Iterate trough U[s] and break once we find that the progress measure of t has become
         * bigger than that in U[s]. */
        for (auto it = U[s].begin(); it != U[s].end(); ++it) {
            if ((t.second & it->second) != emptyset) {
                it->second = (it->second - t.second);
                U[s][t.first] |= (it->second & t.second);
                b = true;
            }
            if (!ProgMComp(it->first, t.first)) break;
        }
    }
}

/**
 * Go over U and look whether the vertex `i` is winning for odd (if `m=T`) or if it is won by
 * player even (`m!=T`) and write it to the `game`.
 */
void VPG_PM::writeResult() {
    for (int i = 0; i < U.size(); i++) {
        auto t = U[i];
        for (const auto& ti : t) {
            if (ti.first[l] == T[l]) { // U[t][i] = T
                game->winning_1[i] |= ti.second;
            } else {
                game->winning_0[i] |= ti.second;
            }
        }
    }
}

/**
 * Run the Progress Measure algorithm.
 */
void VPG_PM::run() {
    // Initialise Q with all vertices in the game.
    queue<int> Q = queue<int>();
    set<int> QSet = set<int>();
    for (int i = 0; i < game->n_nodes; i++) { Q.emplace(i); QSet.emplace(i); }
    /* Main loop of the algorithm. We try to lift the progress measure for each
     * vertex which has an out-neighbour which has been updated. QSet contains the
     * set of vertices in the queue to prevent that a vertex occurs in the queue twice. */
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
                MAX(V, s, updated);
            } else { // Owner is even
                MIN(V, s, updated);
            }
            if (updated) {
                for (auto tii : game->in_edges[s]) {
                    int sii = get<0>(tii);
                    if (QSet.count(sii) != 0) {
                        Q.emplace(sii);
                        QSet.emplace(sii);
                    }
                }
            }
        }
    }
    writeResult();
}