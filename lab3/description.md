Grace Sobrinski
3207 Project 3: Networked Spell Checker
Section 004

The spellchecker program first checks for any command line arguments, then
modifies the port number and dictionary file accordingly. The network is 
initialized using the given code, and the init_variables() function initializes
all of the mutexes and condition variables that will be necessary later. 
The spawn_worker_threads() function is called, which will create N_THREADS each
of worker and logger threads. The dictionary is also initialized by reading in
the strings in the given dictionary file and saving them into a global array of
strings. Inside the main while loop, a file descriptor is retrieved and added
to the connection queue.  Inside the worker thread function, a socket
descriptor is removed from the connection queue and sent to the search()
function which returns whether or not the word was found.  The word and the
response are both sent to the log queue to be added into the log file. In the
log thread function, a string is removed from the log queue and added to the
log file. Both the connection and log queues have their own put and get 
functions, along with their own mutexes and condition variables.  In order to
test the program and confirm that mutual exclusion was being implemented 
properly, I tested the example python 3 client with a variety of words.txt
files.  The main things I looked for were the correct number of entries in the
log file, and no repeats or missing entries.