#include <stdio.h>
#include "api/otter-task-graph/otter-task-graph-wrapper.hpp"

using namespace otter;

static const int NUM_CHILDREN = 10;

int main(int argc, char *argv[])
{
    int flavours[] = {1, 2, 3};
    printf("sizeof(flavours)=%lu\n", sizeof(flavours)/sizeof(int));
    auto& trace = Otter::get_otter();
    auto& root = trace.get_root_task();
    for (int i=0; i<NUM_CHILDREN; i++) {
        int flavour = flavours[i%3];
        printf("flavour=%d\n", flavour);
        auto child = root.make_child(flavour);
    }
    root.synchronise_tasks(otter_sync_children);
    return 0;
}
