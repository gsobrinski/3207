Grace Sobrinski
3207 Project 3: Networked Spell Checker
Section 004

This program is a multithreaded spell checker.  A number of clients can connect
simultaneously, up to a maximum of N_THREADS.  The clients connect by typing nc
along with the IP and port number. Once connected, any client can enter a
word and the program will inform you whether or not it is contained in the
dictionary.  The spellchecker defaults to using dictionary.txt and port number
8888, but one or both of these can be modified by adding them as command line
arguments. As the clients enter words into the spellchecker, the program will
log all entries into the log.txt file, along with an indicator that tells you
if they were found in the dictionary.