#include <stdio.h>
#include <otter/otter-task-graph.h>

static const int NUM_CHILDREN = 5;

int main(int argc, char *argv[])
{
    otterTraceInitialise();

    otter_task_context *root = otterTaskBegin(OTTER_SRC_ARGS(), NULL);

    otter_task_context *child = NULL;
    for (int k=0; k<NUM_CHILDREN; k++)
    {
        child = otterTaskBegin(OTTER_SRC_ARGS(), root);
        otterTaskEnd(child);
    }
    
    otterTaskEnd(root);

    otterTraceFinalise();
    fprintf(stderr, "Done.\n");
    return 0;
}
