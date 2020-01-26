#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    
    FILE *f = fopen(argv[1], "r");

    if(f == NULL) { // check if file can be opened
        printf("cannot open file\n");
        exit(1); 
    }
    
    char line[100];
    

    while(fgets(line, sizeof(line), f) != NULL) { // retrieve a line from the file
        
        for(size_t i = 0; line[i] != '\0'; i++) {
            
            int count = 1; // keeps track of number of consecutive same characters
            while(line[i] != '\0' && line[i] == line[i+1]) {
                count++;
                i++;
            }

            fwrite(&count, sizeof(int), 1, stdout); // print number in binary
            fprintf(stdout, "%c", line[i]); // print character
        }

    }
    
    fclose(f);
    return(0);
}

