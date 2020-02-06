#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#define MAX 10 // size of queues

// event types
enum event_type {
    PROCESS_ARRIVAL,
    PROCESS_ARRIVE_CPU,
    PROCESS_FINISH_CPU,
    PROCESS_EXIT_SYSTEM,
    PROCESS_ARRIVE_DISK1,
    PROCESS_ARRIVE_DISK2,
    PROCESS_FINISH_DISK1,
    PROCESS_FINISH_DISK2,
    PROCESS_ARRIVE_NETWORK,
    PROCESS_FINISH_NETWORK,
    PROCESS_FINISH

};

// events need a time, a type, and a process ID
struct event {
    int time;
    enum event_type type;
    int id;
};

void read_config();
int randNum(int min, int max);
int processID();
struct event newEvent();

// FIFO queue functions
void display(int queue[], int front, int rear);
void enqueue(int n, int ID); // n: cpu = 1, disk1 = 2, disk2 = 3, network = 4
int dequeue(int n);
void pqSort();

// priority queue functions - add function to generate events
void pDisplay();
void pEnqueue(struct event newEvent);
struct event pDequeue();

// global config.txt values
int SEED, INIT_TIME, FIN_TIME, ARRIVE_MIN, ARRIVE_MAX, QUIT_PROB, NETWORK_PROB, CPU_MIN, CPU_MAX,
 DISK1_MIN, DISK1_MAX, DISK2_MIN, DISK2_MAX, NETWORK_MIN, NETWORK_MAX;

// keeps track of whether or not devices are occupied 
// busy = 1, free = 0
bool cpuBusy, d1Busy, d2Busy, netBusy; 

// event handlers
void handle_process_arrival(struct event oldEvent);
void handle_process_arrive_cpu(struct event oldEvent);
void handle_process_finish_cpu(struct event oldEvent);


// FIFO queues
int cpuQ[MAX], cpuFront = -1, cpuRear = -1; // CPU queue (1)
int d1Q[MAX], d1Front = -1, d1Rear = -1; // disk 1 queue (2)
int d2Q[MAX], d2Front = -1, d2Rear = -1; // disk 2 queue (3)
int netQ[MAX], netFront = -1, netRear = -1; // network queue (4)

// priority queue
struct event pq[MAX];
int pFront = -1, pRear = -1; 


int main() {

    read_config();

    //printf("\nSEED: %d\nINIT_TIME: %d\nFIN_TIME: %d\nARRIVE_MIN %d\nARRIVE_MAX %d\nQUIT_PROB %d\nNETWORK_PROB %d\nCPU_MIN %d\nCPU_MAX %d\nDISK1_MIN %d\nDISK1_MAX %d\nDISK2_MIN %d\nDISK2_MAX %d\nNETWORK_MIN %d\nNETWORK_MAX %d\n",  
    //SEED, INIT_TIME, FIN_TIME, ARRIVE_MIN, ARRIVE_MAX, QUIT_PROB, NETWORK_PROB, CPU_MIN, CPU_MAX, DISK1_MIN, DISK1_MAX, DISK2_MIN, DISK2_MAX, NETWORK_MIN, NETWORK_MAX);

    srand(SEED);

    struct event new_event = newEvent();


}

// reads and save all constants from the CONFIG.txt file 
void read_config() {

    FILE *f = fopen("CONFIG.txt", "r");

    if(f == NULL) {
        printf("could not open file\n");
        exit(1);
    }

    // scan all values from f
    fscanf(f, "SEED %d INIT_TIME %d FIN_TIME %d ARRIVE_MIN %d ARRIVE_MAX %d QUIT_PROB %d NETWORK_PROB %d CPU_MIN %d CPU_MAX %d DISK1_MIN %d DISK1_MAX %d DISK2_MIN %d DISK2_MAX %d NETWORK_MIN %d NETWORK_MAX %d", 
    &SEED, &INIT_TIME, &FIN_TIME, &ARRIVE_MIN, &ARRIVE_MAX, &QUIT_PROB, &NETWORK_PROB, &CPU_MIN, &CPU_MAX, &DISK1_MIN, &DISK1_MAX, &DISK2_MIN, &DISK2_MAX, &NETWORK_MIN, &NETWORK_MAX);

    return;

}

// generates and returns a random number between min and max 
int randNum(int min, int max) { 

    int n = (rand() % (max - min + 1)) + min; 

    return n;
}

// generates and returns a unique process ID
int processID() {
    static int counter;
    counter++;
    return counter;
}

// generate new events
struct event newEvent() {
    struct event newEvent;
    newEvent.time = randNum(ARRIVE_MIN, ARRIVE_MAX);
    newEvent.type = PROCESS_ARRIVAL;
    newEvent.id = processID();

    return newEvent;

}

// adds a process to queue n
void enqueue(int n, int ID) {

    if(n == 1) {
        if(cpuRear == -1) {
            cpuFront = cpuRear = 0;
            cpuQ[cpuRear] = ID;
        } else if (cpuRear == MAX-1) {
            printf("\nqueue is full\n");
            return;
        } else {
            cpuRear++;
            cpuQ[cpuRear] = ID;

        }
    } else if (n == 2) {
        if(d1Rear == -1) {
            d1Front = d1Rear = 0;
            d1Q[d1Rear] = ID;
        } else if (d1Rear == MAX-1) {
            printf("\nqueue is full\n");
            return;
        } else {
            d1Rear++;
            d1Q[d1Rear] = ID;

        }
    } else if (n == 3) {
        if(d2Rear == -1) {
            d2Front = d2Rear = 0;
            d2Q[d2Rear] = ID;
        } else if (d2Rear == MAX-1) {
            printf("\nqueue is full\n");
            return;
        } else {
            d2Rear++;
            d2Q[d2Rear] = ID;

        }
    } else if (n == 4) {
        if(netRear == -1) {
            netFront = netRear = 0;
            netQ[netRear] = ID;
        } else if (netRear == MAX-1) {
            printf("\nqueue is full\n");
            return;
        } else {
            netRear++;
            netQ[netRear] = ID;

        }
    }

}

// returns the element at the front of queue n
int dequeue(int n) {

    int a;

    if(n == 1) {
        if(cpuFront == -1) {
            printf("\nqueue is empty\n");
            return 0;
        } else if (cpuFront == cpuRear) {
            a = cpuQ[cpuFront];
            cpuFront = cpuRear = -1;
        } else {
            a = cpuQ[cpuFront];
            cpuFront++;
        }

    } else if (n == 2) {
        if(d1Front == -1) {
            printf("\nqueue is empty\n");
            return 0;
        } else if (d1Front == d1Rear) {
            a = d1Q[d1Front];
            d1Front = d1Rear = -1;
        } else {
            a = d1Q[d1Front];
            d1Front++;
        }

    } else if (n == 3) {
        if(d2Front == -1) {
            printf("\nqueue is empty\n");
            return 0;
        } else if (d2Front == d2Rear) {
            a = d2Q[d2Front];
            d2Front = d2Rear = -1;
        } else {
            a = d2Q[d2Front];
            d2Front++;
        }

    } else if (n == 4) {
        if(netFront == -1) {
            printf("\nqueue is empty\n");
            return 0;
        } else if (netFront == netRear) {
            a = netQ[netFront];
            netFront = netRear = -1;
        } else {
            a = netQ[netFront];
            netFront++;
        }

    }

    return a;

}

// displays queue
void display(int queue[], int front, int rear) {

    if(rear == -1) {
        printf("\nqueue is empty\n");
    }

    for(size_t i = front; i <= rear; i++) {
        printf("%d ", queue[i]);
    }

}

// adds an event to the priority queue
void pEnqueue(struct event newEvent) {
    if(pRear == -1) {
        pFront = pRear = 0;
        pq[pRear] = newEvent;
        
    } else if (pRear == MAX-1) {
        printf("\nqueue is full\n");
        return;
    } else {
        pRear++;
        pq[pRear] = newEvent;

    }

}

// returns the struct at the front of the priority queue
struct event pDequeue() {
    struct event e;

    if(pFront == -1) {
        printf("\nqueue is empty\n");
        return e;
    } else if (pFront == pRear) {
        e = pq[pFront];
        pFront = pRear = -1;
    } else {
        e = pq[pFront];
        pFront++;
    }
    
    return e;

}

// sorts the priority queue by time
void pqSort() {

    for(size_t i = pFront; i <= pRear; i++) {
        for(size_t j = pFront; j <= pRear-i; j++) {
            if(pq[j].time > pq[j+1].time) {
                struct event temp = pq[j];
                pq[j] = pq[j+1];
                pq[j+1] = temp;
            }
        }
    }
   
}


// displays the priority queue
void pDisplay() {
    if(pRear == -1) {
        printf("\nqueue is empty\n");
    }

    for(size_t i = pFront; i <= pRear; i++) {
        printf("\ntime: %d type: %d id: %d", pq[i].time, pq[i].type, pq[i].id);
    }
}


void handle_process_arrival(struct event oldEvent) {
    // if the cpu is busy or if the cpu queue is nonempty
    if(cpuBusy == 1 || cpuQ[cpuFront] != 0){
        enqueue(1, oldEvent.id);
    // cpu is not occupied AND queue is empty
    } else { 
        struct event newEvent;
        newEvent.time = oldEvent.time;
        newEvent.type = PROCESS_ARRIVE_CPU;
        newEvent.id = oldEvent.id;
        cpuBusy = 1;
        pEnqueue(newEvent);
    }

    struct event newEvent;
    newEvent.time = oldEvent.time + randNum(ARRIVE_MIN, ARRIVE_MAX);
    newEvent.type = PROCESS_ARRIVAL;
    newEvent.id = processID();
    pEnqueue(newEvent);

}

void handle_process_arrive_cpu(struct event oldEvent) {
    struct event newEvent;
    newEvent.time = oldEvent.time + randNum(CPU_MIN, CPU_MAX);
    newEvent.type = PROCESS_FINISH_CPU;
    newEvent.id = oldEvent.id;
    pEnqueue(newEvent);
}

void handle_process_finish_cpu(struct event oldEvent) {
    cpuBusy = 0;
    int num = randNum(1, 100);

    // process exits the system
    if (num < QUIT_PROB) {
        struct event newEvent;
        newEvent.time = oldEvent.time;
        newEvent.type = PROCESS_EXIT_SYSTEM;
        newEvent.id = oldEvent.id;
        pEnqueue(newEvent);
    
    // process enters the network
    } else if(num < NETWORK_PROB) {
        if(netBusy == 0) { // if network is not occupied
            struct event newEvent;
            newEvent.time = oldEvent.time;
            newEvent.type = PROCESS_ARRIVE_NETWORK;
            newEvent.id = oldEvent.id;
            netBusy = 1;
            pEnqueue(newEvent);
        } else { // if network is busy, put the process on the network queue
            enqueue(4, oldEvent.id);
        }
    // process enters one of the disks
    } else if (d1Busy == 0) { // if disk1 is not occupied
        struct event newEvent;
        newEvent.time = oldEvent.time;
        newEvent.type = PROCESS_ARRIVE_DISK1;
        newEvent.id = oldEvent.id;
        d1Busy = 1;
        pEnqueue(newEvent);

    } else if (d2Busy == 0) { // if disk2 is not occupied
        struct event newEvent;
        newEvent.time = oldEvent.time;
        newEvent.type = PROCESS_ARRIVE_DISK2;
        newEvent.id = oldEvent.id;
        d2Busy = 1;
        pEnqueue(newEvent);

    // add to disk1 queue 
    } else if (sizeof(d1Q)/sizeof(d1Q[0]) < sizeof(d2Q)/sizeof(d2Q[0])){ 
        enqueue(2, oldEvent.id);
    // add to disk2 queue
    } else { 
        enqueue(3, oldEvent.id);
    }
}



