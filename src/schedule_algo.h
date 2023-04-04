#include "compute_unit.h"

void gen_baseline(vector<vector<int>> &job_list, schedule_table &table){
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
}