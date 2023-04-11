#include "schedule_algo.h"
#include <assert.h>
#include <iostream>

bool in_vector(int target, vector<int> vec){
    for(auto value: vec){
        if(target == value){
            return true;
        }
    }
    return false;
}

bool is_row_finished(unit_mesh_t &mesh){
    int size = mesh.size;
    for(int row = 0; row < size; row++){
        int row_id_begin = (mesh.mesh[row * size + 0].cur_job / size) * size;
        int row_id_end   = row_id_begin + size;
        for(int col = 0; col < size; col++){
            assert(mesh.mesh[row * size + col].cur_job <  row_id_end);
            assert(mesh.mesh[row * size + col].cur_job >= row_id_begin);
            for(int value = row_id_begin; value < row_id_end; value++){
                if(in_vector(value, mesh.mesh[row * size + col].jobs) && !in_vector(value, mesh.mesh[row * size + col].visited)){
                    return false;
                }
            }
        }
    }
    return true;
}

bool gen_baseline(vector<vector<int>> &job_list, schedule_table &table, bool verbose){
    int unit_num = job_list.size();
    int size = int(sqrt(unit_num));
    schedule_item_t item;
    for(int sub_iter_id = 0; sub_iter_id < size - 1; sub_iter_id++){
        for(int round_id = 0; round_id < size - 1; round_id++){
            item.resize(0);
            for(int unit_id = 0; unit_id < unit_num; unit_id++){
                item.push_back(direction::right);
            }
            table.push_back(item);
        }
        item.resize(0);
        for(int unit_id = 0; unit_id < unit_num; unit_id++){
            item.push_back(direction::down);
        }
        table.push_back(item);
    }
    item.resize(0);
    for(int round_id = 0; round_id < size - 1; round_id++){
        for(int unit_id = 0; unit_id < unit_num; unit_id++){
            item.push_back(direction::right);
        }
        table.push_back(item);
    }
    return true;
}

bool gen_naive_greedy(vector<vector<int>> &job_list, schedule_table &table, bool verbose){
    int unit_num = job_list.size();
    int size = int(sqrt(unit_num));
    unit_mesh_t mesh;
    schedule_item_t item(0);

    config_mesh(job_list, mesh);

    mesh.init();
    // mesh.print_status();

    vector<pair<schedule_item_t, int>> candidates;
    candidates.resize(2 + size);
    for(auto &candidate: candidates){
        candidate.first.resize(size);
    }

    while(!mesh.is_finished()){
        item.resize(0);
        for(int row = 0; row < size; row++){
            candidates[0].second = 0;
            for(int col = 0; col < size; col++){
                int cur_job = mesh.mesh[row * size + col].cur_job;
                int dst_unit = mesh.mesh[row * size + col].connections[direction::right];
                if(in_vector(cur_job, mesh.mesh[dst_unit].jobs) && !in_vector(cur_job, mesh.mesh[dst_unit].visited)){
                    candidates[0].second++;
                }
                candidates[0].first[col] = direction::right;
            }
            candidates[1].second = 0;
            for(int col = 0; col < size; col++){
                int cur_job = mesh.mesh[row * size + col].cur_job;
                int dst_unit = mesh.mesh[row * size + col].connections[direction::left];
                if(in_vector(cur_job, mesh.mesh[dst_unit].jobs) && !in_vector(cur_job, mesh.mesh[dst_unit].visited)){
                    candidates[1].second++;
                }
                candidates[1].first[col] = direction::left;
            }
            for(int i = 0; i < size; i++){
                // must exchange i and i + 1
                candidates[2 + i].second = 0;
                int cur_job = mesh.mesh[row * size + i].cur_job;
                int dst_unit = mesh.mesh[row * size + i].connections[direction::right];
                if(in_vector(cur_job, mesh.mesh[dst_unit].jobs) && !in_vector(cur_job, mesh.mesh[dst_unit].visited)){
                    candidates[2 + i].second++;
                }
                int dst_job = mesh.mesh[dst_unit].cur_job;
                if(in_vector(dst_job, mesh.mesh[row * size + i].jobs) && !in_vector(dst_job, mesh.mesh[row * size + i].visited)){
                    candidates[2 + i].second++;
                }
                candidates[2 + i].first[i] = direction::right;
                candidates[2 + i].first[(i + 1) % size] = direction::left;

                // rest
                for(int col = (i + 2) % size; col != i && col != (i + 1) % size && ((col + 1) % size) != i && ((col + 1) % size) != (i + 1) % size; col = (col + 2) % size){
                    cur_job = mesh.mesh[row * size + col].cur_job;
                    dst_unit = mesh.mesh[row * size + col].connections[direction::right];
                    dst_job = mesh.mesh[dst_unit].cur_job;
                    bool gain0 = in_vector(cur_job, mesh.mesh[dst_unit].jobs) && !in_vector(cur_job, mesh.mesh[dst_unit].visited);
                    bool gain1 = in_vector(dst_job, mesh.mesh[row * size +col].jobs) && !in_vector(dst_job, mesh.mesh[row * size + col].visited);
                    if(gain0 + gain1){
                        candidates[2 + i].second += gain0;
                        candidates[2 + i].second += gain1;
                        candidates[2 + i].first[col] = direction::right;
                        candidates[2 + i].first[(col + 1) % size] = direction::left;
                    }
                    else{
                        candidates[2 + i].first[col] = direction::stall;
                        candidates[2 + i].first[(col + 1) % size] = direction::stall;
                    }
                }
            }
            // choose the best candidate
            int best_gain = -1;
            int best_op;
            for(int i = 0; i < 2 + size; i++){
                if(candidates[i].second > best_gain){
                    best_gain = candidates[i].second;
                    best_op = i;
                }
            }
            item.insert(item.end(), candidates[best_op].first.begin(), candidates[best_op].first.end());
        }
        table.push_back(item);
        mesh.communicate(item);
        mesh.update();
        if(!mesh.check()){
            printf("Algorithm Error\n");
            return false;
        }
        // mesh.print_status();
        if(is_row_finished(mesh) && !mesh.is_finished()){
            item.resize(0);
            for(int i = 0; i < unit_num; i++){
                item.push_back(direction::down);
            }
            table.push_back(item);
            mesh.communicate(item);
            mesh.update();
            if(!mesh.check()){
                printf("Algorithm Error\n");
                return false;
            }
            // mesh.print_status();
        }
    }
    return true;
}

#include <set>
#include "min_cost_flow.h"
class mesh_weighted_matching: public min_cost_flow 
{
public:

    int n;
    int gn;
    bool verbose;
    mesh_weighted_matching(int _n, bool _verbose): min_cost_flow() {
        n = _n;
        gn = int(sqrt(n));
        verbose = _verbose;
    }

    int get_next_id(int id, int dir)
    {
        // id: [0, n)
        // dir: 
        //   0 stall
        //   1 left
        //   2 right
        //   3 up
        //   4 down
        // shape like:
        //  0  1  2  3 
        //  4  5  6  7 
        //  8  9 10 11 
        // 12 13 14 15 
        if (dir == 0) return id;
        else if (dir == 1) return ((id % gn == 0) ? id + gn - 1: id - 1);
        else if (dir == 2) return ((id % gn == gn-1) ? id - gn + 1: id + 1);
        else if (dir == 3) return ((id < gn) ? id + gn * (gn - 1): id - gn);
        else if (dir == 4) return ((id >= gn * (gn - 1)) ? id - gn * (gn - 1): id + gn);
        return -1;
    }

    int dist(int x1, int y1, int x2, int y2) {
        int dx = abs(x1 - x2);
        int dy = abs(y1 - y2);
        dx = min(dx, gn - dx);
        dy = min(dy, gn - dy);
        return dx + dy;
    }

    int get_shortest_dist(int id, int target) {
        int x1 = id / gn;
        int y1 = id % gn;
        int x2 = target / gn;
        int y2 = target % gn;
        return dist(x1, y1, x2, y2);
    }

    int get_delta(int x1, int x2) {
        int dx = x2 - x1;
        if (abs(dx - gn) < abs(dx))
            dx = dx - gn;
        else if (abs(dx + gn) < abs(dx))
            dx = dx + gn;
        return dx;
    }

    int get_dir(int id, int target) {
        if (id == target) 
            return 0;
        int x1 = id / gn;
        int y1 = id % gn;
        int x2 = target / gn;
        int y2 = target % gn;
        int dx = get_delta(x1, x2);
        int dy = get_delta(y1, y2);
        int adx = abs(dx);
        int ady = abs(dy);
        if (adx < ady) {
            // left or right
            if (dy < 0) return 1;   // left
            else return 2;          // right
        } else if (adx > ady) {
            // up or down
            if (dx < 0) return 3;   // up
            else return 4;          // down
        } else {
            // adx = ady
            if (dx < 0 && dy < 0) return 5;         //up or left
            else if (dx < 0 && dy > 0) return 6;    //up or right
            else if (dx > 0 && dy < 0) return 7;    //down or left
            else if (dx > 0 && dy > 0) return 8;    //down or right
        }
        // must not reach here
        return -1;
    }

    // opposite direction
    int op(int dir) {
        if (dir == 0) return 0;
        return (dir - 1) ^ 1 + 1;
    }

    void build_graph(std::vector< std::set<int> > &graph_unvisited, std::vector<int> &job_now)
    {

        int graph_locate[n];
        for (int i = 0; i < n; i++) {
            graph_locate[job_now[i]] = i;
        }

        // compute cost between nodes
        int cost[n][5];
        // for each graph data i
        for (int i = 0; i < n; i++) {
            int id = graph_locate[i];
            if (graph_unvisited[i].size() == 0) {
                // free to move
                cost[id][0] = 0;
                cost[id][1] = 0;
                cost[id][2] = 0;
                cost[id][3] = 0;
                cost[id][4] = 0;
            } else {
                // set default
                cost[id][0] = 10000;
                cost[id][1] = 10000;
                cost[id][2] = 10000;
                cost[id][3] = 10000;
                cost[id][4] = 10000;
                // deal adjacent node
                bool is_adj = false;
                for (int dir = 1; dir <= 4; dir++) {
                    int id_adj = get_next_id(id, dir);
                    if ( graph_unvisited[i].count(id_adj) ) {
                        is_adj = true;
                        cost[id][dir] = n - graph_unvisited[i].size();
                    }
                }
                
                // otherwise go to nearest unvisited node
                if (!is_adj) {
                    int nearest_id;
                    int nearest_dist = infi;
                    for (auto it: graph_unvisited[i]) {
                        int dist = get_shortest_dist(id, it);
                        if (dist < nearest_dist) {
                            nearest_dist = dist;
                            nearest_id = it;
                        }
                    }
                    int dir = get_dir(id, nearest_id);
                    if (dir <= 4) {
                        cost[id][dir] = n*n - get_shortest_dist(id, nearest_id);
                        cost[id][op(dir)] = 100000;
                    } else {
                        int cc = n*n - get_shortest_dist(id, nearest_id);
                        if (dir == 5) {         //up or left
                            cost[id][1] = cc;
                            cost[id][2] = 100000;
                            cost[id][3] = cc;
                            cost[id][4] = 100000;
                        } else if (dir == 6) {  //up or right
                            cost[id][1] = 100000;
                            cost[id][2] = cc;
                            cost[id][3] = cc;
                            cost[id][4] = 100000;
                        } else if (dir == 7) {  //down or left
                            cost[id][1] = cc;
                            cost[id][2] = 100000;
                            cost[id][3] = 100000;
                            cost[id][4] = cc;
                        } else if (dir == 8) {  //down or right
                            cost[id][1] = 100000;
                            cost[id][2] = cc;
                            cost[id][3] = 100000;
                            cost[id][4] = cc;
                        }
                    }
                }

                // otherwise go to the unvisited node with the minimum id
                // if (!is_adj) {
                //     int mini_id = infi;
                //     for (auto it: graph_unvisited[i]) {
                //         mini_id = std::min(mini_id, it);
                //     }
                //     int dir = get_dir(id, mini_id);
                //     if (dir <= 4) {
                //         cost[id][dir] = n*n - get_shortest_dist(id, mini_id);
                //         cost[id][op(dir)] = 100000;
                //     } else {
                //         int cc = n*n - get_shortest_dist(id, mini_id);
                //         if (dir == 5) {         //up or left
                //             cost[id][1] = cc;
                //             cost[id][2] = 100000;
                //             cost[id][3] = cc;
                //             cost[id][4] = 100000;
                //         } else if (dir == 6) {  //up or right
                //             cost[id][1] = 100000;
                //             cost[id][2] = cc;
                //             cost[id][3] = cc;
                //             cost[id][4] = 100000;
                //         } else if (dir == 7) {  //down or left
                //             cost[id][1] = cc;
                //             cost[id][2] = 100000;
                //             cost[id][3] = 100000;
                //             cost[id][4] = cc;
                //         } else if (dir == 8) {  //down or right
                //             cost[id][1] = 100000;
                //             cost[id][2] = cc;
                //             cost[id][3] = 100000;
                //             cost[id][4] = cc;
                //         }
                //     }
                // }
            }
        }


        // debug
        if (verbose) {
            printf("cost map:\n");
            for (int i = 0; i < gn; i++) {
                // up
                for (int j = 0; j < gn; j++) {
                    int id = i * gn + j;
                    printf("[%2d]'%2d' %6d        |", id, job_now[id], cost[id][3]);
                }
                printf("\n");
                // left, stall, right
                for (int j = 0; j < gn; j++) {
                    int id = i * gn + j;
                    printf("  %6d %6d %6d |", cost[id][1], cost[id][0], cost[id][2]);
                }
                printf("\n");
                // down
                for (int j = 0; j < gn; j++) {
                    int id = i * gn + j;
                    printf("         %6d        |", cost[id][4]);
                }
                printf("\n------------------------------------------------------------------------------------------------\n");
            }
        }


        // node now:  [0, n)
        // node next: [n, 2n)
        // source: 2n
        // sink: 2n+1
        memset(head, -1, sizeof(head));
        edge_idx = 0;
        for (int i = 0; i < n; i++) {
            // source to node now
            add_edge(2*n,i,1,0);
            // node next to sink
            add_edge(n+i,2*n+1,1,0);
        }
        // edges between nodes
        for (int i = 0; i < n; i++) {
            add_edge(i,n+get_next_id(i, 0),1,cost[i][0]);
            add_edge(i,n+get_next_id(i, 1),1,cost[i][1]);
            add_edge(i,n+get_next_id(i, 2),1,cost[i][2]);
            add_edge(i,n+get_next_id(i, 3),1,cost[i][3]);
            add_edge(i,n+get_next_id(i, 4),1,cost[i][4]);
        }
    }

    bool solve(schedule_item_t &item)
    {
        int flow = run(2*n, 2*n+1, 2*n+2);
        if (flow != n) 
            return false;
        for (int i = 0; i < n; i++) {
            for (int k = head[i]; k != -1; k = edges[k].next) {
                if (edges[k].c == 0 && edges[k].to != 2*n) {
                    if (edges[k].to == n+get_next_id(i, 0)) item.push_back(direction::stall);
                    else if (edges[k].to == n+get_next_id(i, 1)) item.push_back(direction::left);
                    else if (edges[k].to == n+get_next_id(i, 2)) item.push_back(direction::right);
                    else if (edges[k].to == n+get_next_id(i, 3)) item.push_back(direction::up);
                    else if (edges[k].to == n+get_next_id(i, 4)) item.push_back(direction::down);
                    break;
                }
            }
        }
        return true;
    }
};

bool gen_weighted_matching(std::vector<std::vector<int>> &job_list, schedule_table &table, bool verbose){
    int n = job_list.size();
    // job_list[i][j]: node i need to access graph data j 
    // graph_unvisited[i][j]: graph data i need to pass node j 
    std::vector< std::set<int> > graph_unvisited;
    graph_unvisited.resize(n);
    for (int i = 0; i < n; i++) {
        for (auto job: job_list[i]) {
            graph_unvisited[job].insert(i);
        }
    }

    mesh_weighted_matching wm(n, verbose);
    unit_mesh_t mesh;
    config_mesh(job_list, mesh);
    mesh.init();

    while(!mesh.is_finished()){

        std::vector<int> job_now;
        for (int i = 0; i < n; i++) {
            int jn = mesh.mesh[i].get_job();
            job_now.push_back(jn);
            graph_unvisited[jn].erase(i);
        }

        if (verbose) {
            mesh.print_status();
            printf("unvisited: \n");
            for (int i = 0; i < n; i++) {
                printf("'%2d': ", i);
                for (auto u: graph_unvisited[i]) {
                    printf("[%2d] ", u);
                }
                printf("\n");
            }
        }


        if (mesh.cycles >= n*n) {
            printf("Algorithm may stuck in an endless loop\n");
            return false;
        }

        wm.build_graph(graph_unvisited, job_now);
        schedule_item_t item(0);
        if (!wm.solve(item)) {
            printf("weighted_matching solve error\n");
            return false;
        }
        mesh.communicate(item);
        mesh.update();
        if(!mesh.check()){
            printf("Algorithm Error\n");
            return false;
        }
        table.push_back(item);
    }
    if (verbose) 
        mesh.print_status();
    return true;
}
