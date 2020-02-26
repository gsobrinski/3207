#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h> 
#include <fcntl.h>
#include <sys/wait.h> 
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

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
	
	char *input_file;
	char *output_file;
	char *pipe_command_1;
	char *pipe_command_2;
	
};

int getInput(char* line);
int tokenize(char* line, char** tokens);
void printShell();
int parse(char **tokens, struct exec_context ec);
int is_built_in(char *token);
struct exec_context is_io(char **tokens, struct exec_context ec);
void print_exec_context(struct exec_context ec);

// executing commands:
int execute_command(char **tokens);
int execute_built_in(char **tokens);
int execute_IO(char **tokens, struct exec_context ec);

// built in commands:
int execute_cd(char *directory);
int execute_dir(char **tokens);
int execute_environ();
void execute_clr();
void execute_pause();
void execute_echo(char **tokens);
void execute_quit();
void execute_help();

// I/O redirection commands:
int execute_output_redirection(char **tokens, struct exec_context ec);
int execute_input_redirection(char **tokens, struct exec_context ec);
int execute_input_output_redirection(char **tokens, struct exec_context ec);
int execute_background_execution(char **tokens, struct exec_context ec);
int execute_pipe(char **tokens, struct exec_context ec);


int main() {

    bool running = true;
    
    while(running == true) {
        
        // initialize line and tokens array
        char line[max_length];

        char **tokens = calloc(max_size, sizeof(char *));        
        
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

        free(tokens);
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

    ec = is_io(tokens, ec);

    //print_exec_context(ec);

    // if there is redirection
    if(ec.contains_io == true) {
        execute_IO(tokens, ec);
        return 1;
    
    // if there is no redirection, this is either a built in or a regular command
    } else {
        if(is_built_in(tokens[0])) {
            execute_built_in(tokens);
            return 1;
        } else {
            execute_command(tokens);
            return 1;
        }
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

//returns a struct of the execution context
struct exec_context is_io(char **tokens, struct exec_context ec) {
	
	for(size_t i = 0; tokens[i] != NULL; i++) {
		
        // if there is input redirection
		if (strcmp(tokens[i], "<") == 0) {
			if (i == 0) { // if this is the first token there is an error
				ec.contains_error = true;
				return ec;
            }

            ec.contains_io = true;
            ec.input_redirection = true;
            ec.input_file = tokens[i+1];
        }

        // if there is output redirection
        if (strcmp(tokens[i], ">") == 0) {
            if (i == 0) { // if this is the first token there is an error
                ec.contains_error = true;
                return ec;
            }

            ec.contains_io = true;
            ec.output_redirection = true;
            ec.output_file = tokens[i+1];
        }
        
        // if there is appending output redirection
        if (strcmp(tokens[i], ">>") == 0) { 
            if (i == 0 ) { // if this is the first token there is an error
                ec.contains_error = true;
                return ec;
            }

            ec.contains_io = true;
            ec.output_redirection_append = true;
            ec.output_file = tokens[i+1];

        }

        if (strcmp(tokens[i], "|") == 0) {
            // check for errors: pipe must have a command on either side
            if (i == 0) {
                ec.contains_error = true;
                return ec;
            }
            if (tokens[i+1]  == NULL) {
                ec.contains_error = true;
                return ec;
            }

            ec.contains_io = true;
            ec.contains_pipe = true;
            ec.pipe_command_1 = tokens[i-1];
            ec.pipe_command_2 = tokens[i+1];
        }

        if (strcmp(tokens[i], "&") == 0) {
            ec.contains_io = true;
            ec.background_execution = true;
        }

    } // end for loop
    return ec;
}

void print_exec_context(struct exec_context ec) {

    printf("ec.contains_io: %d\nec.contains_error: %d\nec.input_redirection: %d\nec.output_redirection: %d\nec.output_redirection_append: %d\nec.contains_pipe: %d\nec.background_execution: %d\n", ec.contains_io, ec.contains_error, ec.input_redirection, ec.output_redirection, ec.output_redirection_append, ec.contains_pipe, ec.background_execution);
	
	printf("input_file: %s\noutput_file: %s\npipe_command_1: %s\npipe_command_2: %s\n", ec.input_file, ec.output_file, ec.pipe_command_1, ec.pipe_command_2);


}


int execute_built_in(char **tokens) {

    if(strcmp(tokens[0], "cd") == 0) {
        execute_cd(tokens[1]); // pass the argument as the new directory

    } else if(strcmp(tokens[0], "dir") == 0) {
        execute_dir(tokens);

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
int execute_dir(char **tokens) {

	// include additional if statements for output redirection
	// make sure to check for errors

    char *dirName = NULL;
    //if no directory is given use current directory (make sure to free after)
    if(tokens[1] == NULL) { // if no directory was specified, use cwd
         dirName = getcwd(dirName, 0);
         
    } else {
        dirName = tokens[1];
    }

    // check for error
    if(dirName == NULL) {
        return 0;
    }

    DIR *dir = opendir(dirName); // error-check this 

    // check for error
    if(dir == NULL) {
        return 0;
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

// accepts tokens array as a parameter, prints the tokens after echo
void execute_echo(char **tokens) {
	for(size_t i = 1; tokens[i] != NULL; i++) {
        printf("%s ", tokens[i]);
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

// executes bash commands, returns 1 if successful
int execute_IO(char **tokens, struct exec_context ec) {

    // if there is only output redirection
    if(ec.output_redirection == true && ec.input_redirection == false) {
        execute_output_redirection(tokens, ec);

    // if there is only input redirection
    } else if(ec.input_redirection == true && ec.output_redirection == false) {
        execute_input_redirection(tokens, ec);

    // if there is input and output redirection  
    } else if(ec.input_redirection == true && ec.output_redirection == true) {
        execute_input_output_redirection(tokens, ec);
    
    // if there is background execution
    } else if (ec.background_execution == true) {
        execute_background_execution(tokens, ec);

    // if there is a pipe
    } else if (ec.contains_pipe == true) {
        execute_pipe(tokens, ec);
    }

}

int execute_output_redirection(char **tokens, struct exec_context ec) {

    // null terminate the array
    for(size_t i = 0; tokens[i] != NULL; i++) {
        if((strcmp(tokens[i], ">") == 0)) {
            tokens[i] = NULL;
            break;
        }
    }

    int pid;

    if ((pid = fork()) < 0) {  
        printf("fork failed");
        exit(1);
    }
    else if (pid == 0) { // fork successful, this is the child

        // create output file    
        int outFile = open(ec.output_file, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU | S_IRWXG| S_IRWXO);

        //replace stdout with the specified output file
        dup2(outFile, 1);
        close(outFile);

        execvp(tokens[0], tokens);  // execute command
        printf("exec failed"); // exec should not return
        exit(1);

    } else { // parent                              
        wait(NULL); // wait
    }
}


int execute_input_redirection(char **tokens, struct exec_context ec) {

    // null terminate the array
    for(size_t i = 0; tokens[i] != NULL; i++) {
        if((strcmp(tokens[i], "<") == 0)) {
            tokens[i] = NULL;
            break;
        }
    }

    int pid;

    if ((pid = fork()) < 0) {  
        printf("fork failed");
        exit(1);
    }
    else if (pid == 0) { // fork successful, this is the child

        // create input file 
        int inFile = open(ec.input_file, O_RDONLY);

        //replace stdin with the specified input file
        dup2(inFile, 0);
        close(inFile);

        execvp(tokens[0], tokens);  // execute command
        printf("exec failed"); // exec should not return
        exit(1);

    } else { // parent                              
        wait(NULL); // wait
    }

}

int execute_input_output_redirection(char **tokens, struct exec_context ec) {

    // null terminate the array
    for(size_t i = 0; tokens[i] != NULL; i++) {
        if((strcmp(tokens[i], "<") == 0)) {
            tokens[i] = NULL;
            break;
        }
    }

    int pid;

    if ((pid = fork()) < 0) {  
        printf("fork failed");
        exit(1);
    }
    else if (pid == 0) { // fork successful, this is the child

        // create input and output files
        int inFile = open(ec.input_file, O_RDONLY);
        int outFile = open(ec.output_file, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU | S_IRWXG| S_IRWXO);

        // replace stdin and stdout
        dup2(inFile, 0); 
        dup2(outFile, 1); 

        // close duplicate file descriptors
        close(inFile); 
        close(outFile);

        execvp(tokens[0], tokens);  // execute command
        printf("exec failed"); // exec should not return
        exit(1);

    } else { // parent                              
        wait(NULL); // wait
    }

}

int execute_background_execution(char **tokens, struct exec_context ec) {

    // null terminate the array
    for(size_t i = 0; tokens[i] != NULL; i++) {
        if((strcmp(tokens[i], "&") == 0)) {
            tokens[i] = NULL;
            break;
        }
    }

    int pid;

    if ((pid = fork()) < 0) {  
        printf("fork failed");
        exit(1);

    } 
    if (pid == 0) { // fork successful, this is the child
        execvp(tokens[0], tokens);  // execute command
        printf("exec failed");
        exit(1);
    }
    
    return 1;

    // don't wait here
}

int execute_pipe(char **tokens, struct exec_context ec) {

    // create two separate arrays, one for each process
    char **command1, **command2;

    int i;
    while(strcmp(tokens[i], "|") != 0) {
        command1[i] = tokens[i];
        i++;
    }
    command1[i] = NULL;

    int j;
    for(j = i+1; tokens[j] != NULL; j++) {
        command2[j] = tokens[j];
    } 
    command2[j+1] = NULL;

    for(size_t i = 0; command1[i] != NULL; i++) {
        printf("%s ", command1[i]);
    }

    for(size_t i = 0; command2[i] != NULL; i++) {
        printf("%s ", command2[i]);
    }

    exit(0);


    int pid1 = -1; // pid of first child process
    int pid2 = -1; // pid of second child process

    // an array containing the input and output file descriptors
    // pipe_file_descs[0] -> read from this
    // pipe_file_descs[1] -> write to this 
    int fd[2];

    // create pipe
  if (pipe(fd) == -1){
      printf("\ncould not create pipe");
      return 0;
  }

  // fork first child 
  if ((pid1 = fork()) < 0){
    perror("fork 1 failed!\n");
    return 0;
  }

  if(pid1 == 0) { // success, this is the first child
    // close read side of pipe
    close(fd[0]);
    // redirect stdout to write side of pipe
    dup2(fd[1], 1);
    close(fd[1]);
    // exec program 1
    if (execvp(command1[0], command1) < 0){
      perror("Could not execute command 1");
      exit(-1);
    }
  }



}











