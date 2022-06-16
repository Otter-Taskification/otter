#include <stdio.h>
#include <otter/otter-serial.h>

int main(int argc, char *argv[]) {

    otterTraceInitialise();
    otterParallelBegin();
    otterTaskBegin();
    otterTaskEnd();
    otterParallelEnd();
    otterTraceFinalise();

    return 0;

}
