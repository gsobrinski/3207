Grace Sobrinski
3207 Project 2: Creating a Linux Shell
Section 004

This program is a simulation of a Linux shell.  It allows the user to enter 
commands in the format 'command arg1 arg2 ...'' and then executes the command.
It allows for eight built-in commands, as well as external commands.  It also
allows for input/ouput redirection, background execution, and piping. It can
be used like a normal shell where the user enters commands one at a time, or
it can be executed in batch mode, using a file containing commands as an
argument.  When the shell is executed, the first thing it does is check if the
user chose to use batch mode.  If not, the prompt will appear and the user can
begin entering commands.  After retrieving a line of input, the shell sends 
this line to the tokenizer, and it is put into a whitespace delimited array.  
This array is then sent to the parser, which will determine what type of 
command it is and how it will be executed.  The first step the parser takes is
to check the command for any redirection, piping, or background execution, and
it stores this information in an execution context struct.  This also checks
for input errors, such as typing a '<' where the command name should be.  Next,
it checks if the command is built-in, and if yes then the 'execute_built_in'
function is executed. This function calls the corresponding function for the
command in a long if/else statement.  It also allows for output redirection
for dir, echo, environ, and help.  If the command is not built-in and also
contains no special characters, it is sent to the 'execute_command' function
and executed normally using fork() and execvp().  Finally, if the command is
not built-in but it does contain special characters then it is sent to the 
'execute_IO' function, which is also a long if/else statement that will call
the corresponding function depending on what is specified in the command.
All functions that execute commands will return 0 if an error occurs, and
return 1 if successful.  If an error does occur, an appropriate error message
will be printed so the user knows what went wrong.  In order to test my shell
program, I entered many incorrect commands both regularly and also using a
file. To make sure that the error messages were accurate, I first entered the
incorrect command into the bash shell to see what it would output, and then
added that message to my own shell. In my shell, there are several places where
it will check for errors.  The first is when checking the execution context.
If any incorrect syntax is used with the special characters, it prints an
error message and sets the 'contains_error' value to true.  When the struct
is returned, if 'contains_error' is true, then the shell immediately returns
the user back to the command prompt to try again. It also checks for incorrect
syntax in the built-in commands.  Finally, there are error checks in any
function that forks processes.