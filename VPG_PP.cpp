//
// Created by koen on 24-02-21.
//
#include <cassert>
#include "queue"
#include "unordered_map"
#include "VPG_PP.h"
#include "Conf.h"


VPGPPSolver::VPGPPSolver(VPGame *game):
    game(game) {
    emptyvertexset = VertexSetZlnk(game->n_nodes);
    V = new VertexSetZlnk(game->n_nodes);
    C = new vector<ConfSet>(game->n_nodes);
    // Initialise V and C (initially full sets)
    for (int i = 0; i < game->n_nodes; i++) {
        (*V)[i] = true;
        (*C)[i] = game->bigC;
    }
}

void VPGPPSolver::attract(int p) {
    removeFromBigV(p);
    attractQueue(p);
}

/**
 * Removes the region in regions[p] from the game (*V).
 * @param p parity of the region.
 */
void VPGPPSolver::removeFromBigV(int p) {
    VertexSetZlnk current_region = regions[p];
    for (int i = 0; i < current_region.size(); i++) {
        if (current_region[i]) {
            (*C)[i] -= region[i][p];
            if ((*C)[i] == emptyset) (*V)[i] = false;
        }
    }
}

void VPGPPSolver::attractQueue(int p) {
    queue<int> qq;
#ifdef VertexSetZlnkIsBitVector
    for(int vi = 0;vi<game->n_nodes;vi++){
        if(!regions[p][vi])
            continue;
#else
        for (const auto& vi : *regions[p]) {
#endif
        qq.push(vi);
    }
    while(!qq.empty())
    {
        int vii = qq.front();
        qq.pop();

        for(int i = 0;i<game->in_edges[vii].size();i++){
            int vi = target(game->in_edges[vii][i]);
            int gi = edge_index(game->in_edges[vii][i]);
            if(!(*V)[vi]) // vertex not in the playing area anymore
                continue;
            // try to attract as many configurations as possible for vertex vi
            ConfSet attracted;
            // This follows the attractor definition precisely, see pseudo code and definitions in the report
            if(game->owner[vi] == p%2){
                attracted = (*C)[vi];
                attracted &= region[vii][p];
                attracted &= game->edge_guards[gi];
                // Vertex is owned by player p
                strategy[vi].emplace(p, vii);
            } else {
                attracted = (*C)[vi];
                for(auto & j : game->out_edges[vi]){
                    int target = target(j);
                    if ((*V)[target]) {
                        ConfSet s = game->bigC;
                        ConfSet s2 = game->bigC;
                        s -= game->edge_guards[edge_index(j)];
                        s2 &= region[target][p];
                        s |= s2;
                        attracted &= s;
                    }
                }
            }
            if(attracted == emptyset)
                continue;
            if(!regions[p][vi]){
                // add vertex to attracted set
                regions[p][vi] = true;
                region[vi][p] |= attracted; // TODO: Check if already inited
                cout << "Adding to vertex: " << vi << " ";
                cout << attracted << std::endl;
                cout << "Vertex had: " << region[vi][game->priority[vi]] << std::endl;
                region[vi][game->priority[vi]] -= attracted;
            } else {
                region[vi][p] |= attracted;
                region[vi][game->priority[i]] -= attracted;
                cout << "Adding to vertex: " << vi << " ";
                cout << attracted << std::endl;
                cout << "Vertex had: " << region[vi][game->priority[vi]] << std::endl;
                region[vi][game->priority[vi]] -= attracted;
            }
            // remove attracted confs from the game
            (*C)[vi] -= attracted;
            if((*C)[vi] == emptyset)
            {
                (*V)[vi] = false;
            }
            // if we attracted anything we need to reevaluate the predecessors of vi
            qq.push(vi);
        }
    }
}

/**
 * Resets the region of priority p.
 * @param p priority of region to be reset.
 * @summary Goes over all <vertex,Conf> pairs in regions[p] and resets those to the original priority.
 *  Does not reset if priority of the vertex is set to -1 (if solved).
 */
void VPGPPSolver::resetRegion(int p) {
    /* TODO:
     *  [x] Currently vertices are not added back to the VPGame. */
    VertexSetZlnk vertex_set = regions[p];
    for (int i = 0; i < vertex_set.size(); i++) {
        if (vertex_set[i]) { // If vertex is in the region
            // Now reset `region`, `regions` and `strategy` vectors
            int priority_i = game->priority[i];
            region[i][priority_i] |= region[i][p];
            strategy[i].erase(p);
            if (priority_i != p) {
                regions[p][i] = false;
                region[i].erase(p);
            }
            // Add vertex i and its configuration back to the game.
            (*V)[i] = true;
            (*C)[i] |= region[i][priority_i];
        }
    }
}

/**
 *
 * @param bigV
 * @param p
 * @return
 */
bool VPGPPSolver::setupRegion(int p) {
    // First, we make sure the region is empty (should always be the case).

    if (regions[p] == emptyvertexset) return false;
    /* TODO:
     *  [] In the attractor, also update the strategy vector for each vertex. */
    attract(p);
    return true;
}

/**
 * Get the status of region with priority p.
 * @param i index pointing to the first vertex in the subgame G<p.
 * @param p priority of the region that we are considering.
 * @return  -2 if region is open, -1 if region is closed and priority c if we
 *          can promote the region to priority c.
 */
int VPGPPSolver::getRegionStatus(int i, int p) {
    const int a  = p%2;
    VertexSetZlnk region_set = regions[p];
    // See if the region is closed in the subgame
    int lowest_region = -1;
    for (int j = 0; j < region_set.size(); j++) {
        if (region_set[j]) {
            if (game->owner[j] != a) {
                ConfSet vertex_confs = region[j][p];
                for (auto &v : game->out_edges[j]) {
                    /* TODO:
                     *  This has not yet been updated to handle the new region function. */
                    ConfSet edge_guard = game->edge_guards[get<1>(v)];
                    auto it = region[get<0>(v)].begin();
                    while (it != region[get<0>(v)].end()) {
                        if ((vertex_confs & edge_guard & it->second) != emptyset) {
                            int region_priority = it->first;
                            /* TODO:
                             *  Currently we set all confs that have been solved to -1. This is why it is sometimes
                             *  saying the region is open while really it is closed. See if we want to set some other
                             *  special number for this (if we want to keep track of this at all). */
                            if (region_priority < p && region_priority != -1) return -2; // Found a reachable vertex of lower priority.
                            if ((region_priority < lowest_region || lowest_region == -1)
                                    && region_priority != p) {
                                lowest_region = region_priority;
                            }
                        }
                        it++;
                    }
                }
            }
        }
    }
    return lowest_region;
}

/**
 * Promote region[from] to priority of regions[to]. Then computes attractor.
 * @param from priority that we are promoting from.
 * @param to priority we are promoting to.
 */
void VPGPPSolver::promote(int from, int to) {
    VertexSetZlnk promote_region = regions[from];
    for (int i = 0; i < promote_region.size(); i++) {
        if (region[i].count(from)) {
            /* TODO: check how a bdd is instantiated, otherwise have to check if [to] is already initialised. */
            cout << "Adding to vertex: " << i << ", promoting from: " << from << " to: " << to << std::endl;
            cout << region[i][from] << std::endl;
            cout << "Vertex had: " << region[i][to] << std::endl;
            region[i][to] |= region[i][from];
            region[i].erase(from);
            cout << "Vertex now has: " << region[i][to] << std::endl;
        }
    }
    regions[to] |= promote_region;
    regions[from] = emptyvertexset;

    for (int i = from; i < to; i++) {
        // Reset all regions
        resetRegion(i);
    }
    attract(to);
}

/**
 * Set all vertices with configurations Confs to solved for player p%2.
 * @param p the priority of the region that is solved.
 */
void VPGPPSolver::setDominion(int p) {
    const int a = p%2;
    VertexSetZlnk v = regions[p];
    for (int i = 0; i < v.size(); i++) {
        if (v[i]) {
            if (a) {
                game->winning_1[i] |= region[i][p];
                cout << "Winning_1 sets of: ";
                cout << " " << game->winning_1[i] << " for vertex: " << i << std::endl;
            } else {
                game->winning_0[i] |= region[i][p];
                cout << "Winning_0 sets of: ";
                cout << " " << game->winning_0[i] << " for vertex: " << i << std::endl;
            }
            // Set configurations attracted to regions[p] as solved (-1) and remove p from map.
            region[i][-1] |= region[i][p];
            region[i].erase(p);
            /* We check that the configuration that we solved is */   assert(((*C)[i] & region[i][p]) == emptyset);
            /* removed from the underlying game. */   assert((*C)[i] == emptyset ? (*V)[i] == false : (*V)[i] == true);
        }
    }
}


void VPGPPSolver::run() {
    max_prio = game->priority[0];
    regions = std::vector<VertexSetZlnk>(max_prio+1);
    region =  std::vector<std::unordered_map<int, ConfSet>>(game->n_nodes);
    strategy = std::vector<std::unordered_map<int, int>>(game->n_nodes);
    inverse = new int[max_prio+1];

    // Initialise region array, where initially the region of vertex i points to fullset to priority of i.
    for (int i = 0; i < game->n_nodes; i++) {
        region[i][game->priority[i]] = game->bigC;
    }

    for (int i = 0; i < max_prio+1; i++) regions[i] = VertexSetZlnk(game->n_nodes);

    /** We loop over the vertices and do PP algorithm. We first construct a vector with all the
     * vertices with the same priority. Afterwards, {@code i} always points to the first vertex with
     * priority < p. */
    int i = 0;
    while (i < game->n_nodes) {
        int p = game->priority[i];
        inverse[p] = i; // Keep index in case we promote and need to reset.
        bool reset = true;
        // Look for all vertices of priority p that still have some confs enabled
        while (i < game->n_nodes && game->priority[i] == p) {
            regions[p][i] = regions[p][i] || (*V)[i]; // Only returns false if all confs have already been attracted.
            region[i][p] |= (*C)[i];
            if (region[i][p] != emptyset) reset = false;
            i++;
        }
        if (reset) continue; // We skip if the region we are considering has no vertex enabled


        /* Now create the region by attracting nearby vertices and see if the region is
         * open or closed. */
        if (setupRegion(p)) {
            // We created a region, check whether it is open/closed.
            while (true) {
                int c = getRegionStatus(i, p);
                if (c == -2) {
                    // Region is open, so continue search in subgame G<p.
                    break;
                } else if (c == -1) {
                    /* We found a closed region, i.e. dominion. Set the vertices in the dominion
                     * as solved and restart the algorithm, but with the dominion D removed from the game. */
                    setDominion(p);
                    i = 0;
                    break;
                } else {
                    /* We found a region which can be promoted. Promote the region to priority c. */
                    promote(p, c);
                    promotions++;
                    i = inverse[c];
                    p = c;
                }
            }
        } else {
            // Region was empty, go to the next priority.
            continue;
        }
    }
}