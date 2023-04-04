#include "compute_unit.h"

void gen_job_table(vector<vector<int>> graph, vector<vector<int>> &job_list){
    int unit_num = graph.size();
    job_list.resize(unit_num);
    for(int graph_id = 0; graph_id < unit_num; graph_id++){
        for(int unit_id = 0; unit_id < unit_num; unit_id++){
            if(graph[graph_id][unit_id]){
                job_list[unit_id].push_back(graph_id);
            }
        }
    }
}

void config_mesh(vector<vector<int>> &job_list, unit_mesh_t &mesh){
    int size = int(sqrt(job_list.size()));
    mesh.set_size(size);
    for(int i = 0; i < size * size; i++){
        mesh.mesh[i].init_jobs(job_list[i]);
    }
}

bool run(unit_mesh_t &mesh, schedule_table table){
    mesh.init();
    mesh.print_status();
    for(auto item: table){
        mesh.communicate(item);
        mesh.update();
        if(!mesh.check()){
            printf("Error\n");
            return false;
        }
        mesh.print_status();
    }
    for(int i = 0; i < mesh.size * mesh.size; i++){
        mesh.mesh[i].print_log();
    }
    return true;
}

int main(){
    int size = 2;
    unit_mesh_t mesh;

    vector<vector<int>> job_list(size * size);
    job_list[0].push_back(0); job_list[0].push_back(1); job_list[0].push_back(3);
    job_list[1].push_back(1); job_list[1].push_back(3);
    job_list[2].push_back(1); job_list[2].push_back(2);
    job_list[3].push_back(2); job_list[3].push_back(3);

    schedule_table table;
    schedule_item_t item(size * size);
    item[0] = direction::right;
    item[1] = direction::right;
    item[2] = direction::right;
    item[3] = direction::right;
    table.push_back(item);

    item[0] = direction::down;
    item[1] = direction::down;
    item[2] = direction::down;
    item[3] = direction::down;
    table.push_back(item);

    item[0] = direction::stall;
    item[1] = direction::right;
    item[2] = direction::right;
    item[3] = direction::right;
    table.push_back(item);

    config_mesh(job_list, mesh);
    run(mesh, table);   
}