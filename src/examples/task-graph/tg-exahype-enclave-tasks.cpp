#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "api/otter-task-graph/otter-task-graph-wrapper.hpp"

using namespace otter;

void step(int n, int n_traversal_tasks);
void traverse_and_spawn_enclave(int num_enclave_tasks);
void traverse_and_wait_for_enclave();

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "usage: %s steps\n", argv[0]);
        return 1;
    }

    int steps = atoi(argv[1]);

    auto &trace = Otter::get_otter();
    auto &root = trace.get_root_task();

    for (int i=0; i<steps; i++) {
        step(i, 4);
    }

    return 0;
}

void step(int n, int n_traversal_tasks) {

    fprintf(stderr, "TIMESTEP: %2d\n", n);

    auto &root = Otter::get_otter().get_root_task();
    auto timestep_task = root.make_child();

    for (int j=0; j<n_traversal_tasks; j++) {
        auto traversal_task = timestep_task.make_child(3);
        traverse_and_spawn_enclave(10);
        traversal_task.end_task();
    }

    // the first group of traversal tasks must come before the second group
    timestep_task.synchronise_tasks(otter_sync_children);

    // serial part
    usleep(50000);

    for (int j=0; j<n_traversal_tasks; j++) {
        auto traversal_task = timestep_task.make_child(3);
        traverse_and_wait_for_enclave();
        traversal_task.end_task();
    }
    timestep_task.synchronise_tasks(otter_sync_children);
    timestep_task.end_task();
}

void traverse_and_spawn_enclave(int num_enclave_tasks) {
    for (int t=0; t<num_enclave_tasks; t++) {
        // spawn an enclave task as a child of the root task
        auto enclave = Otter::get_otter().get_root_task().make_child(1);
    }
}

void traverse_and_wait_for_enclave() {
    Otter::get_otter().get_root_task().synchronise_tasks(otter_sync_children);
}
