#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX 10 // size of queues

void read_config();
int randNum(int min, int max);
int processID();

// FIFO queue functions
void display(int queue[], int front, int rear);
void enqueue(int n, int ID); // n: cpu = 1, disk1 = 2, disk2 = 3, network = 4
int dequeue(int n);
void pqSort();

enum eType {
    EXECUTE
};

struct event {
    int time;
};

// priority queue functions - add function to generate events
void pDisplay();
void pEnqueue(struct event newEvent);
struct event pDequeue();


// global config.txt values
int SEED, INIT_TIME, FIN_TIME, ARRIVE_MIN, ARRIVE_MAX, QUIT_PROB, NETWORK_PROB, CPU_MIN, CPU_MAX,
 DISK1_MIN, DISK1_MAX, DISK2_MIN, DISK2_MAX, NETWORK_MIN, NETWORK_MAX;

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

    int number = randNum(ARRIVE_MIN, ARRIVE_MAX);
    //printf("\nnumber = %d\n", number);

    struct event newEvent;
    newEvent.time = 5;

    // testing priority queue
    for(size_t i = 0; i < MAX; i++) {
        pEnqueue(newEvent);
        newEvent.time = randNum(ARRIVE_MIN, ARRIVE_MAX);
    }
    printf("\nbefore dequeue:\n");
    pDisplay();
    printf("\npFront: %d pRear: %d\n", pFront, pRear);

    struct event e = pDequeue();
    printf("\nafter dequeue:\n");
    pDisplay();
    printf("\ne: %d\n", e.time);
    printf("pFront: %d pRear: %d\n", pFront, pRear);

    pqSort();
    printf("\nafter sort:\n");
    pDisplay();
    printf("pFront: %d pRear: %d\n", pFront, pRear);



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
        pq[pRear].time = newEvent.time;
    } else if (pRear == MAX-1) {
        printf("\nqueue is full\n");
        return;
    } else {
        pRear++;
        pq[pRear].time = newEvent.time;

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
        printf("%d ", pq[i].time);
    }
}

