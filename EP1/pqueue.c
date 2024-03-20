#include "esc.h"

#define MAX_SIZE 1000000


int parent(int i){
    return (i-1)/2;
}

int left_child(int i){
    return 2*i+1;
}

int right_child(int i){
    return 2*i+2;
}

void swap(struct Process *x, struct Process *y){
    struct Process temp = *x;
    *x = *y;
    *y = temp;
}

void enqueue(struct Process a[], struct Process data, int *n){
    if (*n >= MAX_SIZE){
        printf("%s\n", "The heap is full. Cannot insert");
        return;
    }

    a[*n]=data;
    *n = *n+1;

    int i = *n - 1;
    while(i != 0 && a[parent(i)].dt > a[i].dt){
        swap(&a[parent(i)], &a[i]);
        i = parent(i);
    }
}

void min_heapify(struct Process a[], int i, int n){
    int left = left_child(i);

    int right = right_child(i);

    int smallest = i;

    if (left < n && a[left].dt < a[smallest].dt){
        smallest = left;
    }

    if(right < n && a[right].dt < a[smallest].dt){
        smallest = right;
    }

    if (smallest != i) {
        swap(&a[i], &a[smallest]);
        min_heapify(a,smallest,n);
    }
}

struct Process get_min(struct Process a[]){
    return a[0];
}

struct Process dequeue(struct Process a[], int *n){
    struct Process min_item = a[0];

    a[0] = a[*n-1];
    *n = *n -1;

    min_heapify(a,0,*n);
    return min_item;
}

void print_heap(struct Process a[], int n){
    int i;
    for(i=0;i<n;i++){
        printf("---> %s\n", a[i].name);
        printf("%d\n", a[i].dt);
    }
}

int main(){
    int n = 0;

    struct Process P_queue[MAX_SIZE];

    struct Process item1;
    strcpy(item1.name, "item1");
    item1.dt = 8;

    struct Process item2;
    strcpy(item2.name, "item2");
    item2.dt = 1;

    struct Process item3;
    strcpy(item3.name, "item3");
    item3.dt = 12;

    struct Process item4;
    strcpy(item4.name, "item4");
    item4.dt = 10;

    struct Process item5;
    strcpy(item5.name, "item5");
    item5.dt = 8;

    enqueue(P_queue,item1,&n);
    enqueue(P_queue,item2,&n);
    enqueue(P_queue,item3,&n);
    enqueue(P_queue,item4,&n);
    enqueue(P_queue,item5,&n);

    print_heap(P_queue,n);

    struct Process item6;
    strcpy(item5.name, "item6");
    item5.dt = 0;

    enqueue(P_queue,item6,&n);

    print_heap(P_queue,n);

    dequeue(P_queue, &n);

    print_heap(P_queue,n);

    dequeue(P_queue, &n);
    dequeue(P_queue, &n);
    dequeue(P_queue, &n);

    printf("%s\n", " ");
    print_heap(P_queue,n);


}
