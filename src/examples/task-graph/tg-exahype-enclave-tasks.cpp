#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "api/otter-task-graph/otter-task-graph-wrapper.hpp"

using namespace otter;

void step(int n_traversal_tasks, int n_enclave_tasks);

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "usage: %s steps\n", argv[0]);
        return 1;
    }

    int steps = atoi(argv[1]);

    auto &trace = Otter::get_otter();
    auto &root = trace.get_root_task();

    for (int i=0; i<steps; i++) {
        step(4, 10);
    }

    return 0;
}

void step(int traversal_tasks, int enclave_tasks_per_traversal_task) {

    auto &root = Otter::get_otter().get_root_task();
    auto timestep_task = root.make_child();

    // traverse and spawn enclave tasks
    for (int j=0; j<traversal_tasks; j++) {
        auto traversal_task = timestep_task.make_child(3);
        for (int t=0; t<enclave_tasks_per_traversal_task; t++) {
            auto enclave = Otter::get_otter().get_root_task().make_child(1);
        }
        traversal_task.end_task();
    }

    // the first group of traversal tasks must come before the second group
    timestep_task.synchronise_tasks(otter_sync_children);

    // serial part
    usleep(50000);

    // traverse again and wait for enclave tasks
    for (int j=0; j<traversal_tasks; j++) {
        auto traversal_task = timestep_task.make_child(3);
        Otter::get_otter().get_root_task().synchronise_tasks(otter_sync_children);
        traversal_task.end_task();
    }
    timestep_task.synchronise_tasks(otter_sync_children);
    timestep_task.end_task();
}
