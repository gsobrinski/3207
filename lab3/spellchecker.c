//Grace Sobrinski
//3207 Project 3: Network Spellchecker
//Section 004

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define max_length 100
#define max_size 100000
#define max_queue_size 100
#define N_THREADS 10

// functions
void spawn_worker_threads();
void *workerThread(void *arg);
void *logThread(void *arg);
char* format_word(char *word);
int init_dictionary();
int init_variables();
int get_word(char *word);
int get_input(char* user_word);
void print_dictionary();
int search(char *word, int i);

// connection queue (from textbook)
void put_c(int socket);
int get_c();

// log queue
void put_l(char *word);
char* get_l();


FILE *words; // dictionary.txt
char **dictionary; // dictionary data structure
int numWords;

// connection queue
int queue_c[max_queue_size];
int writePtr_c;
int readPtr_c;
int queue_size_c;

// log queue
char *queue_l[max_queue_size];
int writePtr_l;
int readPtr_l;
int queue_size_l;

// mutex/cv for both queues
pthread_mutex_t mutex_l;
pthread_mutex_t mutex_c;

pthread_cond_t empty_l;
pthread_cond_t fill_l;
pthread_cond_t empty_c;
pthread_cond_t fill_c;


int main() {

    // network initialization
    int portNumber = 8888;
    int socket_desc, new_socket, c;
    struct sockaddr_in server, client;
    char *message;
    // Create socket (create active socket descriptor)
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1){ 
	puts("Error creating socket!");
	exit(1);
    }
    // prepare sockaddr_instructure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; // defaults to 127.0.0.1
    server.sin_port = htons(portNumber);
    // Bind (corrent the server's socket address to the socket descriptor)
    int bind_result = bind(socket_desc, (struct sockaddr*)&server, sizeof(server));
    if (bind_result < 0){
	puts("Error: failed to Bind.");
	exit(1);
    }
    puts("Bind done.");
    // Listen (converts active socket to a LISTENING socket which can accept connections)
    listen(socket_desc, 3);
    puts("Waiting for incoming connections...");

    // initialize variables/check for errors
    if(!init_variables()) {
        exit(0);
    }

    spawn_worker_threads();

    // initialize dictionary
    numWords = init_dictionary();

    while (1){
        // get connection file descriptor
        int fd = accept(socket_desc, (struct sockaddr *)
        &client, (socklen_t *) &c);
        if (fd < 0) {
            puts("error");
            continue;
        }
        puts("Connection accepted");
        // add socket to connection queue
        put_c(fd);
    }

    // free allocated memory
    for(size_t i = 0; dictionary[i] != NULL; i++) {
        free(dictionary[i]);
    }
    free(dictionary); 
}

void spawn_worker_threads(){
    // array which holds worker threads
    pthread_t worker_threads[N_THREADS];
    // create worker threads
    for (size_t i = 0; i < N_THREADS; ++i){
        if (pthread_create(&worker_threads[i],
                NULL,
                workerThread,
                NULL) != 0){
            printf("Error: Failed to create thread\n");
            exit(1);
        }
    }
    // array which holds logger threads
    pthread_t log_threads[N_THREADS];
    // create logger threads
    for (size_t i = 0; i < N_THREADS; ++i){
        if (pthread_create(&log_threads[i],
                NULL,
                logThread,
                NULL) != 0){
            printf("Error: Failed to create thread\n");
            exit(1);
        }
    }
}

void *workerThread(void *arg){

    while (1) {
        char *response;
        char *word = (char *)malloc(sizeof(char)*32); // allocate buffer

        int sd = get_c();
        while ( read(sd, word, 32) > 0){
            
            // remove any trailing characters from the word
            word = format_word(word);

            // search for the word
            if (search(word, numWords)){
                //strcat word and "OK"
                puts("OK");
                strcat(word, " OK\n");
                puts(word);
            } else {
                puts("not found");
                strcat(word, " not found\n");
                puts(word);
            }
            write(sd, word, strlen(word) + 1);
            // add to log queue
            put_l(word);
            word = (char *)malloc(sizeof(char)*32); // allocate buffer
        }
        close(sd);
    }
}

void *logThread(void *arg) {
    // create log file 
    FILE *logFile = fopen("log.txt", "w");

    while (1){
        // remove string from buffer
        char *word = get_l();
        fprintf(logFile, "%s", word);
        fflush(logFile);
        free(word); 
    }
}

// removes any trailing characters from word
char* format_word(char *word) {

    for(size_t i = 0; word[i] != '\0'; i++) {
        if (word[i] == '\n') {
            word[i] = '\0';
            return word;
        } else if (word[i] == '\t') {
            word[i] = '\0';
            return word;
        } else if (word[i] == '\r') {
            word[i] = '\0';
            return word;
        } else if (word[i] == ' ') {
            word[i] = '\0';
            return word;
        }
    }
    return word;
}

int init_dictionary() {
    words = fopen("dictionary.txt", "r"); // open the dictionary in read mode

    if (words == NULL) { // check that the file exists
        puts("error opening file");
        exit(0);
    }

    char word[max_length];

    dictionary = calloc(max_size, sizeof(char *));  // allocate dictionary 

    // while !EOF, get the next word from file and add it to the dictionary
    int i = 0;
    while(get_word(word) != -1) {
        dictionary[i] = (char *) malloc(100);
        word[strlen(word) - 1] = '\0';
        strcpy(dictionary[i], word);
        i++;
    }

    //print_dictionary();

    return i; // return number of words

}

// initialize variables, return 0 if any errors
int init_variables() {
    // initialize condition variables
    if(pthread_cond_init(&empty_l, NULL) != 0) {
        puts("error");
        return 0;
    }
    if(pthread_cond_init(&fill_l, NULL) != 0) {
        puts("error");
        return 0;
    }
    if(pthread_cond_init(&empty_c, NULL) != 0) {
        puts("error");
        return 0;
    }
    if(pthread_cond_init(&fill_c, NULL) != 0) {
        puts("error");
        return 0;
    }
    // initialize mutex
    if(pthread_mutex_init(&mutex_c, NULL) != 0) {
        puts("error");
        return 0;
    }
    if(pthread_mutex_init(&mutex_l, NULL) != 0) {
        puts("error");
        return 0;
    }

    return 1; // success
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
void put_c(int socket) {

    pthread_mutex_lock(&mutex_c); // acquire lock

    // if the connection queue is full, block thread
    while (queue_size_c == max_queue_size){
        pthread_cond_wait(&empty_c, &mutex_c);
    }

    queue_c[writePtr_c] = socket;
    writePtr_c = (writePtr_c + 1) % max_queue_size;
    queue_size_c++;
    
    // signal that socket has been filled and release lock
    pthread_cond_signal(&fill_c);
    pthread_mutex_unlock(&mutex_c);

}

// remove and return socket descriptor from the connection queue (taken from textbook)
int get_c() {

    pthread_mutex_lock(&mutex_c); // acquire lock

    // if the connection queue is empty, block thread
    while (queue_size_c == 0){
        pthread_cond_wait(&fill_c, &mutex_c);
    }

    int socket = queue_c[readPtr_c];
    readPtr_c = (readPtr_c + 1) % max_queue_size;
    queue_size_c--;

    pthread_cond_signal(&empty_c);
    pthread_mutex_unlock(&mutex_c); // release lock

    return socket;

}

// add string to the log queue (taken from textbook)
void put_l(char *word) {

    pthread_mutex_lock(&mutex_l); // acquire lock

    // if the connection queue is full, block thread
    while (queue_size_l == max_queue_size){
        pthread_cond_wait(&empty_l, &mutex_l);
    }

    queue_l[writePtr_l] = word;
    writePtr_l = (writePtr_l + 1) % max_queue_size;
    queue_size_l++;
    
    // signal that socket has been filled and release lock
    pthread_cond_signal(&fill_l);
    pthread_mutex_unlock(&mutex_l);

}

// remove and return a string from the log queue (taken from textbook)
char* get_l() {

    pthread_mutex_lock(&mutex_l); // acquire lock

    // if the connection queue is empty, block thread
    while (queue_size_l == 0){
        pthread_cond_wait(&fill_l, &mutex_l);
    }

    char *word = queue_l[readPtr_l];
    readPtr_l = (readPtr_l + 1) % max_queue_size;
    queue_size_l--;

    pthread_cond_signal(&empty_l);
    pthread_mutex_unlock(&mutex_l); // release lock

    return word;

}



