/***********************************************************************************************************************
 * In this project only algorithms and datastructures are implemented properly.
 *
 * The code in this file is not optimized and not up to standards; it is sufficient only for experimental applications
 * but not for any real application.
 **********************************************************************************************************************/
//
// Copy from VariabilityParityGames/Game.cpp
//

#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <unordered_set>
#include <algorithm>
#include "VariabilityParityGames/Algorithms/zlnkVPG.h"

#include "VPGame.h"
#define PARSER_LINE_SIZE  16777216

void VPGame::set_n_nodes(int nodes) {
    n_nodes = nodes;
    out_edges = new std::vector<std::tuple<int,int>>[n_nodes];
    in_edges = new std::vector<std::tuple<int,int>>[n_nodes];
    priority.resize(n_nodes);
    owner.resize(n_nodes);
    declared.resize(n_nodes);

    // Construct vectors containing the winning configurations fow player 0,1.
    winning_0.resize(n_nodes); winning_1.resize(n_nodes);
}


VPGame::VPGame() {

}

void VPGame::sort() {
    mapping = std::vector<int>(n_nodes);
    constructMapping(mapping);
    /* Using the identity mapping we just constructed, we now make a permutation mapping, where
     * we sort in ascending order of priority. */
    std::sort(mapping.begin(), mapping.end(),
         [&](const int &a, const int &b) {
            return priority[a] < priority[b];
        }
    );
    // Now sort all our values according to this mapping.
    vector<int> inverse(n_nodes);
    for (int i = 0; i < mapping.size(); i++) inverse[mapping[i]] = i;
    permute(inverse);
}

void VPGame::constructMapping(vector<int> &mapping) const {
    for (int i = 0; i < priority.size(); i++) {
        mapping[i] = i;
    }
}

void VPGame::permute(std::vector<int> &mapping) {
    // Update the target index in the `out_edges` vector.
    for (int j = 0; j < mapping.size(); j++) {
        /* For each tuple in `out_edges` list, update the `target` parameter according to new
         * mapping. */
        for (std::tuple<int,int> &oe: out_edges[j]) {
            int new_index = mapping[get<0>(oe)];
            oe = std::make_tuple(new_index, get<1>(oe));
        }
        for (std::tuple<int,int> &ie : in_edges[j]) {
            int new_index = mapping[get<0>(ie)];
            ie = std::make_tuple(new_index, get<1>(ie));
        }
    }
    // Now update the rest of the values (destroys mapping)
    for (int i = 0; i < mapping.size(); i++) {
        while (i != mapping[i]) {
            int swp = mapping[i];
            std::swap(priority[i], priority[swp]);
            std::swap(owner[i], owner[mapping[i]]);
            std::swap(declared[i], declared[mapping[i]]);;
            std::swap(out_edges[i], out_edges[mapping[i]]);
            std::swap(in_edges[i], in_edges[mapping[i]]);
            std::swap(winning_0[i], winning_0[mapping[i]]);
            std::swap(winning_1[i], winning_1[mapping[i]]);
            std::swap(mapping[i], mapping[swp]);
        }

    }
}

/**
 * Pre-processing step:
 * Eliminate self-loops in the graph by computing their attractor set and removing them from the game.
 * @param self_loops
 */
void VPGame::elimateSelfLoops() {
    auto* vertices_0 = new VertexSetZlnk(n_nodes);
    auto* confset_0 = new vector<ConfSet>(n_nodes);
    auto* vertices_1 = new VertexSetZlnk(n_nodes);
    auto* confset_1 = new vector<ConfSet>(n_nodes);

    for (int i = 0; i < n_nodes; i++) {
        for (auto e : out_edges[i]) {
            int v = target(e);
            if (v != i) continue;
            ConfSet guard = edge_guards[edge_index(e)];
            if (owner[v] != (priority[v]&1)) {
                for (auto ei : out_edges[v]) {
                    if (edge_index(ei) != edge_index(e)) {
                        guard -= edge_guards[edge_index(ei)];
                    }
                }
            }
            if (priority[v]&1) {
                (*confset_1)[v] |= guard;
                if (guard != emptyset) (*vertices_1)[v] = true;
            } else {
                (*confset_0)[v] |= guard;
                if (guard != emptyset) (*vertices_0)[v] = true;
            }
        }
    }
    zlnkVPG solver(this);
    solver.attr(0, vertices_0, confset_0);
    solver.attr(1, vertices_1, confset_1);
    for(int i = 0; i < n_nodes; i++) {
        if (vertices_0) {
            winning_0[i] |= (*confset_0)[i];
        }
        if (vertices_1) {
            winning_1[i] |= (*confset_1)[i];
        }
    }
}

/**
 * Reads VPG from file.
 * IMPORTANT: does not like trailing '\n' characters.
 * @param filename
 * @param specificconf
 */
void VPGame::parseVPGFromFile(const string &filename, const char *specificconf) {
    int c = 0;
    if(parsePG){
        c++;
        parseConfs("confs 1;");
    }
    std::ifstream infile(filename);

    char * s = new char[PARSER_LINE_SIZE];
    while (infile.good())
    {
        infile.getline(s, PARSER_LINE_SIZE,';');
        if(c == 0){
            // create bigC
            cout << "Found confs: " << s << '\n';
            parseConfs(s);
            if(strlen(specificconf) > 0)
                parseConfSet(specificconf,0,&bigC);
            c++;
        } else if(c == 1)
        {
            cout << "Found game: " << s << '\n';
            // init with parity
            parseInitialiser(s);
            c++;
        } else {
            // add vertex
            if(strlen(s) > 0)
                parseVertex(s);
        }
    }
    delete[] s;
    if (!infile.eof()) {
        throw std::string("could not open file");
    }
    buildInEdges();
}

void VPGame::parseVPGFromFile(const string &filename) {
    parseVPGFromFile(filename, "");
}

void VPGame::parseConfs(char * line) {
    while(*line == '\n' || *line == '\t' ||*line == ' ')
        line++;
    if(strncmp(line, "confs ",6) != 0) throw std::string("Expected confs");
    int i = 6;
    char c;
    do{
        c = line[i++];
    } while(c != '\0' && c != '+' && c != ';');
    bm_n_vars = i - 7;
    bm_vars.resize(bm_n_vars);
#ifdef subsetbdd
    bdd_init(2000000,2);
//    bdd_setcacheratio(200);
    bdd_setvarnum(bm_n_vars);
    vector<int> order;
    order.resize(bm_n_vars);
    for(i = 0;i<bm_n_vars;i++){
        order[i] = i;
    }
#ifdef randombddorder
    unsigned seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    shuffle (order.begin(), order.end(), default_random_engine(seed));
#else
    if(specificvarlast){
        order[bm_n_vars-1] = specificvar;
        order[specificvar] = bm_n_vars-1;
    }
#endif
    cout << "Bdd order: " ;
    for(i = 0;i<bm_n_vars;i++){
        cout << '[' << i << "]=" << order[i] << ", ";
    }
    cout << "\n";
    for(i = 0;i<bm_n_vars;i++) {
        bm_vars[order[i]] = bdd_ithvar(i);
    }
#endif
#ifdef subsetexplicit
    ConfSetExplicit::size = (1 << (bm_n_vars));
    fullset.items.resize(ConfSetExplicit::size);
    emptyset.items.resize(ConfSetExplicit::size);
    for(int i =0;i<1<<bm_n_vars;i++) {
        fullset.items[i] = true;
        emptyset.items[i] = false;
    }
    for(int i = 0;i<bm_n_vars;i++) {
        bm_vars[i] = ConfSetExplicit(bm_n_vars-i-1);
    }
#endif
    parseConfSet(line, 6, &bigC);
}

void VPGame::parseInitialiser(char *line) {
    while(*line == '\n' || *line == '\t' ||*line == ' ')
        line++;
    if(strncmp(line, "parity ",7) != 0) throw std::string("Expected parity");
    int parity = atoi(line + 7);
    set_n_nodes(parity);
}


int VPGame::parseConfSet(const char *line, int i, ConfSet *result) {
    if(parsePG){
        *result = fullset;
        return i+1;
    }
    bool inverse = false;
    if(line[i] == '!') {
        inverse = true;
        i++;
    }
    *result = emptyset;
    ConfSet entry = fullset;
    int var = 0;
    char c;
    string * seq;
    do
    {
        c = line[i++];
        if(c == 'F'){
            if(var != 0) throw std::string("Unexpected F");
            entry = emptyset;
        } else if (c == '0') {
            if (var > bm_n_vars) throw std::string("Too many bits");
            entry -= bm_vars[var];
            var++;
        } else if (c == '1') {
            if (var > bm_n_vars) throw std::string("Too many bits");
            entry &= bm_vars[var];
            var++;
        } else if (c == '-') {
            if (var > bm_n_vars) throw std::string("Too many bits");
            var++;
        } else {
            *result |= entry;
            entry = fullset;
            var = 0;
        }
    } while(c =='0' || c == '1' || c == '-' || c == '+' || c == 'F');
    if(inverse){
        ConfSet a = *result;
        *result = fullset;
        *result -= a;
    }
    return i;
}

void VPGame::dumpSet(ConfSet * dumpee, ConfSet t, char * p, int var) {
    if(var == bm_n_vars)
    {
        p[var]  = '\0';
        ConfSet result = *dumpee;
        result &= t;
        if(!(result == emptyset)){
            cout << p << ',';
        }
    } else {
        ConfSet t1 = t;
        t1 &= bm_vars[var];
        p[var] = '1';
        dumpSet(dumpee, t1, p , var + 1);
        p[var] = '0';
        ConfSet t2 = t;
        t2 -= bm_vars[var];
        dumpSet(dumpee, t2, p, var + 1);
    }
}

void VPGame::parseVertex(char *line) {
    while(*line == '\n' || *line == '\t' ||*line == ' ')
        line++;
    int index;
    int i;
    i = readUntil(line, ' ');
    index = atoi(line);
    if(declared[index]) throw std::string("Already declared vertex " + std::to_string(index));
    line += i + 1;
    i = readUntil(line, ' ');
    int p = atoi(line);
    priority[index] = p;
    if(p+1 > priorityI.size())
        priorityI.resize(p+1);
    priorityI[p].insert(index);
    line += i + 1;
    i = readUntil(line, ' ');
    owner[index] = atoi(line);
    line += i + 1;


    cout << "\nVertex with index: " << index << " and prio: " << priority[index] << " and owner: " << owner[index] << "\n";
    while(*line != '\0')
    {
        if(*line == ',')
            line++;
        int target;
        if(parsePG){
            i = readUntil(line, ',');
            target = atoi(line);
            line += i;
        } else {
            i = readUntil(line, '|');
            target = atoi(line);
            line += i + 1;
        }

        int guardindex = edge_guards.size();
        edge_guards.resize(guardindex + 1);
        edge_origins.resize(guardindex + 1);
        i = parseConfSet(line, 0,&edge_guards[guardindex]);
        edge_guards[guardindex] &= bigC;
        edge_origins[guardindex] = index;
        if(!(edge_guards[guardindex] == emptyset)){
            int outindex = out_edges[index].size();
            out_edges[index].resize(outindex + 1);
            out_edges[index][outindex] = std::make_tuple(target, guardindex);

            int inindex = in_edges[target].size();
            in_edges[target].resize(inindex+1);
            in_edges[target][inindex] = std::make_tuple(index, guardindex);
//            cout<< "with edge to " << target << " allowing: ";
//            dumpSet(&edge_guards[guardindex], fullset, new char[bm_n_vars+1], 0);
        }
        line += i-1;
    }
    declared[index] = true;
}

int VPGame::readUntil(const char * line, char delim){
    int i = 0;
    while(*(line + i) != delim && *(line + i) != '\0')
        i++;
    return i;
}

void VPGame::printCV(VertexSetZlnk *bigV, vector<ConfSet> *vc, ConfSet t, char * p, int var, bool fulloutput) {
    if(t == emptyset) return;
    if(var == bm_n_vars)
    {
        p[var]  = '\0';
        cout << "For product " << p << " the following vertices are in: ";
        if(fulloutput){
            for(int vi = 0;vi<n_nodes;vi++) {
                if(!(*bigV)[vi])
                    continue;
                ConfSet result = (*vc)[vi];
                result &= t;
                if(!((result) == emptyset)){
                    for(auto j : orgvertices[vi])
                        cout << j << ',';
                }
            }
        } else {
            int vi = findVertexWinningFor0();
            ConfSet result = (*vc)[vi];
            result &= t;
            if(!((result) == emptyset)){
                cout << 0 << ',';
            }
        }
        cout <<"\n";
        fflush(stdout);

    } else {
        ConfSet t1 = t;
        t1 &= bm_vars[var];
        p[var] = '1';
        printCV(bigV,vc, t1, p , var + 1, fulloutput);
        p[var] = '0';
        ConfSet t2 = t;
        t2 -= bm_vars[var];
        printCV(bigV, vc, t2, p, var + 1, fulloutput);
    }
}


void VPGame::printCV(VertexSetZlnk *bigV, vector<ConfSet> *vc, bool fulloutput) {
#ifdef SINGLEMODE
    cout << "The following vertices are in: ";
    if(fulloutput){
        for(int vi = 0;vi<n_nodes;vi++) {
            if(!(*bigV)[vi])
                continue;
            for (auto j : orgvertices[vi])
                cout << j << ',';
        }
    } else {
        if((*bigV)[findVertexWinningFor0()])
            cout << "0,";
    }
    cout << "\n";
    fflush(stdout);
#else
    printCV(bigV, vc, bigC, new char[bm_n_vars+1], 0, fulloutput);
#endif
}

void VPGame::compressPriorities() {
    int pp = -1;
    for(int i = 0;i<priorityI.size();i++){
        if(!priorityI[i].empty())
        {
            if(pp == -1){
                pp = i;
                continue;
            }
            if(i - pp == 1){
                pp = i;
                continue;
            }
            if((i - pp) % 2 == 1){
                // odd
                movePriorities(i, ++pp);
                cout << "Move prio from " << i << " to " << pp << "\n";
            } else {
                // even
                movePriorities(i, pp);
                cout << "Move prio from " << i << " to " << pp << "\n";
            }
        }
    }
    priorityI.resize(pp+1);
}

void VPGame::movePriorities(int from, int to) {
    for(auto &e : priorityI[from]){
        priority[e] = to;
    }
    if(priorityI[to].empty()){
        priorityI[to] = priorityI[from];
    } else {
        priorityI[to].insert(priorityI[from].begin(), priorityI[from].end());
    }
    priorityI[from].clear();
}

void VPGame::reindexVertices() {
    reindexPCutoff.resize(this->priorityI.size() + 2);

    int c = 0;
    reindexPCutoff[0] = c;
    int i;
    for(i = 0;i < this->priorityI.size();i+=2){
        for(auto& v : priorityI[i]){
            reindexedNew[v] = c;
            reindexedOrg[c++] = v;
        }
        reindexPCutoff[i+2] = c;
    }
    reindexPCutoff[1] = c;
    for(i = 1;i < this->priorityI.size();i+=2){
        for(auto& v : priorityI[i]){
            reindexedNew[v] = c;
            reindexedOrg[c++] = v;
        }
        reindexPCutoff[i+2] = c;
    }
    vector<int> priority_old = priority;
    vector<int> owner_old  = owner;
    auto *out_edges_old = new vector<std::tuple<int,int>>[n_nodes];
    auto *in_edges_old = new vector<std::tuple<int,int>>[n_nodes];
    std::copy(out_edges, out_edges+n_nodes, out_edges_old);
    std::copy(in_edges, in_edges+n_nodes, in_edges_old);
    for(i = 0 ;i<n_nodes;i++){
        int to = reindexedNew[i];
        owner[to] = owner_old[i];
        priority[to] = priority_old[i];
        out_edges[to] = out_edges_old[i];
        in_edges[to] = in_edges_old[i];

        for(auto & j : out_edges[to]){
            j = make_tuple(reindexedNew[target(j)], edge_index(j));
        }
        for(auto & j : in_edges[to]){
            j = make_tuple(reindexedNew[target(j)], edge_index(j));
        }
    }
    for(int & edge_origin : edge_origins){
        edge_origin = reindexedNew[edge_origin];
    }
//    VertexSetZlnk p_old;
//    for(auto & p : priorityI){
//        p_old = p;
//        p.clear();
//        for(const auto & pp : p_old)
//            p.insert(reindexedNew[pp]);
//    }
    delete[] out_edges_old;
    delete[] in_edges_old;
}

void VPGame::parsePGFromFile(const string &filename) {
    parsePG = true;
    parseVPGFromFile(filename);
}

void VPGame::writePG(ostream *output) {
    *output << "parity " << n_nodes << ';';
    for(int v = 0;v < n_nodes;v++){
        *output << endl << reindexedOrg[v] << ' ' << priority[v] << ' ' << owner[v];
        char seperator = ' ';
        for(const auto & e : out_edges[v]) {
            *output << seperator << reindexedOrg[target(e)];
            seperator = ',';
        }
        *output << ';';
    }
}

void VPGame::writePG(ostream *output, ConfSet conf) {
    *output << "parity " << n_nodes << ';';
    for(int v = 0;v < n_nodes;v++){
        *output << endl << reindexedOrg[v] << ' ' << priority[v] << ' ' << owner[v];
        char seperator = ' ';
        for(const auto & e : out_edges[v]) {
            ConfSet cc = conf;
            cc &= edge_guards[edge_index(e)];
            if(cc == emptyset)
                continue;
            *output << seperator << reindexedOrg[target(e)];
            seperator = ',';
        }
        *output << ';';
    }
}


void VPGame::compressVertices() {
    compressvertices = true;
    vector<bool> remove(n_nodes);
    for(int i = 0;i < n_nodes;i++){
        if(out_edges[i].size() ==  1) {
            int t = target(out_edges[i][0]);
            if (t != i && priority[t] >= priority[i]) {
                remove[i] = true;
                moveVertexInto(i, t, true);
                continue;
            }
        }
//        if(in_edges[i].size() == 1){
//            int t = target(in_edges[i][0]);
//            if(t != i && priority[t] >= priority[i] && owner[t] == owner[i]){
//                remove[i] = true;
//                moveVertexInto(i,t,false);
//            }
//        }
    }

//    vector<bool> remove(n_nodes);
    vector<int> reindex(n_nodes);
    for(int i = 0;i < n_nodes;i++)
        reindex[i] = i;
    int removed = 0;
    for(int i = 0;i < n_nodes;i++){
        if(remove[i])
            removed++;
        else
            reindex[i] -= removed;
    }
    for(int i = 0;i < n_nodes;i++){
        if(!remove[i]) {
            if(reindex[i] < i){
                owner[reindex[i]] = owner[i];
                priority[reindex[i]] = priority[i];
                out_edges[reindex[i]] = out_edges[i];
                in_edges[reindex[i]] = in_edges[i];
                orgvertices[reindex[i]] = orgvertices[i];
            } else if(reindex[i] > i)
                throw "Incorrect reindex";
        }
    }
    n_nodes -= removed;
    priority.resize(n_nodes);
    owner.resize(n_nodes);
    orgvertices.resize(n_nodes);
    for(int i = 0;i < n_nodes;i++){
        for(auto & e : out_edges[i])
            e = make_tuple(reindex[target(e)], edge_index(e));
        for(auto & e : in_edges[i])
            e = make_tuple(reindex[target(e)], edge_index(e));
    }

//    for(int i = 0;i < n_nodes;i++){
//        int t = reindex[target(out_edges[i][0])];
//        if(out_edges[i].size() ==  1 && t != i){
//            // assume we admit all configurations since VPGs are total
//            remove[i] = true;
//            reindex[i] = t;
//            if(priority[i] > priority[t]){
//                priorityI[priority[t]].erase(t);
//                priorityI[priority[i]].insert(t);
//                priority[t] = priority[i];
//                for(int j = 0;j < n_nodes;j++)
//                    if(reindex[j] == i)
//                        reindex[j] = t;
//            }
//            //todo remove edges
////            out_edges[t].reserve(out_edges[t].size() + out_edges[i].size());
////            out_edges[t].insert(out_edges[t].end(), out_edges[i].begin(), out_edges[i].end());
//        }
//    }

//    auto * out_edges_old = out_edges;
//    out_edges = new vector<std::tuple<int,int>>[n_nodes - removed];
//    vector<int> priority_old(n_nodes);
//    priority_old = priority;
//    vector<int> owner_old(n_nodes);
//    owner_old = owner;
//    for(int i = 0;i < n_nodes;i++){
//        if(!remove[i] && reindex[i] != i){
//            out_edges[reindex[i]] = out_edges_old[i];
//            priority[reindex[i]] = priority_old[i];
//            owner[reindex[i]] = owner_old[i];
//        }
//    }
//    delete[] out_edges_old;
//
//    orgvertices.resize(n_nodes - removed);
//    for(int i = 0;i < n_nodes;i++){
//        orgvertices[reindex[i]].insert(i);
//    }
//    n_nodes -= removed;
//    //todo vector instead of array of out_edges
////    delete[] (out_edges + n_nodes);
//
//
    for(auto &p : priorityI){
        unordered_set<int> pp = p;
        p.clear();
        for(const auto &v : pp)
            if (!remove[v])
                p.insert(reindex[v]);
    }
//    //todo remove this reset
//    for(int i = 0;i < n_nodes;i++)
//        in_edges[i].resize(0);
//    buildInEdges();
}

void VPGame::moveVertexInto(int v1, int v2, bool v1Ev2) {
    int orgindex = orgvertices[v2].size();
    orgvertices[v2].resize(orgindex + orgvertices[v1].size());
    for(int j = 0;j < orgvertices[v1].size();j++)
        orgvertices[v2][orgindex + j] = orgvertices[v1][j];

    vector<std::tuple<int,int>> * out_edges_local = out_edges;
    vector<std::tuple<int,int>> * in_edges_local = in_edges;
    if(!v1Ev2){
        out_edges_local = in_edges;
        in_edges_local = out_edges;
    }
    for(auto &e : in_edges_local[v1]){
        bool merge = false;
        tuple<int,int> edgeorg, edgenew;

        for(auto & oe: out_edges_local[target(e)]){
            if(target(oe) == v2) {
                merge = true;
                edgeorg = oe;
            }
            if(target(oe) == v1){
                oe = make_tuple(v2, edge_index(oe));
                edgenew = oe;
            }
        }
        if(merge) {
            cout << "Merge"<<endl;
            //todo remove this loop by storing the iterator in the previous loop
            auto ite = out_edges_local[target(e)].begin();
            while(ite != out_edges_local[target(e)].end()){
                if(*ite == edgenew){
                    out_edges_local[target(e)].erase(ite);
                    break;
                }
                ite++;
            }
            edge_guards[edge_index(edgeorg)] |= edge_guards[edge_index(edgenew)];
        }else{
            int index = in_edges_local[v2].size();
            in_edges_local[v2].resize(index + 1);
            in_edges_local[v2][index] = e;
        }
    }
    auto ite = in_edges_local[v2].begin();
    while(ite != in_edges_local[v2].end()){
        if(target(*ite) == v1){
            in_edges_local[v2].erase(ite);
            break;
        }
        ite++;
    }
    cout << "Remove " << v1 << " v1Ev2: " << v1Ev2 << endl;
}

void VPGame::buildInEdges() {
    for(int i = 0;i < n_nodes;i++){
        for(const auto & e : out_edges[i]){
            int t = target(e);
            int index = in_edges[t].size();
            in_edges[t].resize(index + 1);
            in_edges[t][index] = make_tuple(i, edge_index(e));
        }
    }
}

int VPGame::findVertexWinningForVertex(int v) {
    for(int i = 0;i< n_nodes;i++)
        for(int j = 0;j < orgvertices[i].size();j++)
            if(orgvertices[i][j] == v)
                return i;
    return -1;
}

int VPGame::findVertexWinningFor0() {
    if(winningfor0 == -1)
        winningfor0 = findVertexWinningForVertex(0);
    return winningfor0;
}

void VPGame::findAllElements(ConfSet s, vector<tuple<ConfSet, string>> *result) {
    auto p = new char[bm_n_vars+1];
    findAllElements(s, result, p, 0);
    delete[] p;
}

void VPGame::findAllElements(ConfSet s, vector<tuple<ConfSet, string>> *result, char *p, int var) {
    if(s == emptyset) return;
    if(var == bm_n_vars)
    {
        p[var] = '\0';
        result->push_back(make_tuple(s, string(p)));
    } else {
        ConfSet t1 = s;
        t1 &= bm_vars[var];
        p[var] = '1';
        findAllElements(t1, result, p, var + 1);
        p[var] = '0';
        ConfSet t2 = s;
        t2 -= bm_vars[var];
        findAllElements(t2, result, p, var + 1);
    }
}

