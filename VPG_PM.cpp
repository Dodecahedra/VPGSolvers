//
// Created by koen on 16-03-21.
//

#include <queue>
#include <set>
#include <cassert>
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
    Z = vector<int>(l+1);
    M = vector<int>(l+1);
    T = vector<int>(l+1);
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
    for (int i = 1; i <= k; i += 2) {
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

void VPG_PM::setTop(vector<int> &m) { for (int i = 1; i < T.size(); i++) m[i] = T[i]; }

/**
 * Updates the progress measures of U[s] using the new mapping V. Updates the mapping if
 * V < U[s].
 * @param V mapping containing the progress measure we just computed for `s`.
 * @param s the vertex we are updating.
 * @param b boolean variable which we set to true if we updated one of the progress measures in U.
 */
void VPG_PM::MIN(map<ProgM, ConfSet, progmcomparator> &W, map<ProgM, ConfSet, progmcomparator> &V, int s) {
    if (W.empty()) {
        for (const auto& t : V) {
            W[t.first] |= t.second;
        }
        return;
    }
    ConfSet C = game->bigC; auto N = map<ProgM, ConfSet, progmcomparator>();
    for (auto & v : V) {
        if (C == emptyset) break;
        auto m = v.first; auto c = v.second;
        for (auto & w : W) {
            if ((c & w.second & C) != emptyset) {
                N[w.first] = (w.second & C) - c;
                N[v.first] |= (c & w.second & C);
                C -= (c & w.second);
                c -= w.second;
            } else {
                N[w.first] |= w.second;
            }
        }
        N[m] |= c;
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
void VPG_PM::MAX(map<ProgM, ConfSet, progmcomparator> &W, map<ProgM, ConfSet, progmcomparator> &V, int s) {
    if (W.empty()) {
        for (const auto& t : V) {
            W[t.first] |= t.second;
        }
        return;
    }
    ConfSet C = game->bigC; auto N = map<ProgM, ConfSet, progmcomparator>();
    for (auto v = V.rbegin(); v != V.rend(); ++v) {
        if (C == emptyset) break;
        auto m = v->first; auto c = v->second;
        for (auto & w : W) {
            if ((c & w.second & C) != emptyset) {
                N[w.first] = (w.second & C) - c;
                N[v->first] |= (c & w.second & C);
                C -= (c & w.second);
                c -= w.second;
            } else {
                N[w.first] |= w.second;
            }
        }
        N[m] |= c;
    }
    for (auto &n : N) {
        W.erase(n.first);
        if (n.second != emptyset) {
            W[n.first] |= n.second;
        }
    }
}

/**
 *
 * @param W
 */
void VPG_PM::fillInW(map<vector<int>, bdd, progmcomparator> &W) {
    ConfSet all = emptyset;
    for (auto &t : W) {
//        assert((all & t.second)==emptyset);
        all |= t.second;
    }
    ConfSet remainder = game->bigC - all;
    if (remainder != emptyset) W[Z] |= remainder;
}

/**
 * Function to write mapping W to U.
 * @param W
 */
void VPG_PM::updateU(map<ProgM, ConfSet, progmcomparator> &W, int s, bool &updated) {

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
    auto V = map<ProgM, ConfSet, progmcomparator>();
    auto W = map<ProgM, ConfSet, progmcomparator>();
    while (!Q.empty()) {
        int s = Q.front(); Q.pop(); QSet.erase(s);
        W.clear();
        for (auto ss: game->out_edges[s]) {
            V.clear();
            int target = get<0>(ss); int guard = get<1>(ss);
            /* We compute the new V mapping for edge s->s' after which we update W=MAX(W,V). */
            for (auto& p : U[target]) {
                ConfSet psi = (p.second & game->edge_guards[guard]);
                if (psi != emptyset) {
                    cout << p.second << std::endl;
                    for (int x : p.first) cout << x << " ";
                    cout << std::endl;
                    ProgM m = vector<int>(l+1);
                    minProg(game->priority[s], p, m);
                    V[m] |= psi;
                    cout << psi << std::endl;
                }
            }
            if (game->owner[s]) { // Owner is odd
                MAX(W, V, s);
                cout << "";
            } else { // Owner is even
                MIN(W, V, s);
                cout << "";
            }
        }
        fillInW(W);
        bool updated = false;
        /* TODO:
         *  We are updating U incorrectly. It should be  MAX(U,W)*/
        updateU(W, s, updated);
        cout << "State s=" << s << std::endl;
        for (auto &u : U[s]) {
            cout << u.second << "For Progress Measure: ( ";
            for (int i : u.first) {
                cout << i << " ";
            }
            cout << ")" << std::endl;
        }
        if(updated) {
            /* We check if our new Progress Measure mapping is different than we mapping we already have
             * for U[s]. If this is the case, we update U[s] with W. */
            /* TODO:
             *  Technically W should be defined for all configurations in U[s]. Check if we can do this
             *  updating in a simpler way. */
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
