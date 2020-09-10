Grace Sobrinski, tuk01723, TUID: 915693998
3207 Project 2: Creating a Linux Shell
Section 004

This is a simulation of a Linux shell.  A shell is a user interface that will
allow you to access this system using some basic commands.  Notice that every
time you hit enter, the shell displays a line of text. The name displayed 
before the colon is the name of the computer, and the information after the
colon shows the current working directory, or where you are currently 
located in the computer's file system. The symbol at the end is called the 
prompt, and this is where you will enter commands. This shell allows
for two types of commands: built-in and external. This shell includes eight
built-in commands.  'cd' allows you to change the current working directory.
To go back a folder, type 'cd ..'. To go forward a folder, type 'cd name', name
being the folder you would like to enter. 'dir' will list all the files and
folders contained in a specific directory.  For example, you can type 'dir /'
to see all files and folders in the system, or you can just type 'dir' and 
you will see only the current working directory.  'environ' will display 
a few of the important environment variables. Environment variables are 
basically changeable values that describe the environment in which the programs
will run. 'clr' will clear the screen. 'pause' will suspend the shell until
you hit enter. 'echo' will print out whatever you pass into it. For example,
you can type 'echo hello world', and see 'hello world' printed by the shell.
Type 'quit' to exit the shell, or type 'help' to see this guide again.  As for
the external commands, there are many possibilities. If you type 'ls' into the
prompt, you can see all the files and folders that are located in the folder
you are currently in.  If you would like to see the contents of one of these
files, you can type 'cat name', name being the file you would like to see, and
the file will be printed onto the screen.  This shell also allows you to 
combine commands with files, or commands with other commands.  For example, if
you would like the contents of file1 to be entered into file2, you could type
'cat file1 > file2'.  If file2 does not exist, it will be created.  If it does
exist, then anything already in it will be deleted and overwritten with file1, 
so be careful! If you don't want to delete everything in file2  but would 
rather append to it, type 'cat file1 >> file2' instead. You can also write to a
file with four of the built-in commands: 'dir', 'environ', 'echo', and
'help'.  Additionally, you can use a file as input.  Your example, if you would
like to check if file1 contains the word 'hello', you can type
'grep hello < file1'.  The shell will then show you any lines in file1 that
contain the word you're searching for. If you would like to run a shell command
without waiting for it to complete, type the command name with an '&' symbol 
at the end.  This is called background execution.  Finally, you can even use
one command's output as another command's input. This is done through a process
called 'piping'. For example, if you would like to see how many characters
make up the names of the files and folders in your current location, you can
type 'ls | wc'.  Note that if you enter a command that does not exist, or if
you include a redirection symbol but don't specify a file, you will receive an
error.  Enjoy the shell!

