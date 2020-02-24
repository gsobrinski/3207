#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h> 
#include <fcntl.h>
#include <sys/wait.h> 
#include <dirent.h>

#define max_size 10
#define max_length 100

// struct containing I/O execution context
struct exec_context {
	
	bool contains_io;
	bool contains_error;

	bool input_redirection;
	bool output_redirection;
	bool output_redirection_append;
	bool contains_pipe;
	bool background_execution;
	
	char input_file[50];
	char output_file[50];
	char pipe_command_1[50];
	char pipe_command_2[50];
	
};


int getInput(char* line);
int tokenize(char* line, char** tokens);
void printShell();
int parse(char **tokens, struct exec_context ec);
int is_built_in(char *token);
int execute_built_in(char **tokens);

//built in commands:
int execute_cd(char *directory);
int execute_dir(char *directory);
int execute_environ();
void execute_clr();
void execute_pause();
void execute_echo(char **tokens);
void execute_quit();
void execute_help();

int execute_command(char **tokens);




int main() {

    bool running = true;
    
    while(running == true) {
        
        // initialize line and tokens array
        char line[max_length];

        char **tokens = malloc(max_size * sizeof(char *));   
        for (size_t i = 0; i < max_size; ++i) {
            tokens[i] = (char *)malloc(max_length);
        }        
        
        // initizalize execution context struct
        struct exec_context ec;
        ec.contains_io = false;
        ec.contains_error = false;

        ec.input_redirection = false;
        ec.output_redirection = false;
        ec.output_redirection_append = false;
        ec.contains_pipe = false;
        ec.background_execution = false;

        int result = getInput(line);

        if(result == 0) {
            printf("\nno input was given\n");
        } else {
            tokenize(line, tokens);
            parse(tokens, ec);
        }
    }

}

// gets the command line from user input
int getInput(char* line) {
    printShell(); // print the current directory

    fgets(line, max_length, stdin); 

    if(strlen(line) == 0) { // if no input was entered
        return 0;
    } else {
        return 1;
    }
}

int tokenize(char* line, char** tokens) {

    char *token = strtok(line, " \t\n"); //" " \t \n are all considered whitespace
    int i = 0;

    while(token != NULL) {
        tokens[i] = token;
        i++;

        token = strtok(NULL, " \t\n");
    } 

    tokens[i] = NULL;

    return 1;

}

// print shell line
void printShell() {
    char cwd[1000];
    getcwd(cwd, sizeof(cwd));
    printf("\ngsobrinski:%s$ ", cwd);
}

// accepts the tokens array and a struct that will contain the execution context
// returns 0 if error
int parse(char **tokens, struct exec_context ec) {

    if(is_built_in(tokens[0])) {
        execute_built_in(tokens);
    } else {
        execute_command(tokens);
    }

}

// accepts a string parameter, checks if that string is a built in command
// returns 1 if token is a built in command
int is_built_in(char *token) {
    
    if(strcmp(token, "cd") == 0) {
        return 1;
    } else if(strcmp(token, "dir") == 0) {
        return 1;
    } else if(strcmp(token, "environ") == 0) {
        return 1;
    } else if(strcmp(token, "clr") == 0) {
        return 1;
    } else if(strcmp(token, "pause") == 0) {
        return 1;
    } else if(strcmp(token, "echo") == 0) {
        return 1;
    } else if(strcmp(token, "quit") == 0) {
        return 1;
    } else if(strcmp(token, "help") == 0) {
        return 1;
    }

    return 0;

}

int execute_built_in(char **tokens) {

    if(strcmp(tokens[0], "cd") == 0) {
        execute_cd(tokens[1]); // pass the argument as the new directory

    } else if(strcmp(tokens[0], "dir") == 0) {
        if (tokens[1] != NULL) { // if the user specified a directory use that argument
            execute_dir(tokens[1]);
        } else {
            execute_dir("cwd"); // else use cwd
        }

    } else if(strcmp(tokens[0], "environ") == 0) {
        execute_environ();

    } else if(strcmp(tokens[0], "clr") == 0) {
        execute_clr();

    } else if(strcmp(tokens[0], "pause") == 0) {
        execute_pause();

    } else if(strcmp(tokens[0], "echo") == 0) {
        execute_echo(tokens);

    } else if(strcmp(tokens[0], "quit") == 0) {
        execute_quit();

    } else if(strcmp(tokens[0], "help") == 0) {
        execute_help();
    }

}

// built in functions

//accepts new directory as parameter, returns 1 if successful
int execute_cd(char *directory) {
	//should also change the PWD environment variable
    chdir(directory);

}

//accepts new directory as parameter, returns 1 if successful
int execute_dir(char *directory) {

	// include additional if statements for output redirection
	// make sure to check for errors

    char *dirName;
    //if no directory is given use current directory - call get_current_dir_name() (make sure to free after)
    if(strcmp(directory, "cwd") == 0) { // if no directory was specified, use cwd
         dirName =(char *)get_current_dir_name();
    } 

    DIR *dir = opendir(dirName); // error-check this 

    // check for error
    if(dir == NULL) {
        perror("cannot open");
        exit(1);
    }

    struct dirent *s; // directory entry 
    while( (s = readdir(dir)) != NULL){ // get contents 
        printf("%s\t", s->d_name); // print name of dirent 
    }
}

// print environment variables, returns 1 if successful
int execute_environ() {
	
	// include additional if statements for output redirection
	printf("\nPATH: %s", getenv("PATH"));
    printf("\nUSER: %s", getenv("USER"));
}

// clear the screen
void execute_clr() {
    printf("\033[H\033[2J"); 
}

// pause the shell until user enters
void execute_pause() {
    // use getchar() in a loop until the user hits enter (\n)
    char c;
    do {
        c = getchar();
    } while(c != '\n');
}

// accepts tokens array as a parameter, prints the tokens
void execute_echo(char **tokens) {
	for(size_t i = 0; i < max_size; i++) {
        printf("%s", tokens[i]);
    }
	
}

void execute_quit() {
	//do resource cleanup
	exit(0);
}

void execute_help() {
	
    // include additional if statements for output redirection
	// print the user manual
}

// execute commands with no I/O redirection, returns 0 if error occurs
int execute_command(char **tokens) {

    int pid;

    if ((pid = fork()) < 0) {  
        printf("fork failed");
        exit(1);
    }
    else if (pid == 0) { // fork successful, this is the child
        execvp(tokens[0], tokens);  // execute command
        printf("exec failed");
        exit(1);

    } else { // parent                              
        wait(NULL); // wait
    }
}







