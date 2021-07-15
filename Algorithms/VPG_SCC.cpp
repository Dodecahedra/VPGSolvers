//
// Created by koen on 23-06-21.
//


#include <algorithm>
#include <chrono>
#include "VPG_SCC.h"

VPG_SCC::VPG_SCC(VPGame *game):
    game(game) {
    this->bigV = new VertexSetZlnk(game->n_nodes);
    this->vc = new vector<ConfSet>(game->n_nodes);
    for (int i = 0; i < game->n_nodes; i++) {
        (*bigV)[i] = true;
        (*vc)[i] = game->bigC;
    }
    emptyvertexset = VertexSetZlnk(game->n_nodes);
}

VPG_SCC::VPG_SCC(VPGame *game, VectorBoolOptimized *subV, vector<bdd> *subvc):
    game(game) {
    this->bigV = subV;
    this->vc = subvc;
    emptyvertexset = VertexSetZlnk(game->n_nodes);
}

tuple<int, int> VPG_SCC::getHighLowPrio() {
    int highest = 0;
    int lowest = INT32_MAX;
    for(int vi = 0;vi<game->n_nodes;vi++){
        if(!(*bigV)[vi])
            continue;
        int prio = game->priority[vi];
        if(prio < lowest) lowest = prio;
        if(prio > highest) highest = prio;
    }
    return make_tuple(highest, lowest);
}

void VPG_SCC::removeFromBigV(VertexSetZlnk * bigA, vector<ConfSet> *ac){
    for (int vi = 0;vi < game->n_nodes;vi++){
        if(!(*bigA)[vi])
            continue;
        removeFromBigV(vi, (*ac)[vi]);
    }
}

void VPG_SCC::removeFromBigV(int i, ConfSet c) {
    (*vc)[i] -= c;
    if((*vc)[i] == emptyset)
    {
        (*bigV)[i] = false;
    }
}

void VPG_SCC::getVCWithPrio(VertexSetZlnk *bigA, vector<ConfSet> *ac, int prio) {
    // use inverse priority assignment function
    for (const auto& vi : game->priorityI[prio]) {
        if((*bigV)[vi]){
            (*bigA)[vi] = true;
            (*ac)[vi] = (*vc)[vi];
        }
    }
}

void VPG_SCC::attr(int player, VertexSetZlnk *bigA, vector<ConfSet> *ac) {
    auto start = std::chrono::high_resolution_clock::now();

    removeFromBigV(bigA, ac);
    attrQueue(player, bigA, ac);
    auto end = std::chrono::high_resolution_clock::now();

    auto elapsed =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    cout << "Attracting took " << elapsed.count() << "ns" << endl;
    attracting += elapsed.count();
    attractions++;
}


void VPG_SCC::attrQueue(int player, VertexSetZlnk *bigA, vector<ConfSet> *ac) {
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
            if(!(*bigV)[vi]) // vertex not in the playing area anymore
                continue;
            // try to attract as many configurations as possible for vertex vi
            ConfSet attracted;
            // This follows the attractor definition precisely, see preudo code and definitions in the report
            if(game->owner[vi] == player){
                attracted = (*vc)[vi];
                attracted &= (*ac)[vii];
                attracted &= game->edge_guards[gi];
            } else {
                attracted = (*vc)[vi];
                for(auto & j : game->out_edges[vi]){
                    int target = target(j);
                    ConfSet s = game->bigC;
                    ConfSet s2 = game->bigC;
                    s -= game->edge_guards[edge_index(j)];
                    s2 -= (*vc)[target];
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
            (*vc)[vi] -= attracted;
            if((*vc)[vi] == emptyset)
            {
                (*bigV)[vi] = false;
            }
            // if we attracted anything we need to reevaluate the predecessors of vi
            qq.push(vi);
        }
    }
}

void VPG_SCC::unify(VertexSetZlnk *bigA, vector<ConfSet> *ac, VertexSetZlnk *bigB, vector<ConfSet> *bc) {
    for (int vi = 0;vi < game->n_nodes;vi++){
        if(!(*bigB)[vi])
            continue;
        (*bigA)[vi] = true;
        (*ac)[vi] |= (*bc)[vi];
    }
}

void VPG_SCC::addTobigV(VertexSetZlnk *bigA, vector<ConfSet> *ac) {
    for (int i = 0; i < game->n_nodes; i++) {
        if (!(*bigA)[i])
            continue;
        (*bigV)[i] = true;
        (*vc)[i] |= (*ac)[i];
    }
}

void VPG_SCC::tarjanTSCC(list<unordered_set<int>> *tscc_map) {
    index = new int[game->n_nodes]{0};
    lowlink = new int[game->n_nodes]{0};
    onstack = new bool[game->n_nodes]{false};
    int idx = 1;
    stack<int> S = stack<int>();
    queue<int> t_scc_index = queue<int>();
    for(int v = 0; v < game->n_nodes; v++) {
        if (index[v] == 0 && (*bigV)[v]){
            strongconnect(v, &idx, &S, tscc_map);
            t_scc_index.push(tscc_map->size());
        }
    }
    auto it = tscc_map->begin(); int i = 0;
    while (it != tscc_map->end()) {
        if (is_terminal(*it)) {
            for (; i < t_scc_index.front(); i++) { // We remove all elements which are not terminal
                ++it;
                tscc_map->erase(std::prev(it));
            }
            t_scc_index.pop();
        } else {
            ++it; i++;
            if (t_scc_index.front() == i) t_scc_index.pop();
        }

    }
    delete[] index;
    delete[] lowlink;
    delete[] onstack;
}

bool VPG_SCC::is_terminal(unordered_set<int> &p) {
    for (int v : p) {
        for (auto e : game->out_edges[v]) {
            int w = target(e);
            ConfSet edge_guard = game->edge_guards[edge_index(e)];
            if (((*vc)[v] & edge_guard & (*vc)[w]) != emptyset) {
                if (p.count(w) == 0) return true;
            }
        }
    }
    return false;
}

void VPG_SCC::strongconnect(int v, int *idx, std::stack<int> *S, list<unordered_set<int>> *tscc_map) {
    index[v] = *idx;
    lowlink[v] = *idx;
    (*idx)++; // Increase value stored at pointer idx.
    S->push(v);
    onstack[v] = true;
    for(auto e : game->out_edges[v]) {
        int w = target(e);
        ConfSet edge_guard = game->edge_guards[edge_index(e)];
        if (((*vc)[v] & edge_guard & (*vc)[w]) != emptyset) {
            // Edge is enabled in the graph, now visit it.
            if (index[w] == 0) {
                strongconnect(w, idx, S, tscc_map);
                lowlink[v] = min(lowlink[v], lowlink[w]);
            } else if (onstack[w]) {
                lowlink[v] = min(lowlink[v], index[w]);
            }
        }

    }
    // If we find root vertex of SCC, pop vertices on stack and generate SCC.
    if (lowlink[v] == index[v]) {
        unordered_set<int> current_scc = unordered_set<int>(); // Set to store elements of SCC
        int w;
        do {
            w = S->top(); S->pop();
            onstack[w] = false;
            current_scc.emplace(w);
        } while (w != v);
        tscc_map->push_back(current_scc);
    }
}

void VPG_SCC::solve(VertexSetZlnk *W0bigV, vector<ConfSet> *W0vc, VertexSetZlnk *W1bigV, vector<ConfSet> *W1vc) {
    // While there are enabled vertices enabled, keep calculating the terminal sccs and solve them
    if ((*bigV) == emptyvertexset) {
        return;
    } else {
        // Compute the SCC decomposition
        while ((*bigV) != emptyvertexset) {
            list<unordered_set<int>> map = list<unordered_set<int>>();
            auto start = std::chrono::high_resolution_clock::now();
            tarjanTSCC(&map);
            tarjan_calls++;
            auto end = std::chrono::high_resolution_clock::now();
            tarjan_time +=
                    std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
                            .count();

            // For each terminal SCC, create a new subgame and call VPG_SCC solve
            for (auto &C : map) {
                auto *subV = new VertexSetZlnk(game->n_nodes);
                auto *subC = new vector<ConfSet>(game->n_nodes);
                for (auto v : C) {
                    (*subV)[v] = true;
                    (*subC)[v] |= (*vc)[v];
                }
                // Created subgameH, now solve them
                VPG_SCC subgameH(game, subV, subC);
                auto *bigA = new VertexSetZlnk(game->n_nodes);
                auto *ac = new vector<ConfSet>(game->n_nodes);
                auto[h, l] = subgameH.getHighLowPrio();
                int a = l % 2;
                subgameH.getVCWithPrio(bigA, ac, l);
                subgameH.attr(a, bigA, ac); // H = G - A
                auto *W0subV = new VertexSetZlnk(game->n_nodes);
                auto *W0subvc = new vector<ConfSet>(game->n_nodes);
                auto *W1subV = new VertexSetZlnk(game->n_nodes);
                auto *W1subvc = new vector<ConfSet>(game->n_nodes);
                subgameH.solve(W0subV, W0subvc, W1subV, W1subvc);
                tarjan_calls += subgameH.tarjan_calls;
                tarjan_time += subgameH.tarjan_time;
                attracting += subgameH.attracting;
                attractions += subgameH.attractions;
                // Create subV and subvc to store solution of the subgame we solved
                VertexSetZlnk *WMesubV;
                vector<ConfSet> *WMevc;
                VertexSetZlnk *WOpsubV;
                vector<ConfSet> *WOpvc;
                if (a == 0) {
                    WMesubV = W0subV;
                    WMevc = W0subvc;
                    WOpsubV = W1subV;
                    WOpvc = W1subvc;
                } else {
                    WMesubV = W1subV;
                    WMevc = W1subvc;
                    WOpsubV = W0subV;
                    WOpvc = W0subvc;
                }
                if ((*WOpsubV) == emptyvertexset) {
                    unify(WMesubV, WMevc, bigA, ac);
                } else {
                    auto *B = new VertexSetZlnk(game->n_nodes);
                    auto *bc = new vector<ConfSet>(game->n_nodes);
                    unify(B, bc, WOpsubV, WOpvc);
                    VPG_SCC subgame(game, subV, subC);
                    subgame.attr(1 - a, B, bc);
                    // Empty previous solution
                    std::fill(W0subV->begin(), W0subV->end(), false);
                    std::fill(W1subV->begin(), W1subV->end(), false);
                    fill(W0subvc->begin(), W0subvc->end(), emptyset);
                    fill(W1subvc->begin(), W1subvc->end(), emptyset);
                    subgame.solve(W0subV, W0subvc, W1subV, W1subvc);
                    tarjan_calls += subgame.tarjan_calls;
                    tarjan_time += subgame.tarjan_time;
                    attracting += subgame.attracting;
                    attractions += subgame.attractions;
                    unify(WOpsubV, WOpvc, B, bc);
                }
                attr(0, W0subV, W0subvc);
                attr(1, W1subV, W1subvc);
                unify(W0bigV, W0vc, W0subV, W0subvc);
                unify(W1bigV, W1vc, W1subV, W1subvc);
            }
        }
    }
}
