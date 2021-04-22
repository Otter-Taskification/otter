#include <stdio.h>
#include <omp.h>

#define LOOPS 30
#define CHILDREN 2
#define LEVELS 4

int spawnTasks(int j);
int doMaths(void);

int sum=0;
int tasks=0;

int doMaths(void)
{
	int p = 2, m = 0;
	for (m=0; m<LOOPS; m++)
		p += (m*p + m);
	return p;
}

int spawnTasks(int j)
{
	int k=0, result=0;
	if (j==0) {
		for (k=0; k<=j; k++) {
			#pragma omp task depend(inout: sum)
			{
				result = doMaths();
				#pragma omp critical
				{
					sum += result;
					tasks++;
				}
			}
		}
	} else {
		for (k=0; k<CHILDREN; k++) {
			#pragma omp task depend(inout: sum)
			{
				result = spawnTasks(j-1);
				#pragma omp critical
				{
					sum += result;
					tasks++;
				}
			}
		}
	}
}

int main(void)
{
	int j=0;
	#pragma omp parallel num_threads(4)
	{
		//#pragma omp single
		{
			for (j=0; j<CHILDREN; j++)
			{
				#pragma omp task
				{
					spawnTasks(LEVELS);
				}
			}
		}
	}
	printf("tasks = %d\n sum = %d\n", tasks, sum);
	return 0;
}
