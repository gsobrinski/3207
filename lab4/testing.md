Grace Sobrinski
3207 Project 4: Signals
Section 004

Testing

In program 1, The signal generator sent slightly more SIGUSR1 signals out,
which was not fully consistent with the results of the signal handler and
signal reporter counter values.  The signal handlers received most of these,
but the signal reporter only received about half of them.  Since there were two
signal handlers for each signal type, this means that both the handlers and the
reporter only received approximately half the signals that were sent. 

In program 2, the reporting function received nearly the exact same number as
those that were sent, and the handlers received about twice as many, which is
accurate because there should be 2 handlers incrementing for each signal type. 