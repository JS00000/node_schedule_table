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

bool gen_baseline(vector<vector<int>> &job_list, schedule_table &table){
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

bool gen_naive_greedy(vector<vector<int>> &job_list, schedule_table &table){
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