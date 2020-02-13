#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *f = fopen("text.c", "r");
    char a[50];
    
    // check if the file can be opened
    if(f == NULL) {
        printf("cannot open file\n");
        exit(1); // return exit status 1
    }

    while(fgets(a, 50, f) != NULL) {
        printf("%s", a);
    }

    fclose(f); // close the file

    return(0);
}
