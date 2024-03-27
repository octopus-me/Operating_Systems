#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include <unistd.h>
#include <time.h>

#include <string.h>

#define MAX_PROCESSES 10000

#define MICROSECONDS 1000000

#define MAX_NAME_LENGTH 50

#define QUANTUM 0.1

/*ESTRUTURA QUE GUARDA UM PROCESSO*/
struct Process {
    char name[MAX_NAME_LENGTH];
    int deadline;
    int t0;
    int dt;
    double remaining_time;
};
/*---------------------------------*/


/*VARIÁVEIS DE CONTROLE DO TEMPO*/
clock_t global_time;
clock_t intervalo;
double global_time_seconds;
double tempo_de_processo;
/*--------------------------------*/

/*VARIÁVEL QUE IDENTIFICA O ESCALONAMENTO*/
int Flag;

/*
Flag = 1 (Shortest Job First Scheduling)
Flag = 2 (Round Robin Scheduling)
Flag = 3 (Priority Scheduling)
*/


/*-------------------------------------------------+*/
/*PRIORITY QUEUE*/
struct Priority_Queue {
    int quant;
    struct Process process[MAX_PROCESSES];
};

void swap(struct Process *a, struct Process *b);
void heapify(struct Process arr[], int n, int i);
void insert(struct Priority_Queue *pq, struct Process proc);
struct Process dequeue(struct Priority_Queue *pq) ;
int is_empty(struct Priority_Queue *pq);
void print_queue(struct Priority_Queue *pq);
/*-------------------------------------------------+*/


/*-------------------------------------------------+*/
/*CIRCULAR QUEUE*/
struct Circular_Queue {
    struct Process items[MAX_PROCESSES];
    int front;
    int rear;
};

void initializeQueue(struct Circular_Queue *q);
int isFull(struct Circular_Queue *q);
int isEmpty(struct Circular_Queue *q);
void enQueue(struct Circular_Queue *q, struct Process element);
struct Process deQueue(struct Circular_Queue *q);
void display(struct Circular_Queue *q);
/*-------------------------------------------------+*/

/*FUNÇÕES DE USO GERAL*/

void atualiza_tempo();

void read_file(FILE *file);

void *run_process(void *arg);

void ordena_processo_por_t0(struct Process processes[], int num_processes);

void new_processes(struct Process PROCESS[],int *indice_atual, int quant_processos, struct Circular_Queue *cq, struct Priority_Queue *pq);


/*ESCALONAMENTO COM PRIORIDADE COM PRIORIDADES*/

/*COM PRIORIDADES*/

/*
    Prioridade 1 - Crítico - 10 Quantuns    - Atrasados
    Prioridade 2 - Alto    -  7 Quantuns    - 1 segundo
    Prioridade 3 - Medio   -  5 Quantuns    - 2 segundos
    Prioridade 4 - Baixo   -  2 Quantuns    - Demais
*/

void execute_priority_scheduling(struct Process PROCESS[], int quant_processo);

/*ESCALONAMENTO ROUND ROBIN*/

void execute_round_robin(struct Process PROCESS[], int quant_processo);

/*ESCALONAMENTO TAREFA MAIS CURTA*/

void execute_shortest_job_first(struct Process PROCESSOS[], int quant_processo);

