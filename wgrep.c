#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    
    for(size_t i = 2; i < argc; i++) { // start at i = 2 for files

        FILE *f = fopen(argv[i], "r");
        char line[50];

        char *word = argv[1];
    
        // check if the file can be opened
        if(f == NULL) {
            printf("cannot open file\n");
            exit(1); // return exit status 1
        }


        while (fgets(line, sizeof(line), f)) { // retrieve 1 line of the file at a time 
            if(strstr(line, word)) {
                printf("%s", line);
            }
        }

        fclose(f); // close the file
    
    }

    return(0);
}
