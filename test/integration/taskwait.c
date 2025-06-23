#include <stdio.h>
#include <unistd.h>
#include <omp.h>

#include "api/otter-task-graph/otter-task-graph-user.h"

void leaf_task(void) {
    printf("Thread %d executing leaf task\n", omp_get_thread_num());
    usleep(5000);
}

void main_task(void) {
    OTTER_DEFINE_TASK(leaf_1, otterGetActiveTask(), otter_add_to_pool, "leaf");
    #pragma omp task
    {
        OTTER_TASK_START(leaf_1);
        leaf_task();
        OTTER_TASK_END();
    }

    OTTER_DEFINE_TASK(leaf_2, otterGetActiveTask(), otter_add_to_pool, "leaf");
    #pragma omp task
    {
        OTTER_TASK_START(leaf_2);
        leaf_task();
        OTTER_TASK_END();
    }

    OTTER_DEFINE_TASK(leaf_3, otterGetActiveTask(), otter_add_to_pool, "leaf");
    #pragma omp task
    {
        OTTER_TASK_START(leaf_3);
        leaf_task();
        OTTER_TASK_END();
    }

    OTTER_DEFINE_TASK(leaf_4, otterGetActiveTask(), otter_add_to_pool, "leaf");
    #pragma omp task
    {
        OTTER_TASK_START(leaf_4);
        leaf_task();
        OTTER_TASK_END();
    }

    OTTER_DEFINE_TASK(leaf_5, otterGetActiveTask(), otter_add_to_pool, "leaf");
    #pragma omp task
    {
        OTTER_TASK_START(leaf_5);
        leaf_task();
        OTTER_TASK_END();
    }

    OTTER_TASK_WAIT_START(children);
    #pragma omp taskwait
    OTTER_TASK_WAIT_END();
}

int main(void) {
    OTTER_INITIALISE();
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("Number of threads: %d\n", omp_get_num_threads());
        }
        printf("Thread %d executing main task\n", omp_get_thread_num());
        #pragma omp single
        {
            OTTER_DEFINE_TASK(strassen, otterGetActiveTask(), otter_no_add_to_pool, "main");
            #pragma omp task
            {
                OTTER_TASK_START(strassen);
                main_task();
                OTTER_TASK_END();
            }
        }
        OTTER_TASK_WAIT_START(children);
        #pragma omp taskwait
        OTTER_TASK_WAIT_END();
    }
    OTTER_FINALISE();
}
