#include "compute_unit.h"

bool unit_t::is_job(int job_id){
    for(auto job: jobs){
        if(job == job_id){return true;}
    }
    return false;
}

bool unit_t::is_finished(){
    bool found = false;
    for(auto job: jobs){
        for(auto done_job: visited){
            if(job == done_job){
                found = true;
                break;
            }
        }
        if(found == false){
            return false;
        }
        found = false;
    }
    return true;
}

void unit_t::print_log(){
    printf("@@================%2d=================@@\n", unit_id);
    if(is_finished()){printf("job is finished\n");}
    else{printf("job is not finished\n");}
    double utilization = jobs.size() / double(visited.size());
    printf("The utilization is %lf\n", utilization);
    printf("@@===================================@@\n");
}

void unit_mesh_t::set_size(int _size){
    size = _size;
    mesh.resize(size * size);
    // set connection
    for(int i = 0; i < size * size; i++){
        mesh[i].init_unit_id(i);
        mesh[i].init_connections(
            (i / size) * size + (i - 1) % size,
            (i / size) * size + (i + 1) % size,
            (i - size + size * size) % (size * size),
            (i + size + size * size) % (size * size)
        );
    }
}

void unit_mesh_t::init(){
    for(int i = 0; i < size * size; i++){
        mesh[i].init(i);
    }
}

void unit_mesh_t::communicate(schedule_item_t item){
    for(int i = 0; i < size * size; i++){
        int dst_unit_id = mesh[i].connections[item[i]];
        int sub_graph_id = mesh[i].get_job();
        mesh[dst_unit_id].set_job(sub_graph_id);
    }
}

void unit_mesh_t::update(){
    for(int i = 0; i < size * size; i++){
        mesh[i].update();
    }
}

bool unit_mesh_t::check(){
    vector<bool> graph_id(size * size);
    for(int i = 0; i < size * size; i++){
        int sub_graph_id = mesh[i].get_job();
        if(graph_id[sub_graph_id] == false){
            graph_id[sub_graph_id] = true;
        }
        else{
            return false;
        }
    }
    return true;
}

void unit_mesh_t::print_status(){
    printf("@@======================================@@\n");
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            printf("%2d ", mesh[i * size + j].get_job());
        }
        printf("\n");
    }
}