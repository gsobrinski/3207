#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
       
    FILE *f = fopen(argv[1], "r");

    if(f == NULL) { // check if file can be opened
        printf("cannot open file\n");
        exit(1); 
    }

    int *n;
    char *val;

    fread(&n, sizeof(int), 1, f); // read first integer

    while(fread(&val, sizeof(char), 1, f) == sizeof(char)) {
        
        for(size_t i = 0; i < *n; i++) { // print character n times
            fprintf(stdout, "%s", val); 
        }
    
        
        fread(&n, sizeof(int), 1, f);

    }

}
    
