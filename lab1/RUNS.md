# RUNS file

**Run 1:**
SEED 1
INIT_TIME 0
FIN_TIME 15000
ARRIVE_MIN 5
ARRIVE_MAX 20
QUIT_PROB 20
NETWORK_PROB 30
CPU_MIN 5
CPU_MAX 10
DISK1_MIN 20
DISK1_MAX 30
DISK2_MIN 20
DISK2_MAX 30
NETWORK_MIN 50
NETWORK_MAX 100

These are the CONFIG file values I used while creating my STATS file. I noticed
that the CPU queue was consistently much larger than the disk queues, and the
disk queues were larger than the network queue. The average response time for
each component fell pretty much exactly in between its MIN and MAX. As
expected, because every single process must pass through the CPU, its
throughput was highest. The disks were nearly identical for all of the
statistics. Although the CPU was used most frequently, its utilization was
still just slightly less than the disks, because its MIN and MAX times are much
less. In 15000 time units, 1178 processes entered the system and 10907 jobs
were performed.

**Run 2:**
SEED 1
INIT_TIME 0
FIN_TIME 15000
ARRIVE_MIN 5
ARRIVE_MAX 10
QUIT_PROB 20
NETWORK_PROB 30
CPU_MIN 5
CPU_MAX 10
DISK1_MIN 9
DISK1_MAX 15
DISK2_MIN 9
DISK2_MAX 15
NETWORK_MIN 50
NETWORK_MAX 100

For this run, I used what was specified in the project description for
QUIT_PROB and NET_PROB. From what I read, the average time to access a disk is
9-15 seconds, so I set the disks MIN and MAX at 9 and 15 respectively. I also
lowered the ARRIVE_MAX. I was kind of surprised by how drastically the
statistics were affected by lowering the disk values. In lessening the time
processes spent in the disks, the maximum disk queue size dropped from 105 to
just 1. Because of the lower disk and arrival times, the CPU was also affected.
Its average queue size was 1617, so it was probably holding up everything else.
Lowering arrival times also allowed for many more processes to enter the
system. In total, 2012 processes entered the system and 13300 jobs were
performed.

**Run 3:**
SEED 10
INIT_TIME 0
FIN_TIME 5000
ARRIVE_MIN 5
ARRIVE_MAX 10
QUIT_PROB 20
NETWORK_PROB 30
CPU_MIN 3
CPU_MAX 5
DISK1_MIN 10
DISK1_MAX 20
DISK2_MIN 15
DISK2_MAX 30
NETWORK_MIN 25
NETWORK_MAX 50

For this run, I tried lowering the CPU times and increasing the disk times, to
see if it would even out the utilization, and it seemed to work. The
utilization values for the CPU, Disk 1, and Disk 2 were nearly identical. Also,
the CPU queue was consistently smaller than the disk queues. The network queue
was still very small. In 5000 time units, 667 processes entered the system and
6134 jobs were performed.
