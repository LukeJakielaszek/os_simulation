# os_simulation
A simulation of an operating system using a heap to store processes and
fifo queues to store pending jobs. This simulates jobs/processes cycling
through the cpu and local disks.

log.txt:
	A simple log file which stores seeding information, important events,
	and statistical information.

config.txt:
	A configuration file used to initialize seeding constants of the
	simulation.

read_config.c:
	Reads a config files specified constants. These constants are returned
	as an allocated array and are used to initialize the main program's
	seeded values.

queue.c:
	A Fifo queue which stores job IDs as integers. This file has general
	functions related to fifo queues such as enqueue and dequeue. There
	is a count_queue function which runs in o(n) time, counting nodes one
	by one. A future optimization of this would be to have a struct
	queue which stores the root qnode and the size of the queue. Therefore,
	no extra space would be needed and obtaining the size of the queue
	would be o(1).

pqueue.c:
	An array implementation of a heap which stores job id, type, time,
	heap size, and the max length of the heap. The is implemented like an
	array_list, doubling size when max heap size = the current heap size.
	The root index is a burner index to track the size and max length of
	the heap, making getting size of the heap o(1). Sorting when pushing
	and popping from the heap uses the heap_sort algorithm, with 2n and
	2n+1 to access sub_trees. An optimiztion for this heap would be
	similar to the queue, whereas there is a container struct, heap,
	which stores size and max_length. Access time for size and max_length
	would still be o(1), but there would be no wasted space as every hnode
	of the current implementation of the heap wastes 8 bytes (2 ints) to
	store size and max_length.

main.c:
	The constructed simulation. The CPU, Disk1, and Disk2, are all
	represented as subsystem structs, which contain links to their
	corresponding queues, track occupancy, and hold constants pertaining
	to their type. Each subsystem also holds an init_time tracker queue
	that is used to calculate response times of subsystems. It would have
	been more efficient to simply have each subsystem's fifo queue track
	both job id and job visit time, but I saw the response time statistical
	calculation after the entire simulation was already coded in and
	changing it would have required altering most of the lab.

	The main simulation is ran through a sort of game-loop where an event
	is popped from the heap and processed based on its event type and
	corresponding handler function. This loop is always true and the
	simulation can only be exited when the SIM_FIN event is removed from
	the heap.

	If a JOB_ENTERS event is removed from the heap, the job is either
	put directly on the CPU if the CPU is free or appended to the CPU's
	queue if it is occupied. If a job is placed directly into the CPU, a
	JOB_FIN_CPU event is created and added to the heap. A new job is also
	created and added to the heap with JOB_ENTERS as type in order to
	grow the simulation at a linear rate.

	If a JOB_FIN_CPU event is removed from the heap, the can_exit function
	is called, testing whether the job is ready to leave the program with
	probability read from the config file. If the job fails to leave the
	program, the job_arrive_disk function is called, which adds the job to
	an unoccupied disk, or if both are occupied, to the shortest disk
	queue. A Job_FIN_D* event is the created and added to the heap where *
	is the corresponding disk number.

	If a JOB_FIN_D1 OR JOB_FIN_D2 event are popped from the heap, the
	current job at the selected disk is sent to the CPU in the same way a
	job is sent to the CPU when the JOB_ENTERS event is popped from the
	heap. The jobs corresponding disk subsystem then looks to its queue
	for the next job. If it does not find one, it sits in idle. If it
	does find one, it creates a new JOB_FIN_D* event and appends
	it to the heap.

	If a SIM_FIN event is popped from the heap, statistics are calculated
	from current simulation tracking items and are appended to the log
	file. Allocated memory is also freed and the log_file is finally
	closed.

Testing:
	The program was tested throughout development in the following order

	    (1) reading config file
	    (2) constructing fifo queue
	    (3) constructing heap
	    (4) constructing subsystem structs
	    (5) converting QUIT_PROB to a usable form
	    (6) Developing SIM_FIN handling
	    (7) developing JOB_ENTERS handling
	    (8) developing JOB_FIN_CPU handling
	    (9) developing JOB_FIN_D1 and JOB_FIN_D2 handling
	    (10) logging events to log_file
	    (11) tracking statistical calculations
	    (12) logging statistics to log_file

	 Each handler function was checked for correctness with periodic
	 print statements and then final walkthroughs in the log file with edge
	 cases such as 0 and 1 QUIT_PROB. Statistical calculations were
	 checked for correctness by also testing edge cases such as 0 and 1
	 QUIT_PROB, by walking through events in the log_file, and by
	 calculating expected values directly from the config file.
	    e.g.
	    Simulation :
	    	   CPU Throughput = 22.35 jobs per 100 ms
	    Config File :
	    	   CPU_MIN = 2
		   CPU_MAX = 9
		   Av. CPU time = (9+2)/2 = 4.5 ms per job
		   100 ms / 4.5 ms per job = 22.222222 jobs

  	    The expected throughput (22.22222) is very close to the
	    simulated throughput (22.35). Therefore throughput appears
	    to be calculating correctly for the set config values.