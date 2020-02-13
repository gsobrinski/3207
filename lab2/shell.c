#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SIZE 20

void tokenize(char line[]);
void tokenDisplay();

char *tokens[SIZE];

int main() {

    char line[50];
    char end[] = "stop";
    
    while(strcmp(line, end) != 0) {

        scanf("%s", line);

        if(strcmp(line, end) == 0) {
            break;
        } else{   
            tokenize(line);
        }
    }
}

void tokenize(char line[]) {
    char *token;
    token = strtok(line, " ");
    int count = 0;

    while(token != NULL) {
        tokens[count] = token;
        count++;

        token = strtok(NULL, " ");
    }

    tokenDisplay();

}

void tokenDisplay() {
    for(size_t i = 0; tokens[i] != NULL; i++) {
        printf("%s ", tokens[i]);
    }
}
