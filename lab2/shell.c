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
#define max_length 1000

// struct containing bash command execution context
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

// input line commands:
void getInput(char* line);
int get_file_input(char *line);
int tokenize(char* line, char** tokens);
void printShell();

// parsing functions:
int parse(char **tokens, struct exec_context ec);
int is_built_in(char *token);
struct exec_context is_io(char **tokens, struct exec_context ec);
void print_exec_context(struct exec_context ec);

// executing commands:
int execute_command(char **tokens);
int execute_built_in(char **tokens);
int execute_IO(char **tokens, struct exec_context ec);

// built in commands:
void execute_cd(char *directory);
int execute_dir(char **tokens);
void execute_environ();
void execute_clr();
void execute_pause();
void execute_echo(char **tokens);
void execute_quit(char **tokens);
void execute_help();

// bash commands:
int execute_output_redirection(char **tokens, struct exec_context ec);
int execute_input_redirection(char **tokens, struct exec_context ec);
int execute_input_output_redirection(char **tokens, struct exec_context ec);
int execute_background_execution(char **tokens, struct exec_context ec);
int execute_pipe(char **tokens, struct exec_context ec);
int execute_output_redirection_append(char **tokens, struct exec_context ec);

// initialize batch mode file
FILE *batch;

int main(int argc, char *argv[]) {

    bool batch_mode = false;

    // check if a file was added to ./myshell
    if(argc == 2) {
        batch = fopen(argv[1], "r"); // open the input file in read mode

        if (batch == NULL) { // check that the file exists
            puts("ERROR: file does not exist");
            exit(0);
        }
        batch_mode = true;
    }

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

        // if batch mode is true read lines from the file
        if(batch_mode == true) {
            // if this is EOF
            if(get_file_input(line) == -1) {
                puts("EOF");
                execute_quit(tokens);
            }
        
        // else take user input
        } else {
            getInput(line);
        }

        if(line[0] == '\0') { // fix this
            break; // no input given
        } else {
            tokenize(line, tokens);
            if(!parse(tokens, ec)) {
                puts("ERROR");
            }
        }

        free(tokens);
    }

}

// gets the command from user input
void getInput(char* line) {
    
    printShell(); // print the current directory

    fgets(line, max_length, stdin); 

}

// gets the command from the batch file, returns -1 if EOF
int get_file_input(char *line) {

    printShell(); // print the current directory

    size_t max = max_length;
    int c = getline(&line, &max, batch);

    return c;

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

    if(ec.contains_error == true) {
        return 0; // error with command input
    }

    // first check for built ins
    if(is_built_in(tokens[0])) {
        if(!execute_built_in(tokens)) {
            return 0; // error occurred
        } else {
            return 1; //success
        }
    
    // next check for bash commands
    } else if (ec.contains_io == true) {
        if(!execute_IO(tokens, ec)) {
            return 0; // error occurred
        } else {
            return 1; // success
        }
    
    // else this is a regular command
    } else {
        if(!execute_command(tokens)) {
            return 0; // error occurred
        } else {
            return 1; // success
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
            } else if(tokens[i+1] == NULL) { // if no file is specified there is an error
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
            } else if(tokens[i+1] == NULL) { // if no file is specified there is an error
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
            } else if(tokens[i+1] == NULL) { // if no file is specified there is an error
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
    
    int result = 1;

    if(strcmp(tokens[0], "cd") == 0) {
        execute_cd(tokens[1]); 
        return 1;

    } else if(strcmp(tokens[0], "dir") == 0) { // must support output redirection

        char *file_name; // this will be the output file

        for(size_t i = 0; tokens[i] != NULL; i++) {

            // output redirection to file 
            if(strcmp(tokens[i], ">") == 0) {
                if(tokens[i+1] == NULL) {
                    return 0; // error
                } else {
                    file_name = tokens[i+1]; // save output file name
                    tokens[i] = NULL; // null terminate the array
                }

                int duplicate = dup(1); 
                int outFile = open(file_name, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU | S_IRWXG| S_IRWXO); // create the output file
                dup2(outFile, 1);
                close(outFile);

                result = execute_dir(tokens); // when dir is called, its output will go into the outFile
            
                dup2(duplicate, 1);
                close(outFile);
                return result;

            // output redirection appending to file
            } else if(strcmp(tokens[i], ">>") == 0) {
                if(tokens[i+1] == NULL) {
                    return 0; // error
                } else {
                    file_name = tokens[i+1]; // save output file name
                    tokens[i] = NULL; // null terminate the array
                }

                int duplicate = dup(1); 
                int outFile = open(file_name, O_WRONLY|O_CREAT|O_APPEND, S_IRWXU | S_IRWXG| S_IRWXO); // create the output file
                dup2(outFile, 1);
                close(outFile);
                
                result = execute_dir(tokens);
                dup2(duplicate, 1);
                close(outFile);
                return result;

            }
        }

        // if no redirection, call dir normally
        result = execute_dir(tokens);

        return result;

    } else if(strcmp(tokens[0], "environ") == 0) { // must support output redirection
        
        char *file_name; // this will be the output file

        for(size_t i = 0; tokens[i] != NULL; i++) {

            // output redirection to file 
            if(strcmp(tokens[i], ">") == 0) {
                if(tokens[i+1] == NULL) {
                    return 0; // error
                } else {
                    file_name = tokens[i+1]; // save output file name
                    tokens[i] = NULL; // null terminate the array
                }

                int duplicate = dup(1); 
                int outFile = open(file_name, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU | S_IRWXG| S_IRWXO); // create the output file
                dup2(outFile, 1);
                close(outFile);

                execute_environ(); // when environ is called, its output will go into the outFile

                dup2(duplicate, 1);
                close(outFile);
                return 1;

            // output redirection appending to file
            } else if(strcmp(tokens[i], ">>") == 0) {
                if(tokens[i+1] == NULL) {
                    return 0; // error
                } else {
                    file_name = tokens[i+1]; // save output file name
                    tokens[i] = NULL; // null terminate the array
                }

                int duplicate = dup(1); 
                int outFile = open(file_name, O_WRONLY|O_CREAT|O_APPEND, S_IRWXU | S_IRWXG| S_IRWXO); // create the output file
                dup2(outFile, 1);
                close(outFile);

                execute_environ(); // when echo is called, its output will go into the outFile

                dup2(duplicate, 1);
                close(outFile);
                return 1;

            }
        }

        // if no redirection, call environ normally
        execute_environ();
        return 1;

    } else if(strcmp(tokens[0], "clr") == 0) {
        execute_clr();

    } else if(strcmp(tokens[0], "pause") == 0) {
        execute_pause();
        return 1;

    } else if(strcmp(tokens[0], "echo") == 0) { // must support output redirection

        char *file_name; // this will be the output file

        for(size_t i = 0; tokens[i] != NULL; i++) {

            // output redirection to file 
            if(strcmp(tokens[i], ">") == 0) {
                if(tokens[i+1] == NULL) {
                    return 0; // error
                } else {
                    file_name = tokens[i+1]; // save output file name
                    tokens[i] = NULL; // null terminate the array
                }

                int duplicate = dup(1); 
                int outFile = open(file_name, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU | S_IRWXG| S_IRWXO); // create the output file
                dup2(outFile, 1);
                close(outFile);
                execute_echo(tokens); // when echo is called, its output will go into the outFile
                dup2(duplicate, 1);
                close(outFile);
                return 1;

            // output redirection appending to file
            } else if(strcmp(tokens[i], ">>") == 0) {
                if(tokens[i+1] == NULL) {
                    return 0; // error
                } else {
                    file_name = tokens[i+1]; // save output file name
                    tokens[i] = NULL; // null terminate the array
                }

                int duplicate = dup(1); 
                int outFile = open(file_name, O_WRONLY|O_CREAT|O_APPEND, S_IRWXU | S_IRWXG| S_IRWXO); // create the output file
                dup2(outFile, 1);
                close(outFile);
                execute_echo(tokens); // when echo is called, its output will go into the outFile
                dup2(duplicate, 1);
                close(outFile);
                return 1;

            }
        }

        // if no redirection, call echo normally
        execute_echo(tokens);
        return 1;

    } else if(strcmp(tokens[0], "quit") == 0) {
        execute_quit(tokens);
        return 1;

    } else if(strcmp(tokens[0], "help") == 0) { // must support output redirection

        char *file_name; // this will be the output file

        for(size_t i = 0; tokens[i] != NULL; i++) {

            // output redirection to file 
            if(strcmp(tokens[i], ">") == 0) {
                if(tokens[i+1] == NULL) {
                    return 0; // error
                } else {
                    file_name = tokens[i+1]; // save output file name
                    tokens[i] = NULL; // null terminate the array
                }

                int duplicate = dup(1); 
                int outFile = open(file_name, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU | S_IRWXG| S_IRWXO); // create the output file
                dup2(outFile, 1);
                close(outFile);
                execute_help(); // when help is called, its output will go into the outFile
                dup2(duplicate, 1);
                close(outFile);
                return 1;

            // output redirection appending to file
            } else if(strcmp(tokens[i], ">>") == 0) {
                if(tokens[i+1] == NULL) {
                    return 0; // error
                } else {
                    file_name = tokens[i+1]; // save output file name
                    tokens[i] = NULL; // null terminate the array
                }

                int duplicate = dup(1); 
                int outFile = open(file_name, O_WRONLY|O_CREAT|O_APPEND, S_IRWXU | S_IRWXG| S_IRWXO); // create the output file
                dup2(outFile, 1);
                close(outFile);
                execute_help(); // when help is called, its output will go into the outFile
                dup2(duplicate, 1);
                close(outFile);
                return 1;

            }
        }

        // if no redirection, call help normally
        execute_help();
        return 1;
    }

}

// built in functions

//accepts new directory as parameter
void execute_cd(char *directory) {
	//should also change the PWD environment variable
    chdir(directory);

}

//accepts new directory as parameter, returns 1 if successful
int execute_dir(char **tokens) {

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
        fflush(stdout);
    }

    return 1; // success
}

// print environment variables
void execute_environ() {
	
	printf("\nPATH: %s\n", getenv("PATH"));
    fflush(stdout);
    printf("\nUSER: %s\n", getenv("USER"));
    fflush(stdout);
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
        printf("%s ",tokens[i]);
        fflush(stdout);
    }
	
}

void execute_quit(char **tokens) {
	//do resource cleanup
    free(tokens);
	exit(0);
}

// prints the user manual
void execute_help() {
	
	printf("\nuser manual goes here");
    fflush(stdout);
}

// execute commands with no I/O redirection, returns 0 if error occurs
int execute_command(char **tokens) {

    int pid;

    if ((pid = fork()) < 0) {  
        return 0; // error, fork failed
    }
    else if (pid == 0) { // fork successful, this is the child
        execvp(tokens[0], tokens);  // execute command
        return 0; // error, exec failed

    } else { // parent                              
        wait(NULL); // wait
    }

    return 1; // success!
}

// executes bash commands, returns 1 if successful
int execute_IO(char **tokens, struct exec_context ec) {

    // if there is only output redirection
    if(ec.output_redirection == true && ec.input_redirection == false) {
        if(!execute_output_redirection(tokens, ec)) {
            return 0; // error
        } else {
            return 1;
        }

    // if there is only input redirection
    } else if(ec.output_redirection_append == true) {
        if(!execute_output_redirection_append(tokens, ec)) {
            return 0; // error
        } else {
            return 1;
        }

    // if there is only input redirection
    } else if(ec.input_redirection == true && ec.output_redirection == false) {
        if(!execute_input_redirection(tokens, ec)) {
            return 0; // error
        } else {
            return 1;
        }

    // if there is input and output redirection  
    } else if(ec.input_redirection == true && ec.output_redirection == true) {
        if(!execute_input_output_redirection(tokens, ec)) {
            return 0; //error
        } else {
            return 1;
        }
    
    // if there is background execution
    } else if (ec.background_execution == true) {
        if(!execute_background_execution(tokens, ec)) {
            return 0; //error
        } else {
            return 1;
        }

    // if there is a pipe
    } else if (ec.contains_pipe == true) {
        if(!execute_pipe(tokens, ec)) {
            return 0; // error
        } else {
            return 1;
        }
    }

}

// command > file
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
        return 0; // error, fork failed
    }
    else if (pid == 0) { // fork successful, this is the child

        // create output file    
        int outFile = open(ec.output_file, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU | S_IRWXG| S_IRWXO);

        //replace stdout with the specified output file
        dup2(outFile, 1);
        close(outFile);

        execvp(tokens[0], tokens);  // execute command
        return 0; // error, exec failed

    } else { // parent                              
        wait(NULL); // wait
    }

    return 1; //success
}

// command >> file
int execute_output_redirection_append(char **tokens, struct exec_context ec) {

    // null terminate the array
    for(size_t i = 0; tokens[i] != NULL; i++) {
        if((strcmp(tokens[i], ">>") == 0)) {
            tokens[i] = NULL;
            break;
        }
    }

    int pid;

    if ((pid = fork()) < 0) {  
        exit(1);
        return 0; //error, fork failed
    }
    else if (pid == 0) { // fork successful, this is the child

        // create output file    
        int outFile = open(ec.output_file, O_WRONLY|O_CREAT|O_APPEND, S_IRWXU | S_IRWXG| S_IRWXO);

        //replace stdout with the specified output file
        dup2(outFile, 1);
        close(outFile);

        execvp(tokens[0], tokens);  // execute command
        exit(1);
        return 0; //error, exec failed

    } else { // parent                              
        wait(NULL); // wait
    }

    return 1; // success
}

// command < file
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
        exit(1);
        return 0; //error, fork failed
    }
    else if (pid == 0) { // fork successful, this is the child

        // create input file 
        int inFile = open(ec.input_file, O_RDONLY);

        //replace stdin with the specified input file
        dup2(inFile, 0);
        close(inFile);

        execvp(tokens[0], tokens);  // execute command
        exit(1);
        return 0; //error, exec failed

    } else { // parent                              
        wait(NULL); // wait
    }

    return 1; //success

}

// command < file > file
// returns 1 if successful
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
        exit(1);
        return 0; //error, fork failed
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
        exit(1);
        return 0; //error, exec failed

    } else { // parent                              
        wait(NULL); // wait
    }

    return 1; // success

}

// command &
// returns 1 if successful;
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
        exit(1);
        return 0; // error, fork failed

    } 
    if (pid == 0) { // fork successful, this is the child
        execvp(tokens[0], tokens);  // execute command
        exit(1);
        return 0; //error, exec failed
    }
    
    return 1;

    // don't wait here
}

// command | command
int execute_pipe(char **tokens, struct exec_context ec) {

    // create two separate arrays, one for each process
    char **command1 = calloc(max_size, sizeof(char *));        
    char **command2 = calloc(max_size, sizeof(char *));        

    int i;
    for(i = 0; tokens[i] != NULL; i++) {
        if(strcmp(tokens[i], "|") == 0) {
            break;
        } else {
            command1[i] = tokens[i];
        }
    }
    i++;

    int j = 0;
    while(tokens[i] != NULL) {
        command2[j] = tokens[i];
        i++;
    }


    // create process id for two children
    int pid1 = -1; 
    int pid2 = -1; 

    // input/ouput file descriptors
    int fd[2];

    // create pipe
    if (pipe(fd) == -1){
        return 0; // error, pipe failed to create
    }

    // command1
    if ((pid1 = fork()) < 0){
        return 0; // error, fork did not create
    }

    if(pid1 == 0) { // success, this is the first child
        // close read side of pipe
        close(fd[0]);
        // redirect stdout to write side of pipe
        dup2(fd[1], 1);
        close(fd[1]);

        // call first command
        execvp(command1[0], command1);

        // if this point is reached an error occurred
        return 0; 
    
    }

    // command2
    if ((pid2 = fork()) < 0){
        return 0; // error, fork failed
    }

    if(pid2 == 0) { // success, child 2 is created
        // close write side of pipe
        close(fd[1]);
        // redirect stdin to read side of pipe
        dup2(fd[0], 0);
        close(fd[0]);

        // call command 2
        execvp(command2[0], command2);

        // if this point is reached an error occurred
        return 0;
    
    }

    // -1 signifies to wait for ALL children to finish
    waitpid(-1, NULL, 0);

    close(fd[0]);
    close(fd[1]);

    // free memory used
    free(command1);
    free(command2);

    return 1; //success!

}











