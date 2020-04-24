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
#include <errno.h>
#include <assert.h>

#define N_THREADS 8
#define max_time 10
#define SIGUSR1 10
#define SIGUSR2 12
#define SIGTERM 15

// shared memory struct
struct shared_val {
    int sigusr1_sent;
    int sigusr1_received;
    int sigusr2_sent;
    int sigusr2_received;

    int sigusr1_report_received;
    int sigusr2_report_received;
    int signal_counter;

    int total_signals;

    pthread_mutex_t mutex;
}; 

// reporting process struct
struct signal_time {
    int signal;
    int time;
};

struct shared_val shm_ptr;

// signal masking functions
void block_sigusr();
void unblock_sigusr();
void block_sigusr1();
void block_sigusr2();

// init functions
double randNum(int min, int max);
int randSignal();
int sleep_milli(long given_time);
int init_mutex();

// signal generator/handlers
void *signal_generator(void *arg);
void *sigusr1_handler(void *arg);
void *sigusr2_handler(void *arg);


// signal reporting functions
void *signal_reporter_main(void *arg);
int get_time();
void print_current_time();
void print_avg_time_gap();
void print_results();
struct signal_time st_array[10];
// array which holds worker threads
pthread_t threads[N_THREADS];

int main() {

    // initialize struct
    shm_ptr.sigusr1_sent = 0;
    shm_ptr.sigusr1_received = 0;
    shm_ptr.sigusr2_sent = 0;
    shm_ptr.sigusr2_received = 0;
    shm_ptr.sigusr1_report_received = 0;
    shm_ptr.sigusr2_report_received = 0;
    shm_ptr.signal_counter = 0;
    shm_ptr.total_signals = 0;

    init_mutex(); // initialize mutex
    block_sigusr();

    for(size_t i = 0; i < N_THREADS; i++) {

        if(i == 0 || i == 1 || i == 2) { // signal generating process
            // create thread
            if (pthread_create(&threads[i],
                    NULL,
                    signal_generator,
                    NULL) != 0){
                printf("Error: Failed to create thread\n");
                exit(1);
            }

        } else if (i == 3 || i == 4) { // signal handling process for SIGUSR1
            //create thread
            if (pthread_create(&threads[i],
                    NULL,
                    sigusr1_handler,
                    NULL) != 0){
                printf("Error: Failed to create thread\n");
                exit(1);
            }

        } else if (i == 5 || i == 6) { // signal handling process for SIGUSR2
            // create thread
            if (pthread_create(&threads[i],
                    NULL,
                    sigusr2_handler,
                    NULL) != 0){
                printf("Error: Failed to create thread\n");
                exit(1);
            } 
        
        } else { // reporting process
            // call signal reporting function
            if (pthread_create(&threads[i],
                    NULL,
                    signal_reporter_main,
                    NULL) != 0){
                printf("Error: Failed to create thread\n");
                exit(1);
            }
        }
    }

    //sleep(30);
    //exit(0);

    while(1) {
        if(shm_ptr.total_signals >= 10000) {
            printf("\ntotal signals: %d", shm_ptr.total_signals);
            exit(0);
        }
        sleep(0.1);
    }
    
}


void block_sigusr() {
    sigset_t sigset;
    sigemptyset(&sigset); // initalize set to empty
    sigaddset(&sigset, SIGUSR1); // add SIGUSR1 to set
    sigaddset(&sigset, SIGUSR2); // add SIGUSR2 to set
    pthread_sigmask(SIG_BLOCK, &sigset, NULL); // modify mask

}

void unblock_sigusr() {
    sigset_t sigset;
    sigemptyset(&sigset); // initalize set to empty
    sigaddset(&sigset, SIGUSR1); // add SIGUSR1 to set
    sigaddset(&sigset, SIGUSR2); // add SIGUSR2 to set
    pthread_sigmask(SIG_UNBLOCK, &sigset, NULL); // modify mask
}

void block_sigusr1() {
    sigset_t sigset;
    sigemptyset(&sigset); 
    sigaddset(&sigset, SIGUSR1); // add SIGUSR1 to set
    pthread_sigmask(SIG_BLOCK, &sigset, NULL); 
}

void block_sigusr2() {
    sigset_t sigset;
    sigemptyset(&sigset); 
    sigaddset(&sigset, SIGUSR2); // add SIGUSR2 to set
    pthread_sigmask(SIG_BLOCK, &sigset, NULL); 

}

// returns a random number between min and max
double randNum(int min, int max) {
    long interval = (long) ((rand() % (max - min + 1)) + min);
    return interval;
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
int sleep_milli(long given_time) {

    int ret;
    struct timespec ts;

    assert(given_time >= 0);
    ts.tv_sec = given_time / 1000;
    ts.tv_nsec = (given_time % 1000) * 1000000;
    do {
        ret = nanosleep(&ts, &ts);
    } while (ret && errno == EINTR);

}

int init_mutex() { 
    // init mutex
    if(pthread_mutex_init(&(shm_ptr.mutex), NULL) != 0) {
        puts("error");
        return 0;
    }

    return 1; // success
}

// increments signal sent counters
void *signal_generator(void *arg) {

	while (1) {    

        sleep(0.5);

        double rand_interval = randNum(10, 100); // generate random interval
        sleep_milli(rand_interval); // sleep random interval

        int signal = randSignal(); // randomly choose between SIGUSR1 or SIGUSR2

        // broadcast signal to all threads
        for(size_t i = 0; i < N_THREADS; i++) {
            pthread_kill(threads[i], signal);
        }


        if (signal == SIGUSR1){ // SIGUSR1
            
            //acquire lock 
            pthread_mutex_lock(&(shm_ptr.mutex));

            shm_ptr.sigusr1_sent++; //increment signal counter
            shm_ptr.total_signals++;
            
            // release lock
            pthread_mutex_unlock(&(shm_ptr.mutex));

        } else { // SIGUSR2

            //acquire lock 
            pthread_mutex_lock(&(shm_ptr.mutex));

            shm_ptr.sigusr2_sent++; //increment signal counter
            shm_ptr.total_signals++;
            
            // release lock
            pthread_mutex_unlock(&(shm_ptr.mutex));
        } 
    }

}

void *sigusr1_handler(void *arg) {

    sigset_t sigset;
    int return_val = 0;
    int signal;
    sigemptyset(&sigset);
    // initalize set to empty
    sigaddset(&sigset, SIGUSR1);

    while(1) {

        return_val = sigwait(&sigset, &signal);

        //acquire lock 
        pthread_mutex_lock(&(shm_ptr.mutex));

        shm_ptr.sigusr1_received++; //increment signal counter
        shm_ptr.total_signals++;
        
        // release lock
        pthread_mutex_unlock(&(shm_ptr.mutex));

    }

}

void *sigusr2_handler(void *arg) {

    sigset_t sigset;
    int return_val = 0;
    int signal;
    sigemptyset(&sigset);
    // initalize set to empty
    sigaddset(&sigset, SIGUSR2);

    while(1) {

        return_val = sigwait(&sigset, &signal);

        //acquire lock 
        pthread_mutex_lock(&(shm_ptr.mutex));

        shm_ptr.sigusr2_received++; //increment signal counter
        shm_ptr.total_signals++;
        
        // release lock
        pthread_mutex_unlock(&(shm_ptr.mutex));
    }
}


// signal reporting process 
void *signal_reporter_main(void *arg) {

    sigset_t sigset;
    int return_val = 0;
    int signal;
    sigemptyset(&sigset);
    // initalize set to empty
    sigaddset(&sigset, SIGUSR1);
    sigaddset(&sigset, SIGUSR2);

    while(1) {

        return_val = sigwait(&sigset, &signal);

        // every 10 signals
        if(shm_ptr.signal_counter == 10) {
            print_current_time();
            print_results();
            print_avg_time_gap();
            shm_ptr.signal_counter = 0;
        }

        shm_ptr.total_signals++;

        if(signal == SIGUSR1) { // SIGUSR1
            st_array[shm_ptr.signal_counter].time = get_time();
            st_array[shm_ptr.signal_counter].signal = SIGUSR1;

            // increment counter
            shm_ptr.sigusr1_report_received++;

        } else { // SIGUSR2
            st_array[shm_ptr.signal_counter].time = get_time();
            st_array[shm_ptr.signal_counter].signal = SIGUSR2;

            // increment counter
            shm_ptr.sigusr2_report_received++;    
        }
        shm_ptr.signal_counter++;
        
    }
}

int get_time() {
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    //printf ("\ncurrent system time: %d:%d:%d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    return timeinfo->tm_sec;
}

void print_current_time() {
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf ("\n\ncurrent system time: %d:%d:%d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

void print_avg_time_gap() {

    int num_1 = 0; 
    int num_2 = 0;
    int total_time_1 = 0;
    int total_time_2 = 0;
    int prev_time_1 = 0;
    int prev_time_2 = 0;

    for(size_t i = 0; i < 10; i++) {

        if(st_array[i].signal == SIGUSR1) { // SIGUSR1

            if(num_1 == 0) {
                prev_time_1 = st_array[i].time;
                i++;
            }
            num_1++;
            int temp = st_array[i].time - prev_time_1;
            total_time_1 += temp;
            prev_time_1 = st_array[i].time;
        
        } else { // SIGUSR2
            if(num_2 == 0) {
                prev_time_2 = st_array[i].time;
                i++;
            }
            num_2++;
            int temp = st_array[i].time - prev_time_2;
            total_time_2 += temp;
            prev_time_2 = st_array[i].time;
        }
    }
    //printf("\ntotal_1: %d total_time_1: %d", num_1, total_time_1);
    //printf("\ntotal_2: %d total_time_2: %d", num_2, total_time_2);

    double avg_1 = total_time_1/(double)num_1;
    double avg_2 = total_time_2/(double)num_2;
    printf("\naverage time between SIGUSR1: %f\naverage time between SIGUSR2: %f", avg_1, avg_2);

}

void print_results() {
    printf("\nsigusr1_sent: %d\nsigusr2_sent: %d", shm_ptr.sigusr1_sent, shm_ptr.sigusr2_sent);
    printf("\nsigusr1_received: %d\nsigusr2_received: %d", shm_ptr.sigusr1_received, shm_ptr.sigusr2_received);
    printf("\nsigusr1_report_received: %d\nsigusr2_report_received: %d", shm_ptr.sigusr1_report_received, shm_ptr.sigusr2_report_received);
    fflush(stdout);

}
