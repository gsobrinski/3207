#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define MAX 100

int getInput(char* line);
int parse(char* line, char** tokens);

int main() {

    bool running = true;
    
    while(running == true) {

        char *tokens[MAX];
        char line[MAX];

        int result = getInput(line);

        if(result == 0) {
            printf("\nno input was given\n");
        } else {
            int cmdType = parse(line, tokens);
        }
    }

}

// gets the command line from user input
int getInput(char* line) {
    //printShell(); // print the shell info

    fgets(line, MAX, stdin); 

    if(strlen(line) == 0) { // if no input was entered
        return 0;
    } else {
        return 1;
    }
}

int parse(char* line, char** tokens) {

    char *token = strtok(line, " ");
    int i = 0;

    while(token != NULL) {
        tokens[i] = token;
        i++;

        token = strtok(NULL, " ");
    }

    return 1;

}



