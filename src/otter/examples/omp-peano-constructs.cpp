/* 
    This file provided by Holger Schultz 04/05/21

*/

#include <omp.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#define StandardPriority 16
#define BackgroundConsumerPriority 1
#define NAP 1000
#define SLEEP 10000

int nonblockingTasks;
unsigned int completedNonblockingTasks;
enum taskType {small, big};

void spawnTask(unsigned int M, taskType type) {
  unsigned int j=0;
  switch(type) {
    case small:
      for (j=0; j<M; j++) {
        #pragma omp atomic
        nonblockingTasks--;
        #pragma omp task priority(BackgroundConsumerPriority) //Low priority
        {
          usleep(NAP);
          #pragma omp taskyield 
        }
        #pragma omp atomic
        completedNonblockingTasks++;
      }
    break;

    case big:
      usleep(SLEEP);
    break;
  }
}

void spawnAndWait(unsigned int N, unsigned int M, taskType type){
  unsigned int i=0;
  #pragma omp taskloop nogroup priority(StandardPriority) //High priority
  for (i=0; i<N; i++) {
    spawnTask(M, type);
  }
  #pragma omp taskwait
}

void usage(int argc, char* argv[]) {
  if (argc < 3) {
    printf("USAGE: %s N M\n", argv[0]);
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char* argv[]) {
  usage(argc, argv);
  unsigned int N = std::stoul(argv[1]);
  unsigned int M = std::stoul(argv[2]);
  nonblockingTasks = N*M; completedNonblockingTasks = 0;
  printf("N and M: %d and %d\n", N, M);
  
  #pragma omp parallel
  #pragma omp single
  {
  //Traversal tasks (creation of tiny tasks)
  spawnAndWait(N, M, taskType(small));
  // How many tiny tasks are left ?
  printf("Completed tiny tasks: %d\nRemaining tiny tasks: %d\n", completedNonblockingTasks, nonblockingTasks);
  usleep(1000); //Emulates the serial code between the 1st and 2nd batch of tasks
  //Big tasks
  spawnAndWait(N, M, taskType(big));   
  }
}
