#include "esc.h"
#include "pqueue.c"



void round_robin(){
/*Something*/
}

void priority_scheduling(){
/*Something*/ 
}

void shortest_job_first(FILE *file){
    struct Process P_Queue_Shortest_Job[MAX_SIZE];
    int num_processes = 0;
    int n = 0;

    create_P_Queue(file, &num_processes, &n, P_Queue_Shortest_Job);
}

void create_P_Queue(FILE *file, int *num_processes, int *n, struct Process P_Queue_Shortest_Job[]){
    char line[100];

    while(fgets(line, sizeof(line), file) != NULL) {
        (*num_processes) += 1;
        char name[MAX_NAME_LENGTH];
        int deadline, t0, dt;

        sscanf(line, "%16s %d %d %d", name, &deadline, &t0, &dt);

        struct Process process;

        strcpy(process.name, name);
        process.deadline = deadline;
        process.t0 = t0;
        process.t0 = dt;

        enqueue(P_Queue_Shortest_Job,process,&n);     
    }
}

int main(int argc, char *argv[]){
    FILE *file = fopen(argv[1], "r");

    if (file==NULL){
        perror("Erro ao abrir o arquivo \n");
        return 1;
    }

    clock_t start, end;
    double cpu_time_used;
    start = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; // Calcula o tempo de CPU usadocpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; // Calcula o tempo de CPU usado

    shortest_job_first(file);

    end = clock();
    fclose(file);
    return 0;

}