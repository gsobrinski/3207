#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#define MAX 10000 // size of queues

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
void handle_process_exit_system(struct event oldEvent);
void handle_process_arrive_disk1(struct event oldEvent);
void handle_process_arrive_disk2(struct event oldEvent);
void handle_process_finish_disk1(struct event oldEvent);
void handle_process_finish_disk2(struct event oldEvent);
void handle_process_arrive_network(struct event oldEvent);
void handle_process_finish_network(struct event oldEvent);
void handle_process_finish(struct event oldEvent);



// FIFO queues
int cpuQ[MAX], cpuFront = -1, cpuRear = -1, cpuSize = 0; // CPU queue (1)
int d1Q[MAX], d1Front = -1, d1Rear = -1, d1Size = 0; // disk 1 queue (2)
int d2Q[MAX], d2Front = -1, d2Rear = -1, d2Size = 0; // disk 2 queue (3)
int netQ[MAX], netFront = -1, netRear = -1, netSize = 0;; // network queue (4)

// STATS file 

double cpuAvg, cpuUtil, cpuCount, cpuResponse, cpuThroughput;
double d1Avg, d1Util, d1Count, d1Response, d1Throughput;
double d2Avg, d2Util, d2Count, d2Response, d2Throughput;
double netAvg, netUtil, netCount, netResponse, netThroughput;
int cpuMax = 0, d1Max = 0, d2Max = 0, netMax = 0;
int cpuTotal, d1Total, d2Total, netTotal;
int cpuMaxResponse = 0, d1MaxResponse = 0, d2MaxResponse = 0, netMaxResponse = 0;

// priority queue
struct event pq[MAX];
int pFront = -1, pRear = -1; 

FILE *logFile, *statsFile;


int main() {

    logFile = fopen("log.c", "w+");
    statsFile = fopen("stats.c", "w+");

    read_config();

    fprintf(logFile, "\nSEED: %d\nINIT_TIME: %d\nFIN_TIME: %d\nARRIVE_MIN %d\nARRIVE_MAX %d\nQUIT_PROB %d\nNETWORK_PROB %d\nCPU_MIN %d\nCPU_MAX %d\nDISK1_MIN %d\nDISK1_MAX %d\nDISK2_MIN %d\nDISK2_MAX %d\nNETWORK_MIN %d\nNETWORK_MAX %d\n",  
    SEED, INIT_TIME, FIN_TIME, ARRIVE_MIN, ARRIVE_MAX, QUIT_PROB, NETWORK_PROB, CPU_MIN, CPU_MAX, DISK1_MIN, DISK1_MAX, DISK2_MIN, DISK2_MAX, NETWORK_MIN, NETWORK_MAX);

    srand(SEED);

    // create initial events
    struct event first, last;
    first.time = INIT_TIME;
    first.type = PROCESS_ARRIVAL;
    first.id = processID();
    last.time = FIN_TIME;
    last.type = PROCESS_FINISH;
    last.id = 0;
    pEnqueue(first);
    pEnqueue(last);

    bool running = true;
    struct event e;


    while(running = true) {

        e = pDequeue();

        switch(e.type) {
            case PROCESS_ARRIVAL: 
                handle_process_arrival(e);
                break;
            
            case PROCESS_ARRIVE_CPU:
                handle_process_arrive_cpu(e);
                break;
            
            case PROCESS_FINISH_CPU:
                handle_process_finish_cpu(e);
                break;

            case PROCESS_EXIT_SYSTEM:
                handle_process_exit_system(e);
                break;

            case PROCESS_ARRIVE_DISK1:
                handle_process_arrive_disk1(e);
                break;

            case PROCESS_ARRIVE_DISK2:
                handle_process_arrive_disk2(e);
                break;
            
            case PROCESS_FINISH_DISK1:
                handle_process_finish_disk1(e);
                break;
            
            case PROCESS_FINISH_DISK2:
                handle_process_finish_disk2(e);
                break;
            
            case PROCESS_ARRIVE_NETWORK:
                handle_process_arrive_network(e);
                break;

            case PROCESS_FINISH_NETWORK:
                handle_process_finish_network(e);
                break;

            case PROCESS_FINISH:
                handle_process_finish(e);
                fclose(logFile);
                fclose(statsFile);
                running = false;
                exit(0);
                break;
        }

    }


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

        cpuSize++;

        // stats file
        cpuAvg += cpuSize;
        cpuTotal++;
        if(cpuSize > cpuMax) {
            cpuMax = cpuSize;
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

        d1Size++;

        // stats file
        d1Avg += d1Size;
        d1Total++;
        if(d1Size > d1Max) {
            d1Max = d1Size;
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

        d2Size++;

        //stats file
        d2Avg += d2Size;
        d2Total++;
        if(d2Size > d2Max) {
            d2Max = d2Size;
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

        netSize++;

        // stats file
        netAvg += netSize;
        netTotal++;
        if(netSize > netMax) {
            netMax = netSize;
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

        cpuSize--;

        // stats file
        cpuAvg += cpuSize;
        cpuTotal++;

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

        d1Size--;

        // stats file
        d1Avg += d1Size;
        d1Total++;

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

        d2Size--;

        //stats file
        d2Avg += d2Size;
        d2Total++;

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

        netSize--;

        // stats file
        netAvg += netSize;
        netTotal++;

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


    // sort the priority queue
    pqSort();

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

    struct event temp;

    for(size_t i = pFront; i <= pRear; i++) {
        for(size_t j = pFront; j < pRear; j++) {
            if(pq[j].time > pq[j+1].time) {
                temp = pq[j];
                pq[j] = pq[j+1];
                pq[j+1] = temp;

            }
        }
    }
   
}


// displays the priority queue
void pDisplay() {

    printf("\npriority queue:");
    if(pRear == -1) {
        printf("\nqueue is empty\n");
    }

    for(size_t i = pFront; i <= pRear; i++) {
        printf("\ntime: %d type: %d id: %d", pq[i].time, pq[i].type, pq[i].id);
    }
}


// EVENT HANDLERS:

void handle_process_arrival(struct event oldEvent) {

    fprintf(logFile, "\ntime %d: process %d arrives to the system", oldEvent.time, oldEvent.id);

    // if the cpu is busy or if the cpu queue is nonempty, add to CPU queue
    if(cpuBusy == 1 || cpuSize > 0){
        fprintf(logFile, "\ntime %d: process %d enters the CPU queue", oldEvent.time, oldEvent.id);
        enqueue(1, oldEvent.id);
    // if cpu is not occupied AND queue is empty, create new event
    } else { 
        struct event newEvent;
        newEvent.time = oldEvent.time;
        newEvent.type = PROCESS_ARRIVE_CPU;
        newEvent.id = oldEvent.id;
        cpuBusy = 1;
        pEnqueue(newEvent);
    }

    // create new event
    struct event newEvent;
    newEvent.time = oldEvent.time + randNum(ARRIVE_MIN, ARRIVE_MAX);
    newEvent.type = PROCESS_ARRIVAL;
    newEvent.id = processID();
    pEnqueue(newEvent);

}

void handle_process_arrive_cpu(struct event oldEvent) {
    fprintf(logFile, "\ntime %d: process %d enters the CPU", oldEvent.time, oldEvent.id);
    struct event newEvent;
    int num = randNum(CPU_MIN, CPU_MAX);

    // stats file
    cpuUtil += num;
    cpuCount++;
    if (num > cpuMaxResponse) {
        cpuMaxResponse = num;
    }

    newEvent.time = oldEvent.time + num;
    newEvent.type = PROCESS_FINISH_CPU;
    newEvent.id = oldEvent.id;
    pEnqueue(newEvent);
}

void handle_process_finish_cpu(struct event oldEvent) {
    cpuBusy = 0;
    int num = randNum(1, 100);

    fprintf(logFile, "\ntime %d: process %d exits the CPU", oldEvent.time, oldEvent.id);

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

    // add to the smaller disk queue
    } else if (d1Size <= d2Size){ 
        enqueue(2, oldEvent.id);
    } else { 
        enqueue(3, oldEvent.id);
    }

    // if CPU queue is nonempty, create process_arrive_cpu event
    if(cpuSize > 0) {
        int newID = dequeue(1); // pull process off cpu queue
        struct event newEvent;
        newEvent.time = oldEvent.time;
        newEvent.type = PROCESS_ARRIVE_CPU;
        newEvent.id = newID;
        cpuBusy = 1; // set cpu to occupied
        pEnqueue(newEvent);
    }

}

void handle_process_exit_system(struct event oldEvent) {
    fprintf(logFile, "\ntime %d: process %d exits the system", oldEvent.time, oldEvent.id);

}

void handle_process_arrive_disk1(struct event oldEvent) {
    fprintf(logFile, "\ntime %d: process %d enters disk 1", oldEvent.time, oldEvent.id);
    struct event newEvent;
    int num = randNum(DISK1_MIN, DISK1_MAX);

    // stats file
    d1Util += num;
    d1Count++;
    if (num > d1MaxResponse) {
        d1MaxResponse = num;
    }

    newEvent.time = oldEvent.time + num;
    newEvent.type = PROCESS_FINISH_DISK1;
    newEvent.id = oldEvent.id;
    pEnqueue(newEvent);
}

void handle_process_arrive_disk2(struct event oldEvent) {
    fprintf(logFile, "\ntime %d: process %d enters disk 2", oldEvent.time, oldEvent.id);
    struct event newEvent;
    int num = randNum(DISK2_MIN, DISK2_MAX);

    d2Util += num;
    d2Count++;
    if (num > d2MaxResponse) {
        d2MaxResponse = num;
    }
    
    newEvent.time = oldEvent.time + num;
    newEvent.type = PROCESS_FINISH_DISK2;
    newEvent.id = oldEvent.id;
    pEnqueue(newEvent);
}

void handle_process_finish_disk1(struct event oldEvent) {

    d1Busy = 0; // set disk to not occupied
    fprintf(logFile, "\ntime %d: process %d exits disk 1", oldEvent.time, oldEvent.id);

    // if the cpu is busy or if the cpu queue is nonempty
    if(cpuBusy == 1 || cpuSize > 0){
        fprintf(logFile, "\ntime %d: process %d enters the CPU queue", oldEvent.time, oldEvent.id);
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

    // if disk queue is nonempty
    if(d1Size > 0) {
        int newID = dequeue(2); // pull process off disk1 queue
        struct event newEvent;
        newEvent.time = oldEvent.time;
        newEvent.type = PROCESS_ARRIVE_DISK1;
        newEvent.id = newID;
        d1Busy = 1; // set disk1 to occupied
        pEnqueue(newEvent);
    }
}

void handle_process_finish_disk2(struct event oldEvent) {

    d2Busy = 0; // set disk to not occupied
    fprintf(logFile, "\ntime %d: process %d exits disk 2", oldEvent.time, oldEvent.id);

    // if the cpu is busy or if the cpu queue is nonempty
    if(cpuBusy == 1 || cpuSize > 0){
        fprintf(logFile, "\ntime %d: process %d enters the CPU queue", oldEvent.time, oldEvent.id);
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

    // if disk queue is nonempty
    if(d2Size > 0) {
        int newID = dequeue(3); // pull process off disk2 queue
        struct event newEvent;
        newEvent.time = oldEvent.time;
        newEvent.type = PROCESS_ARRIVE_DISK2;
        newEvent.id = newID;
        d2Busy = 1; // set disk2 to occupied
        pEnqueue(newEvent);
    }
}

void handle_process_arrive_network(struct event oldEvent) {
    fprintf(logFile, "\ntime %d: process %d enters the network", oldEvent.time, oldEvent.id);
    struct event newEvent;
    int num = randNum(NETWORK_MIN, NETWORK_MAX);

    netUtil += num;
    netCount++;
    if (num > netMaxResponse) {
        netMaxResponse = num;
    }

    newEvent.time = oldEvent.time + num;
    newEvent.type = PROCESS_FINISH_NETWORK;
    newEvent.id = oldEvent.id;
    pEnqueue(newEvent);

}

void handle_process_finish_network(struct event oldEvent) {

    netBusy = 0; // set network to not occupied
    fprintf(logFile, "\ntime %d: process %d exits the network", oldEvent.time, oldEvent.id);

    // if the cpu is busy or if the cpu queue is nonempty
    if(cpuBusy == 1 || cpuSize > 0){
        fprintf(logFile, "\ntime %d: process %d enters the CPU queue", oldEvent.time, oldEvent.id);
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

    // if network queue is nonempty
    if(netSize > 0) {
        int newID = dequeue(4); // pull process off network queue
        struct event newEvent;
        newEvent.time = oldEvent.time;
        newEvent.type = PROCESS_ARRIVE_NETWORK;
        newEvent.id = newID;
        netBusy = 1; // set network to occupied
        pEnqueue(newEvent);
    }
}

// ends the simulation
void handle_process_finish(struct event oldEvent) {

    cpuAvg = cpuAvg/cpuTotal;
    d1Avg = d1Avg/d1Total;
    d2Avg = d2Avg/d2Total;
    netAvg = netAvg/netTotal;

    cpuResponse = cpuUtil/cpuCount;
    d1Response = d1Util/d1Count;
    d2Response = d2Util/d2Count;
    netResponse = netUtil/netCount;

    cpuUtil = cpuUtil/(FIN_TIME - INIT_TIME);
    d1Util = d1Util/(FIN_TIME - INIT_TIME);
    d2Util = d2Util/(FIN_TIME - INIT_TIME);
    netUtil = netUtil/(FIN_TIME - INIT_TIME);

    cpuThroughput = cpuCount/(FIN_TIME - INIT_TIME);
    d1Throughput = d1Count/(FIN_TIME - INIT_TIME);
    d2Throughput = d2Count/(FIN_TIME - INIT_TIME);
    netThroughput = netCount/(FIN_TIME - INIT_TIME);

    fprintf(statsFile, "\nAverage Queue Size:\nCPU = %f\nDisk 1 = %f\nDisk 2 = %f\nNetwork = %f\n", cpuAvg, d1Avg, d2Avg, netAvg);
    fprintf(statsFile, "\nMaximum Queue Size:\nCPU = %d\nDisk 1 = %d\nDisk 2 = %d\nNetwork = %d\n", cpuMax, d1Max, d2Max, netMax);
    fprintf(statsFile, "\nUtilization:\nCPU = %f\nDisk 1 = %f\nDisk 2 = %f\nNetwork - %f\n", cpuUtil, d1Util, d2Util, netUtil);
    fprintf(statsFile, "\nAverage Response Time:\nCPU = %f\nDisk 1 = %f\nDisk 2 = %f\nNetwork = %f\n", cpuResponse, d1Response, d2Response, netResponse);
    fprintf(statsFile, "\nMaximum Response Time:\nCPU = %d\nDisk 1 = %d\nDisk 2 = %d\nNetwork = %d\n", cpuMaxResponse, d1MaxResponse, d2MaxResponse, netMaxResponse);
    fprintf(statsFile, "\nThroughput:\nCPU = %f\nDisk 1 = %f\nDisk 2 = %f\nNetwork = %f\n", cpuThroughput, d1Throughput, d2Throughput, netThroughput);
    
    fprintf(logFile, "\nsimulation finished\n");

}