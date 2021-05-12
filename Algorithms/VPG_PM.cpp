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
    d = findHighestPriority();
    for (int i = 0; i < game->n_nodes; i++) {
        auto m =  map<ProgM, ConfSet, progmcomparator>();
        vector<int> n = vector<int>(d + 1);
        m[n] = game->bigC;
        U[i] = m;
    }
    bottom = vector<int>(d + 1);
    M = vector<int>(d + 1);
    top = vector<int>(d + 1);
    // Compute the progress measure M.
    for (int i = 0; i < game->n_nodes; i++) {
        int pr = game->priority[i];
        if (pr%2!=0) {
            M[pr]++;
            top[pr]++;
        }
    }
    // We set top to be one higher than M.
    top[0]++;
}

int VPG_PM::findHighestPriority() {
    int max = -1;
    for (int i = 0; i < game->n_nodes; i++) {
        max = std::max(max, game->priority[i]);
    }
    return max;
}

/**
 * Finds the progress measure m s.t. `m ≽(k) U(t)(phi)`.
 * @param k the index we need to compare up to.
 * @param phi pair of <ProgM, ConfSet> that belongs to U(t)(phi).
 * @param m Progress Measure that we write the result to.
 */
void VPG_PM::minProg(int k, pair<ProgM, ConfSet> phi, ProgM &m) {
    ProgM u = get<0>(phi);
    if (u[0] == top[0]) { // If progress measure u=top, then set m=top and return.
        setTop(m);
        return;
    }
    /* We only go over the odd places in m.
     * TODO: Look into only storing odd indices of m.*/
    if (k%2==0) {
        for (int i = 1; i <= k; i++) {
            m[i] = u[i];
        }
    } else {
        bool increased = false;
        for (int i = k; i > 0; i--) {
            if (u[i] < M[i] && !increased) {
                m[i] = u[i] + 1;
                increased = true;
            } else if (!increased) {
                m[i] = 0; // Set to 0, since we might increase a higher index later.
            } else {
                m[i] = u[i];
            }
        }
        if (!increased) setTop(m);
    }
}

void VPG_PM::setTop(vector<int> &m) { for (int i = 0; i < top.size(); i++) m[i] = top[i]; }

/**
 * Updates the progress measures of U[s] using the new mapping V. Updates the mapping if
 * V < U[s].
 * @param V mapping containing the progress measure we just computed for `s`.
 * @param s the vertex we are updating.
 * @param b boolean variable which we set to true if we updated one of the progress measures in U.
 */
void VPG_PM::MIN(map<ProgM, ConfSet, progmcomparator> &W, map<ProgM, ConfSet, progmcomparator> &V) {
    if (W.empty()) {
        for (const auto& t : V) {
            W[t.first] |= t.second;
        }
        return;
    }
    auto N = map<ProgM, ConfSet, progmcomparator>();
    ConfSet C = game->bigC;
    for (auto & w : W) {
        ProgM m = w.first; ConfSet c = w.second & C; // We use C to keep track of measures which have already been set and are therefore lower.
        N[m] |= c; // Set N[m] to the original measure in W.
        for (auto & v : V) {
            // We loop over the measures set in V and see if we have a measure for (part of) c that is lower
            if (!ProgMComp(m, v.first)) {
                if ((v.second & c) != emptyset) {
                    // We can lift (part of) w to a lower measure
                    N[m] -= v.second & c; // Technically we don't need to do the intersection, but just to be safe.
                    N[v.first] |= v.second & c;
                    C -= v.second & c;
                    c -= v.second & c;
                }
            }
        }
        C -= c; // Remove the remainder that we have set at the beginning.
    }
    // Lastly we look if we missed any measures in V and set these.
    for (auto &v : V) {
        if (C == emptyset) break;
        if ((v.second & C) != emptyset) {
            N[v.first] |= v.second & C;
            C -= v.second;
        }
    }
    for (auto &n : N) {
        W.erase(n.first);
        if (n.second != emptyset) {
            W[n.first] |= n.second;
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
void VPG_PM::MAX(map<ProgM, ConfSet, progmcomparator> &W, map<ProgM, ConfSet, progmcomparator> &V) {
    if (W.empty()) {
        for (const auto& t : V) {
            W[t.first] |= t.second;
        }
        return;
    }
    auto N = map<ProgM, ConfSet, progmcomparator>();
    ConfSet C = game->bigC;
    for (auto w = W.rbegin(); w != W.rend(); ++w) {
        ProgM m = w->first; ConfSet c = w->second & C;
        N[m] |= c;
        for (auto v = V.rbegin(); v != V.rend(); ++v) {
            if (!ProgMComp(v->first, m)) {
                if ((v->second & c) != emptyset) {
                    auto f = v->second & c;
                    // We can lift (part of) w to a higher measure
                    N[m] -= v->second & c; // Technically we don't need to do the intersection, but just to be safe.
                    N[v->first] |= v->second & c;
                    C -= v->second & c;
                    c -= v->second & c;
                }
            }
        }
        C -= c;
    }
    // Lastly, we look if we missed any measures in V and set these.
    for (auto v = V.rbegin(); v != V.rend(); ++v) {
        if (C == emptyset) break;
        if ((v->second & C) != emptyset) {
            N[v->first] |= v->second & C;
            C -= v->second;
        }
    }
    for (auto &n : N) {
        W.erase(n.first);
        if (n.second != emptyset) {
            W[n.first] |= n.second;
        }
    }
}

/**
 * Function to write mapping W to U.
 * @param W
 */
void VPG_PM::updateU(map<ProgM, ConfSet, progmcomparator> &W, int s, bool &updated) {
    auto N = map<ProgM, ConfSet, progmcomparator>();
    ConfSet C = game->bigC;
    for (auto w = W.rbegin(); w != W.rend(); ++w) {
        if (C == emptyset) break;
        auto m = w->first; auto c = w->second;
        for (auto &u : U[s]) {
            if ((c & u.second & C) != emptyset) {
                if (!ProgMComp(m, u.first)) {
                    N[u.first] -= c & C;
                    N[w->first] |= c & C;
                    if (w->second != u.second || !equiv(m,u.first)) updated = true;
                    C -= c;
                    c -= u.second;
                } else {
                    c -= u.second;
                }
            }
        }
        N[m] |= c;
    }
    for (auto &n : N) {
        U[s].erase(n.first);
        if (n.second != emptyset) {
            U[s][n.first] |= n.second;
        }
    }
}

/**
 * Go over U and look whether the vertex `i` is winning for odd (if `m=top`) or if it is won by
 * player even (`m!=top`) and write it to the `game`.
 */
void VPG_PM::writeResult() {
    for (int i = 0; i < U.size(); i++) {
        auto t = U[i];
        for (const auto& ti : t) {
            if (ti.first[0] == top[0]) { // U[t][i] = top
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
    // Maintain a set of vertices which are already in the queue (to avoid duplicates)
    set<int> QSet = set<int>();
    // Initialise Q with all vertices in the game.
    queue<int> Q = queue<int>();
    for (int i = 0; i < game->n_nodes; i++) { Q.emplace(i); QSet.emplace(i); }
    auto V = map<ProgM, ConfSet, progmcomparator>();
    auto W = map<ProgM, ConfSet, progmcomparator>();
    /* Main loop of the algorithm. We try to lift the progress measure for each
     * vertex which has an out-neighbour which has been updated. */
    while (!Q.empty()) {
        int s = Q.front(); Q.pop(); QSet.erase(s);
        W.clear();
        for (auto ss: game->out_edges[s]) {
            V.clear();
            int target = get<0>(ss); int guard = get<1>(ss);
            /* We compute the new V mapping for edge s->s'. */
            for (auto& p : U[target]) {
                ConfSet psi = (p.second & game->edge_guards[guard]);
                if (psi != emptyset) {
                    ProgM m = vector<int>(d + 1);
                    minProg(game->priority[s], p, m);
                    V[m] |= psi;
                }
            }
            if (game->owner[s]) {
                MAX(W, V);
            } else {
                MIN(W, V);
            }
        }
        bool updated = false;
        /* We try to update our mapping U[s] using the newly computed progress measures in W. Where U[s]=MAX(U[s],W).
         * If we updated one of the values in U[s], `updated` will be set to true. */
        updateU(W, s, updated);
        if(updated) {
            /* We check if our new mapping is different than we mapping we already had
             * for U[s]. If this is the case, we update U[s] with W. */
            for (auto tii : game->in_edges[s]) {
                int sii = get<0>(tii);
                if (QSet.count(sii) == 0) {
                    Q.emplace(sii);
                    QSet.emplace(sii);
                }
            }
        }
    }
    writeResult();
}
