#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#define num_pids 8
#define max_time 10

struct shared_val {
    int value;
    int sigusr1_sent;
    int sigusr1_received;
    int sigusr2_sent;
    int sigusr2_received;
    pthread_mutex_t mutex;
}; 

struct shared_val *shm_ptr;
pthread_mutexattr_t attr;

// signal masking functions
void block_sigusr();
void unblock_sigusr();
void block_sigusr1();
void block_sigusr2();

// init functions
int randNum(int min, int max);
int randSignal();
int sleep_milli(long double given_time);
int init_mutex();

void signal_generator();
void signal_handler(int signal);
void signal_reporter();
void print_results();

int PIDs[num_pids]; // process ID array
int total_time;

int main() {

    int shm_id;
    pid_t pid;
    
    shm_id = shmget(IPC_PRIVATE, sizeof(struct shared_val), IPC_CREAT | 0666); // created shared mem region
    assert(shm_id >= 0); // error check memory creation
    shm_ptr = (struct shared_val *) shmat(shm_id, NULL, 0); // attach memory
    assert(shm_ptr != (struct shared_val *) -1); // error check memory attachment
    shm_ptr->value = 0; // set value in shared memory

    init_mutex(); // initialize mutex
    block_sigusr();

    for(size_t i = 0; i < num_pids; i++) {
        
        PIDs[i] = fork();

        unblock_sigusr();

        if (PIDs[i] < 0) {
            puts("fork failed");
            exit(0);

        } else if (PIDs[i] == 0) { // child

            PIDs[i] = getpid();
            printf("\nPIDs[%lu] = %d\n", i, PIDs[i]);
            fflush(stdout);

            if(i == 0 || i == 1 || i == 2) { // signal generating process
                signal_generator();

            } else if (i == 3 || i == 4) { // signal handling process for SIGUSR1
                // unblock
                unblock_sigusr();
                // block SIGUSR2
                block_sigusr2();
                // set signal_handler to receive SIGUSR1
                signal(SIGUSR1, signal_handler);
                // call signal handler function with SIGUSR1
                signal_handler(SIGUSR1); 

            } else if (i == 5 || i == 6) { // signal handling process for SIGUSR2
                // unblock
                unblock_sigusr();
                // block SIGUSR1
                block_sigusr1();
                // set signal_handler to receive SIGUSR2
                signal(SIGUSR2, signal_handler);
                // call signal handler function with SIGUSR2
                signal_handler(SIGUSR2); 
            
            } else { // reporting process
                // unblock
                unblock_sigusr();
                // set signal_handler to receive SIGUSR1 and SIGUSR2
                signal(SIGUSR1, signal_handler);
                signal(SIGUSR2, signal_handler);
                // call signal reporting function
                signal_reporter();
            }
        }
    }
    
    print_results();

    // wait for all children to complete
    int status;
    int n = num_pids;
    while (n > 0) {
        pid = wait(&status);
        --n; 
        shmdt(shm_ptr); // detach memory before exiting
    }

    print_results();

    exit(0);
    
}

void block_sigusr() {
    sigset_t sigset;
    sigemptyset(&sigset); // initalize set to empty
    sigaddset(&sigset, SIGUSR1); // add SIGUSR1 to set
    sigaddset(&sigset, SIGUSR2); // add SIGUSR2 to set
    sigprocmask(SIG_BLOCK, &sigset, NULL); // modify mask

}

void unblock_sigusr() {
    sigset_t sigset;
    sigemptyset(&sigset); // initalize set to empty
    sigprocmask(SIG_BLOCK, &sigset, NULL); // modify mask
}

void block_sigusr1() {
    sigset_t sigset;
    sigemptyset(&sigset); 
    sigaddset(&sigset, SIGUSR1); // add SIGUSR1 to set
    sigprocmask(SIG_BLOCK, &sigset, NULL); 
}

void block_sigusr2() {
    sigset_t sigset;
    sigemptyset(&sigset); 
    sigaddset(&sigset, SIGUSR2); // add SIGUSR2 to set
    sigprocmask(SIG_BLOCK, &sigset, NULL); 

}

// returns a random number between min and max
int randNum(int min, int max) {
    return (rand() % (max - min + 1)) + min;
}

// randomly returns either SIGUSR1 or SIGUSR2
int randSignal() {
    int n = randNum(1, 2);
    if(n == 1) {
        return SIGUSR1;
    } else {
        return SIGUSR2;
    }
    
}

// sleeps for a given number of milliseconds, returns 1 if successful
int sleep_milli(long double given_time) {

    // nanosleep returns a value < 0 if there is an error
    // sleeps from 0 to given_time
    if(nanosleep((const struct timespec[]){{0, given_time}}, NULL) < 0 ) { 
        printf("\nnanosleep failed");
        return 0;
    }

   // nanosleep successful
   return 1;

}

int init_mutex() {
    // init mutex
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&(shm_ptr->mutex), &attr);

    return 1; // success
}

// increments signal sent counters
void signal_generator() {

	while (total_time < max_time) {    

        int rand_interval = randNum(.01, .1); // generate random interval
        total_time += rand_interval; // add rand_interval to the total runtime
        sleep_milli(rand_interval * 1000); // sleep random interval

        int signal = randSignal(); // randomly choose between SIGUSR1 or SIGUSR2
        kill(0, signal); // send that signal to all children

        if (signal == SIGUSR1){ // SIGUSR1
            
            //acquire lock 
            pthread_mutex_lock(&(shm_ptr->mutex));

            shm_ptr->sigusr1_sent++; //increment signal counter
            
            // release lock
            pthread_mutex_unlock(&(shm_ptr->mutex));

        } else { // SIGUSR2

            //acquire lock 
            pthread_mutex_lock(&(shm_ptr->mutex));

            shm_ptr->sigusr2_sent++; //increment signal counter
            
            // release lock
            pthread_mutex_unlock(&(shm_ptr->mutex));
        } 
    }

    exit(0);
}

// increments signal received counters 
void signal_handler(int signal) { 
    if (signal == SIGUSR1){ 

        //acquire lock 
        pthread_mutex_lock(&(shm_ptr->mutex));

        shm_ptr->sigusr1_received++; //increment signal counter
        
        // release lock
        pthread_mutex_unlock(&(shm_ptr->mutex));

    } else { // SIGUSR2

         //acquire lock 
        pthread_mutex_lock(&(shm_ptr->mutex));

        shm_ptr->sigusr2_received++; //increment signal counter
        
        // release lock
        pthread_mutex_unlock(&(shm_ptr->mutex));

    }

    exit(0);
}

void signal_reporter() {
    exit(0);
}

void print_results() {
    printf("\nsigusr1_sent: %d\nsigusr2_sent: %d", shm_ptr->sigusr1_sent, shm_ptr->sigusr2_sent);
    printf("\nsigusr1_received: %d\nsigusr2_received: %d", shm_ptr->sigusr1_received, shm_ptr->sigusr2_received);

}
