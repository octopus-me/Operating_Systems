#include "esc.h"

void soma_numeros(){
    int i = 0;
    i += 1;
}

void execute_shortest_job_first(struct Priority_Queue pq){

    while(! is_empty(&pq)){
        /*Executa processo mais curto agora*/
        struct Process data = dequeue(&pq);
        printf("Dequeued process: %d  \n", data.dt);
    }

}

void shortest_job_first(FILE *file){
    struct Priority_Queue pq;

    char line[100];
    int num = 0;

    while(fgets(line, sizeof(line), file) != NULL){
        char name[MAX_NAME_LENGTH];
        int deadline, t0, dt;

        sscanf(line, "%16s %d %d %d", name, &deadline, &t0, &dt);

        struct Process data;

        strcpy(data.name, name);
        data.deadline = deadline;
        data.t0 = t0;
        data.dt = dt;

        insert(&pq, data);

        num += 1;
    }

    execute_shortest_job_first(pq);
}

int main(int argc, char *argv[]){
    FILE *file = fopen(argv[1], "r");
    
    if (file == NULL) {
        perror("Erro ao abrir o arquivo \n");
        return 1;
    }

    /*APLICAR O RELÓGIO*/

    shortest_job_first(file);

    fclose(file);
    return 0;
}
