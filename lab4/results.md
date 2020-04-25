Grace Sobrinski
3207 Project 4: Signals
Section 004

Results/Analysis

In 30 seconds of runtime, program 1's signal total was 5691.  Documenting 
10,000 signals took approximately 1 minute and 20 seconds. The time between
signals for both SIGUSR1 and SIGUSR2 was typically between 0.0 and 0.5 seconds.

Program 2 documented 6411 signals in 30 seconds, while 10,000 signals only took
about 47 seconds. The time between signals was generally between 0.0 and 
0.5 seconds, but most were 0 seconds.  It can be seen that program 2 generates
signals slightly faster than program 1, or that the signals are more 
consistently received. 

The results of program 1, which used multiple processes, were much less 
consistent than those of program 2, which used multithreading. Program 1 was 
done asynchronously using signal() to register a signal handler while program 2
was done synchronously using sigwait().  