//
// Created by koen on 24-02-21.
//
#include <cassert>
#include "queue"
#include "unordered_map"
#include "VPG_PP.h"

/**
 * Implementation of the priority promotion algorithm for VPGs.
 */

VPG_PP::VPG_PP(VPGame *game):
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

void VPG_PP::attract(int p) {
    removeFromBigV(p);
    attractQueue(p);
}

/**
 * Removes the region in regions[p] from the game (*V).
 * @param priority parity of the region.
 */
void VPG_PP::removeFromBigV(int priority) {
    VertexSetZlnk current_region = regions[priority];
    for (int i = 0; i < current_region.size(); i++) {
        if (current_region[i]) {
            (*C)[i] -= region[i][priority];
            if ((*C)[i] == emptyset) (*V)[i] = false;
        }
    }
}

void VPG_PP::attractQueue(int priority) {
    queue<int> qq;
#ifdef VertexSetZlnkIsBitVector
    for(int vi = 0;vi<game->n_nodes;vi++){
        if(!regions[priority][vi])
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
            if(game->owner[vi] == priority % 2){
                attracted = (*C)[vi];
                attracted &= region[vii][priority]; // <-- Why arent we taking the union of all out-edges?
                attracted &= game->edge_guards[gi];
                // Vertex is owned by player p
                strategy[vi].emplace(priority, vii);
            } else {
                attracted = (*C)[vi];
                for(auto & j : game->out_edges[vi]){
                    int target = target(j);
//                    if ((*V)[target]) {
                        ConfSet s = game->bigC;
                        ConfSet s2 = game->bigC;
                        s -= game->edge_guards[edge_index(j)];
//                        s2 -= (*C)[target];/// <-----
                        s2 &= region[target][priority];
                        s |= s2;
                        attracted &= s;
//                    }
                }
            }
            if(attracted == emptyset)
                continue;
            if(!regions[priority][vi]){
                // add vertex to attracted set
                region[vi][game->priority[vi]] -= attracted;
                if (region[vi][game->priority[vi]] == emptyset)
                    region[vi].erase(game->priority[vi]);

                region[vi][priority] |= attracted;
                if (region[vi][priority] != emptyset)
                    regions[priority][vi] = true;
            } else {
                region[vi][game->priority[vi]] -= attracted;
                region[vi][priority] |= attracted;
//                region[vi][game->priority[i]] -= attracted; // <-- What is going on here???
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
void VPG_PP::resetRegion(int p) {
    VertexSetZlnk vertex_set = regions[p];
    for (int i = 0; i < game->n_nodes; i++) {
        if (vertex_set[i]) { // If vertex is in region {@code p}
            int priority_i = game->priority[i];
            region[i][priority_i] |= region[i][p];
            /* If we have a conf set for vertex {@code i}, make sure that vertex {@code i} is added to the
             * {@code regions} vertex set. */
//            if (region[i][priority_i] != emptyset) regions[priority_i][i] = true;
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
 * Given a region[p], compute its attractor. This removes the attracted region from the underlying game.
 * @param p priority of the region we are creating.
 * @return true if we succesfully setup a region, false if the initial region is empty.
 */
bool VPG_PP::setupRegion(int p) {
    if (regions[p] == emptyvertexset) return false;
    attract(p);
    return true;
}

/**
 * Get the status of region with priority p.
 * @param p priority of the region that we are considering.
 * @return  -2 if region is open, -1 if region is closed and priority c if we
 *          can promote the region to priority c.
 */
int VPG_PP::getRegionStatus(int p) {
    const int a  = p%2;
    VertexSetZlnk region_set = regions[p];
    // See if the region is closed in the subgame
    int lowest_region = -1;
    for (int j = 0; j < region_set.size(); j++) {
        if (region_set[j]) {
            if (game->owner[j] != a) {
                ConfSet vertex_confs = region[j][p];
                for (auto &v : game->out_edges[j]) {
                    ConfSet edge_guard = game->edge_guards[get<1>(v)];
                    auto it = region[get<0>(v)].begin();
                    while (it != region[get<0>(v)].end()) {
                        if ((vertex_confs & edge_guard & it->second) != emptyset) {
                            int region_priority = it->first;
                            if (region_priority < p) return -2; // Found a reachable vertex of lower priority.
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
 */ // TODO: Check this method
void VPG_PP::promote(int from, int to) {
    VertexSetZlnk promote_region = regions[from];
    for (int i = 0; i < promote_region.size(); i++) {
        if (region[i].count(from)) {
            region[i][to] |= region[i][from];
            region[i].erase(from);
        }
    }
    regions[to] |= promote_region;
    regions[from] = emptyvertexset;
    for (int i = 0; i < game->n_nodes; i++) {
        assert(region[i][from] == emptyset);
    }

    for (int i = from; i < to; i++) {
        resetRegion(i);
    }
    attract(to);
    cout << "";
}

/**
 * Set all vertices with configurations Confs to solved for player p%2.
 * @param p the priority of the region that is solved.
 */ // TODO: Check this method
void VPG_PP::setDominion(int p) {
    /* First, we add all the regions back to the game, to compute the attractor in the entire
     * subgame V. */
    for (int i = 0; i < game->n_nodes; i ++) {
        for (auto &t : region[i]) {
            (*V)[i] = (*V)[i] || regions[t.first][i];
            (*C)[i] |= t.second;
        }
    }
    cout << "";
    attract(p);
    const int a = p%2;
    VertexSetZlnk v = regions[p]; // Vertices in region {@code p}
    for (int i = 0; i < game->n_nodes; i++) {
        if (v[i]) {
            if (a) {
                game->winning_1[i] |= region[i][p];
            } else {
                game->winning_0[i] |= region[i][p];
            }
            /* We check that the configuration that we solved is */   assert(((*C)[i] & region[i][p]) == emptyset);
            /* removed from the underlying game. */   assert((*C)[i] == emptyset ? (*V)[i] == false : (*V)[i] == true);
            region[i].erase(p); // Remove the vertex-configuration pair
        }
    }
    /* Remove regions from the game. */
    for (int i = 0; i < max_prio+1; i++) regions[i] = VertexSetZlnk(game->n_nodes);
    for(int i = 0; i < game->n_nodes; i++) {
        region[i].clear();
    }
    setUpRegions();
    cout << " ";
}


void VPG_PP::run() {
    max_prio = game->priority[0];
    // TODO: See if we can replace these static-sized vectors with arrays.
    regions = std::vector<VertexSetZlnk>(max_prio+1);
    region =  std::vector<std::unordered_map<int, ConfSet>>(game->n_nodes);
    strategy = std::vector<std::unordered_map<int, int>>(game->n_nodes);
    inverse = new int[max_prio+1];

    // Initialise region array, where initially the region of vertex i points to fullset to priority of i.
    setUpRegions();

    for (int i = 0; i < max_prio+1; i++) regions[i] = VertexSetZlnk(game->n_nodes);

    /** We loop over the vertices and do PP algorithm. We first construct a vector with all the
     * vertices with the same priority. Afterwards, {@code i} always points to the first vertex with
     * priority < p. */
    int i = 0;
    promotions = 0;
    while (i < game->n_nodes) {
        int p = game->priority[i];
        inverse[p] = i; // Keep index in case we promote and need to reset.
        bool reset = true;
        // Look for all vertices of priority p that still have some confs enabled
        while (i < game->n_nodes && game->priority[i] == p) {
            regions[p][i] = regions[p][i] || (*V)[i];
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
                int regionStatus = getRegionStatus(p);
                if (regionStatus == -2) {
                    // Region is open, so continue search in subgame G<p.
                    break;
                } else if (regionStatus == -1) {
                    /* We found a closed region, i.e. dominion. Set the vertices in the dominion
                     * as solved and restart the algorithm, but with the dominion D removed from the game. */
                    setDominion(p);
                    i = 0; // Restart loop from the beginning
                    break;
                } else {
                    /* We found a region which can be promoted. Promote the region to priority {@code regionStatus}. */
                    promote(p, regionStatus);
                    promotions++;
                    i = inverse[regionStatus];
                    p = regionStatus;
                }
            }
        } else {
            // Region was empty, go to the next priority.
            continue;
        }
    }
}

void VPG_PP::setUpRegions() {
    for (int i = 0; i < game->n_nodes; i++) {
        VPG_PP::region[i][game->priority[i]] = (*C)[i];
    }
}
