#include "schedule_algo.h"
#include <fstream>

void read_file(string in_file_path, vector<vector<int>> &graph){
    ifstream in_file(in_file_path);
    int shape_h, shape_w;
    in_file >> shape_h >> shape_w;
    graph.resize(shape_h);
    for(int i = 0; i < shape_h; i++){
        graph[i].resize(shape_w);
        for(int j = 0; j < shape_w; j++){
            in_file >> graph[i][j];
        }
    }
}

void gen_job_table(vector<vector<int>> &graph, vector<vector<int>> &job_list){
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
    int size;
    unit_mesh_t mesh;

    vector<vector<int>> job_list;
    vector<vector<int>> graph;
    read_file("./input/web-Google.txt", graph);
    size = int(sqrt(graph.size()));
    gen_job_table(graph, job_list);

    schedule_table table;
    gen_baseline(job_list, table);
    // gen_naive_greedy(job_list, table);

    config_mesh(job_list, mesh);
    run(mesh, table);   
}