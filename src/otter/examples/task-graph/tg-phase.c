#include <stdio.h>
#include <otter/otter-task-graph.h>

int main(int argc, char* argv[]) {

    otterTraceInitialise();

    otter_task_context *root = otterTaskBegin(OTTER_SRC_ARGS(), NULL);

    otterPhaseBegin("test-phase");

    otterPhaseSwitch("other-phase");

    otterPhaseEnd();

    otterTaskEnd(root);


    otterTraceFinalise();

    return 0;
}
