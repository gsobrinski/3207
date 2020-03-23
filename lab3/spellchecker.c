//Grace Sobrinski
//3207 Project 3: Network Spellchecker
//Section 004

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define max_length 100
#define max_size 100000
#define max_queue_size 100

// dictionary 
int get_word(char *word);
int search(char *word, int i);
void print_dictionary();

int get_input(char* user_word);

// connection queue (from textbook)
void put(int socket);
int get();


FILE *words; // dictionary.txt
char **dictionary; // dictionary data structure

// connection queue
int queue[100];
int writePtr;
int readPtr;
int queue_size;
pthread_mutex_t mutex;
pthread_cond_t empty;
pthread_cond_t fill;


int main() {

     words = fopen("dictionary.txt", "r"); // open the dictionary in read mode

    if (words == NULL) { // check that the file exists
        puts("error opening file");
        exit(0);
    }

    char word[max_length];
    char user_word[max_length];

    dictionary = calloc(max_size, sizeof(char *));  // allocate dictionary 

    // while !EOF, get the next word from file and add it to the dictionary
    int i = 0;
    while(get_word(word) != -1) {
        dictionary[i] = (char *) malloc(100);
        strcpy(dictionary[i], word);
        i++;
    }

    //print_dictionary();

    while(get_input(user_word)) {

        // call search to check if the word is contained in the dictionary
        if(search(user_word, i)) {
            puts("OK");
        } else {
            puts("INCORRECT");
        }
    }

    for(size_t i = 0; dictionary[i] != NULL; i++) {
        free(dictionary[i]);
    }
    free(dictionary); // free allocated memory

}

// retrieves the next word in the dictionary.txt file, returns -1 if EOF
int get_word(char *word) {

    size_t max = max_length;
    int c = getline(&word, &max, words);
    return c;

}

// gets a word from user input
int get_input(char* user_word) {
    puts("enter a word");
    if (fgets(user_word, max_length, stdin) == NULL) {
        return 0;
    } 

    return 1;

}

void print_dictionary() {
    for(size_t i = 0; dictionary[i] != NULL; i++) {
        puts(dictionary[i]);
    }
}

// uses binary search to look for a word in the dictionary, if found returns 1
int search(char *word, int i) {

    int min = 0; 
    int max = i-1; 

    while (min <= max) { 
        int mid = min + (max - min) / 2; 
  
        // check if word == mid
        if (strcasecmp(dictionary[mid], word) == 0) { 
            return 1; // word found, return 1
        }
   
        // if word is greater move min so we only search the right half of the dictionary
        if (strcasecmp(dictionary[mid], word) < 0) {
            min = mid + 1; 
        }
  
        // else move max so we only search the left half
        else {
            max = mid - 1; 
        }
    } 
  
    // user_word not found
    return 0; 

}

// add socket descriptor to the connection queue (taken from textbook)
void put(int socket) {

    pthread_mutex_lock(&mutex); // acquire lock

    // initialize empty
    if(pthread_cond_init(&empty, NULL) != 0) {
        puts("error");
    }

    // if the connection queue is full, block thread
    while (queue_size == max_queue_size){
        pthread_cond_wait(&empty, &mutex);
    }

    queue[writePtr] = socket;
    writePtr = (writePtr + 1) % max_queue_size;
    queue_size++;
    
    // intialize fill
    if(pthread_cond_init(&fill, NULL) != 0) {
        puts("error");
    }

    // signal that socket has been filled and release lock
    pthread_cond_signal(&fill);
    pthread_mutex_unlock(&mutex);

}

// remove and return socket descriptor from the connection queue (taken from textbook)
int get() {

    pthread_mutex_lock(&mutex); // acquire lock

    int socket = queue[readPtr];
    readPtr = (readPtr + 1) % max_queue_size;
    queue_size--;

    pthread_mutex_unlock(&mutex); // release lock

    return socket;

}


