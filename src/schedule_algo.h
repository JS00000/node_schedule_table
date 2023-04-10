#include "compute_unit.h"

bool gen_baseline(vector<vector<int>> &job_list, schedule_table &table);
bool gen_naive_greedy(vector<vector<int>> &job_list, schedule_table &table);

bool in_vector(int target, vector<int> vec);
bool is_row_finished(unit_mesh_t &mesh);