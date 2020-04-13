#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>

int randNum(int min, int max);
char *randSignal();
int sleep_milli(long double given_time);

int main() {
    

}

// returns a random number between min and max
int randNum(int min, int max) {
    return (rand() % (max - min + 1)) + min;
}

// randomly returns either SIGUSR1 or SIGUSR2
char *randSignal() {
    char *signalType;
    int n = randNum(1, 2);
    if(n == 1) {
        signalType = "SIGUSR1";
    } else {
        signalType = "SIGUSR2";
    }
    
    return signalType;
}

// sleeps for a given number of milliseconds, returns 1 if successful
int sleep_milli(long double given_time) {

    // nanosleep returns a value < 0 if there is an error
    // sleeps from 0 to given_time
    if(nanosleep((const struct timespec[]){{0, given_time}}, NULL) < 0 ) { 
        printf("\nnanosleep failed");
        return 0;
    }

   printf("\nnanosleep successful");
   return 1;

}