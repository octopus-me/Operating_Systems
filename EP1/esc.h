#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <pthread.h>

#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <time.h>

#define MAX_NAME_LENGTH 16

struct Process{
    char name[MAX_NAME_LENGTH];
    int deadline;
    int t0;
    int dt;
};


void round_robin();
void priority_scheduling();
void shortest_job_first(FILE *file);

void create_P_Queue(FILE *file, int *num_processes, int *n, struct Process P_Queue_Shortest_Job[]);
