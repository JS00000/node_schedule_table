#include <cstdlib>
#include <cstdio>
#include <string.h>
#include <string>
#include <math.h>
#include <vector>
using namespace std;

enum direction{
    stall = 0,
    left = 1,
    right = 2,
    up = 3,
    down = 4
};

typedef vector<direction> schedule_item_t;
typedef vector<schedule_item_t> schedule_table;

// struct schedule_item_t{
//     int size;
//     vector<direction> actions;
//     schedule_item_t(int _size){size = _size; actions.resize(size * size);}
// };

struct unit_t{
    int cur_job;
    int next_job;
    int unit_id;
    int connections[5];
    vector<int> jobs;
    vector<int> visited;
    
    // set the unit
    void init(int job_id){cur_job = job_id; visited.push_back(cur_job);}
    void init_unit_id(int _unit_id){unit_id = _unit_id;}
    void init_connections(int left_unit_id, int right_unit_id, int up_unit_id, int down_unit_id){
        connections[direction::stall] = unit_id;
        connections[direction::left] = left_unit_id;
        connections[direction::right] = right_unit_id;
        connections[direction::up] = up_unit_id;
        connections[direction::down] = down_unit_id;
    }
    void init_jobs(int job_id){jobs.push_back(job_id);}
    void init_jobs(vector<int> job_list){jobs.insert(jobs.end(), job_list.begin(), job_list.end());}

    // set job
    void set_job(int job_id){next_job = job_id;}
    int get_job(){return cur_job;}
    // update
    void update(){
        if(cur_job != next_job){
            cur_job = next_job;
            visited.push_back(cur_job);
        }
    }
    bool is_job(int job_id);
    bool is_finished();

    // print_log
    void print_log();
};

struct unit_mesh_t{
    int size;
    int cycles;
    vector<unit_t> mesh;

    // set the configuration
    void set_size(int _size);

    void init();

    // run
    void communicate(schedule_item_t item);
    void update();
    bool check();
    void print_status();
};