//
// Created by koen on 24-02-21.
//
#include "queue"

#include "VPG_PP.h"


VPGPPSolver::VPGPPSolver(VPGame *game):
    game(game) {
    VPGPPSolver::emptyvertexset = VertexSetZlnk(game->n_nodes);
}

void VPGPPSolver::attract(int player, VertexSetZlnk *subV, std::vector<ConfSet> *vc) {
    removeFromBigV(subV, vc);
    attractQueue(player, subV, vc);
}

void VPGPPSolver::removeFromBigV(VertexSetZlnk *bigA, vector<ConfSet> *ac) const { int a = 0; }

void VPGPPSolver::attractQueue(int player, VertexSetZlnk *bigA, std::vector<ConfSet> *ac) const {
    queue<int> qq;
#ifdef VertexSetZlnkIsBitVector
    for(int vi = 0;vi<game->n_nodes;vi++){
        if(!(*bigA)[vi])
            continue;
#else
        for (const auto& vi : *bigA) {
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
            if(game->owner[vi] == player){
                attracted = (*C)[vi];
                attracted &= (*ac)[vii];
                attracted &= game->edge_guards[gi];
            } else {
                attracted = (*C)[vi];
                for(auto & j : game->out_edges[vi]){
                    int target = target(j);
                    ConfSet s = game->bigC;
                    ConfSet s2 = game->bigC;
                    s -= game->edge_guards[edge_index(j)];
                    s2 -= (*C)[target];
                    s |= s2;
                    attracted &= s;
                }
            }
            if(attracted == emptyset)
                continue;
            if(!(*bigA)[vi]){
                // add vertex to attracted set
                (*bigA)[vi] = true;
                (*ac)[vi] = attracted;
            } else {
                (*ac)[vi] |= attracted;
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
    VertexSetZlnk vertex_set = get<0>(regions[p]);
    std::vector<ConfSet> conf_set = get<1>(regions[p]);
    for (int i = 0; i < vertex_set.size(); i++) {
        if (vertex_set[i]) { // If vertex is in the region
            // Now reset `region` and `regions` vectors
            for (int j = 0; j < region[i].size(); j++) {
                if (get<1>(region[i][j]) == p && p != game->priority[i]) { // Now reset Confs of priority p
                    ConfSet new_conf = (get<0>(region[i][0]) & conf_set[i]);
                    region[i][0] = std::make_tuple(new_conf, game->priority[i]);
                    strategy[i][0] = std::make_tuple(new_conf, game->priority[i]);
                    region[i].erase(region[i].begin() + j);
                    strategy[i].erase(strategy[i].begin() + j);
                }
            }
        }
    }
}

/**
 *
 * @param bigV
 * @param p
 * @return
 */
bool VPGPPSolver::setupRegion(VertexSetZlnk *bigV, int p, std::vector<ConfSet> *vc) {
    // First, we make sure the region is empty (should always be the case).
    if (get<0>(regions[p]) != emptyvertexset) resetRegion(p);

    if (*bigV == emptyvertexset) return false;
    /* TODO:
     *  In the attractor, also update the strategy vector for each vertex. */
    attract(p%2, bigV, vc);
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
    VertexSetZlnk region_set = get<0>(regions[p]);
    std::vector<ConfSet> region_confs = get<1>(regions[p]);
    // See if the region is closed in the subgame
    for (int j = 0; j < region_set.size(); j++) {
        if (region_set[j]) {
            if (game->owner[j] != a) {
                return closedSubgame(p, region_confs, j);
            }
        }
    }

    // Next, we check if region is closed in entire game
    int lowest_region = -1;
    for (int j = 0; j < region_set.size(); j++) { // TODO: deduplicate this with above code (lambda function).
        if (region_set[j] && game->owner[j] != a) {
            lowest_region = findLowestRegion(p, region_confs, lowest_region, j);
        }
    }
    return lowest_region;
}

int VPGPPSolver::findLowestRegion(int p, const vector<bdd> &region_confs, int lowest_region, int j) {
    ConfSet vertex_confs = region_confs[j];
    for (auto &v : game->out_edges[j]) {
        auto &target_region = region[get<0>(v)];
        ConfSet edge_guard = game->edge_guards[get<1>(v)];
        for (auto &c : target_region) {
            ConfSet target_confs = get<0>(c);
            if ((vertex_confs & edge_guard & target_confs) != emptyset) {
                int region_priority = get<1>(c);
                if (region_priority < p &&
                        (region_priority < lowest_region || lowest_region == -1)) {
                    lowest_region = region_priority;
                }
            }
        }
    }
    return lowest_region;
}

int VPGPPSolver::closedSubgame(int p, const vector<bdd> &region_confs, int j) {
    ConfSet vertex_confs = region_confs[j];
    for (auto &v : game->out_edges[j]) {
        auto &target_region = region[get<0>(v)];
        ConfSet edge_guard = game->edge_guards[get<1>(v)];
        for (auto &c : target_region) {
            ConfSet target_confs = get<0>(c);
            if ((vertex_confs & edge_guard & target_confs) != emptyset) {
                if (get<1>(c) < p) return -2; // Found a reachable vertex of lower priority.
            }
        }
    }
}

/**
 * Promote region[from] to priority of regions[to]. Then computes attractor.
 * @param from priority that we are promoting from.
 * @param to priority we are promoting to.
 */
void VPGPPSolver::promote(int from, int to) {
    VertexSetZlnk promote_region = get<0>(regions[from]);
    std::vector<ConfSet> promote_confs = get<1>(regions[from]);
    promote_region |= get<0>(regions[to]);
    for (int i = 0; i < promote_confs.size(); i++) promote_confs[i] |= get<1>(regions[to])[i];
    regions[to] = std::make_tuple(promote_region, promote_confs);
    regions[from] = std::make_tuple(emptyvertexset, std::vector<ConfSet>(game->n_nodes));
    for (int i = from; i < to; i++) {
        // Reset all regions
        resetRegion(i);
    }
    attract(from%2, &promote_region, &promote_confs);
}

/**
 * Set all vertices with configurations Confs to solved for player p%2.
 * @param p the priority of the region that is solved.
 */
void VPGPPSolver::setDominion(int p) {
    const int a = p%2;
    VertexSetZlnk v = get<0>(regions[p]);
    std::vector<ConfSet> c = get<1>(regions[p]);
    for (int i = 0; i < v.size(); i++) {
        if (v[i]) {
            /* TODO: Also make sure that we set `region` or other vector to special value to indicate that
             *       this configuration has already been solved and is no longer part of the subgame.
             *       (Maybe subtract confs in winning_0 and winning_1 when resetting the game)*/
            if (a) {
                game->winning_1[i] |= c[i];
            } else {
                game->winning_0[i] |= c[i];
            }

        }
    }
}

// Run the solver
void VPGPPSolver::run() {
    max_prio = game->priority[0];
    /* TODO:
     *  Change from vector<ConfSet> to vector<int> which points to index of region vector, making it easier to do
     *  reset, setDmoninion and other operations. */
    regions = std::vector<std::tuple<VertexSetZlnk, std::vector<ConfSet>>>(max_prio+1);
    region = new vector<std::tuple<ConfSet, int>>[game->n_nodes];
    strategy = new std::vector<std::tuple<ConfSet, int>>[game->n_nodes];
    inverse = new int[max_prio+1];

    // Initialise the strategy array, with entire conf pointing to -1.
    for (int i = 0; i < game->n_nodes; i++) strategy[i].emplace_back(fullset, -1);
    // Initialise region array, where initially the region of vertex i points to fullset to priority of i.
    for (int i = 0; i < game->n_nodes; i++) region[i].emplace_back(std::make_tuple(fullset, game->priority[i]));

    /** We loop over the vertices and do PP algorithm. We first construct a vector with all the
     * vertices with the same priority. Afterwards, {@code i} always points to the first vertex with
     * priority < p. */
    int i = 0;
    while (i < game->n_nodes) {
        int p = game->priority[i];
        inverse[p] = i; // Keep index in case we promote and need to reset.
        VertexSetZlnk current_region(game->n_nodes);
        std::vector<ConfSet> *vc = new vector<ConfSet>(game->n_nodes);
        bool reset = true;
        // Look for all vertices of priority p that still have some confs enabled
        while (i < game->n_nodes && game->priority[i] == p) {
            (current_region)[i] = true;
            (*vc)[i] = (*C)[i];
            if ((*C)[i] != emptyset) reset = false;
            i++;
        }
        if (i == game->n_nodes) break; // We have run out of vertices to process
        if (reset) continue; // We skip if the region we are considering has no vertex enabled


        /* Now create the region by attracting nearby vertices and see if the region is
         * open or closed. */
        if (setupRegion(&current_region, p, vc)) {
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