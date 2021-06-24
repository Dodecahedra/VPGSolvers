//
// Created by koen on 23-06-21.
//


#include <algorithm>
#include "VPG_SCC.h"

VPG_SCC::VPG_SCC(VPGame *game):
    game(game) {
    V = new VertexSetZlnk(game->n_nodes);
    C = new vector<ConfSet>(game->n_nodes);
    for (int i = 0; i < game->n_nodes; i++) {
        (*V)[i] = true;
        (*C)[i] = game->bigC;
    }
}

void VPG_SCC::tarjanTSCC(vector<unordered_set<int>> *tscc_map) {
    index = new int[game->n_nodes]{0};
    lowlink = new int[game->n_nodes]{0};
    onstack = new bool[game->n_nodes]{false};
    int idx = 1;
    stack<int> S = stack<int>();
    for(int v = 0; v < game->n_nodes; v++) {
        if (index[v] == 0){
            strongconnect(v, &idx, &S, tscc_map);
        }
    }
    /* TODO:
     *  We can make this more efficient since we know that once we found a non-terminal element,
     *  all SCCs after it are non-terminal, until we hit the index of the new `strongconnect` search. */
    tscc_map->erase(
            std::remove_if(
                    tscc_map->begin(),
                    tscc_map->end(),
                    [this](unordered_set<int> p) {
                        return is_terminal(p);
                    }),
            tscc_map->end());
    delete[] index;
    delete[] lowlink;
    delete[] onstack;
}

bool VPG_SCC::is_terminal(unordered_set<int> &p) {
    for (int v : p) {
        for (auto e : game->out_edges[v]) {
            int w = target(e);
            ConfSet edge_guard = game->edge_guards[edge_index(e)];
            if (((*C)[v] & edge_guard & (*C)[w]) != emptyset) {
                if ((p).count(w) == 0) return true;
            }
        }
    }
    return false;
}

void VPG_SCC::strongconnect(int v, int *idx, std::stack<int> *S,
                            vector<unordered_set<int>> *tscc_map) {
    index[v] = *idx;
    lowlink[v] = *idx;
    (*idx)++; // Increase value stored at pointer idx.
    S->push(v);
    onstack[v] = true;
    for(auto e : game->out_edges[v]) {
        int w = target(e);
        ConfSet edge_guard = game->edge_guards[edge_index(e)];
        if (((*C)[v] & edge_guard & (*C)[w]) != emptyset) {
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
        tscc_map->push_back(current_scc); // Add current SCC to our `tscc_map`
    }
}

void VPG_SCC::run() {
    /* TODO:
     *  Iteratively compute the terminal SCCs and solve them using Zielonka or Priority Promotion. Adjust
     *  the underlying game (*V and *C) and recompute terminal SCCs until we have solved the entire game. */
    vector<unordered_set<int>> map = vector<unordered_set<int>>();
    tarjanTSCC(&map);

}
