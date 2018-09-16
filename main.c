#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "pqueue.h"
#include "read_config.h"

// define event type constants
#define JOB_ENTERS 0
#define JOB_FIN_CPU 1
#define JOB_FIN_D1 2
#define JOB_FIN_D2 3
#define SIM_FIN 4

// global job counter
int num_jobs = 1;

// cpu statistics
int cpu_q_count = 0;
int cpu_q_sum = 0;
int cpu_q_max = 0;
int cpu_busy = 0;
int cpu_count = 0;
int cpu_response = 0;
int cpu_max_response = 0;

// disk1 statistics
int d1_q_count = 0;
int d1_q_sum = 0;
int d1_q_max = 0;
int d1_busy = 0;
int d1_count = 0;
int d1_response;
int d1_max_response = 0;

// disk2 statistics
int d2_q_count = 0;
int d2_q_sum = 0;
int d2_q_max = 0;
int d2_busy = 0;
int d2_count = 0;
int d2_response;
int d2_max_response = 0;

// global simulation time counter
int sim_time = 0;
int time_end = 0;

typedef struct sub_system{
  // current job id, -1 if not busy
  int cur_id;

  // 1 if handling process, 0 if not
  int is_busy;

  // tracks type of finish to add to heap
  int fin_type;

  // subsystems corresponding queue
  qnode * queue;

  // min time for a job to process
  int min_time;

  // max time for a job to process
  int max_time;

  // queue to track response times of server
  qnode * init_job_times;
}sub_system;

// prototypes
int gen_rand_int(int min, int max);
int can_exit(double prob);
hnode * job_arrives_cpu(int id, sub_system * sub, hnode * heap,
			FILE * log_file, int arr_min, int arr_max);
sub_system * create_sub(int type, int min_time, int max_time);
int compare_qs(qnode * q1, qnode * q2);
hnode * create_job_fin(sub_system * sub, hnode * heap, int min_time,
		       int max_time);
hnode * job_arrive_disk(sub_system * d1, sub_system * d2, hnode * heap,
			int id, FILE * log_file);
hnode * job_finish_disk(sub_system * disk, sub_system * cpu, hnode * heap,
			FILE * log_file);
hnode * job_finish_cpu(sub_system * cpu, sub_system * d1, sub_system * d2,
		       hnode * heap, double quit_prob, int arr_min,
		       int arr_max, FILE * log_file);

// takes in a config file and log file
int main(char argc, char ** argv){
  // opens log file
  FILE *log_file;

  // clears log_file
  log_file = fopen(argv[2], "w");
  
  // prints error if log file fails to open
  if(log_file == NULL){
    printf("ERROR: Failed to open [%s]\n", argv[2]);
    exit(-1);
  }

  // closes log file for open in append
  fclose(log_file);
  
  // opens log file for append
  log_file = fopen(argv[2], "a");

  // prints error if log file fails to open
  if(log_file == NULL){
    printf("ERROR: Failed to open [%s]\n", argv[2]);
    exit(-1);
  }

  // reads config values from txt file
  double * config_vals = read_config(argv[1]);

  // logs success
  fprintf(log_file, "Processed config file [%s]\n", argv[1]);
  
  // initializes config value constants
  const int SEED = (int)config_vals[0];
  const int INIT_TIME = (int)config_vals[1];
  const int FIN_TIME = (int)config_vals[2];
  const int ARRIVE_MIN = (int)config_vals[3];
  const int ARRIVE_MAX = (int)config_vals[4];
  const double QUIT_PROB = config_vals[5];
  const int CPU_MIN = (int)config_vals[6];
  const int CPU_MAX = (int)config_vals[7];
  const int DISK1_MIN = (int)config_vals[8];
  const int DISK1_MAX = (int)config_vals[9];
  const int DISK2_MIN = (int)config_vals[10];
  const int DISK2_MAX = (int)config_vals[11];

  // for tracking time
  time_end = FIN_TIME;
  
  // frees config val array
  free(config_vals);

  // initializes random number generators.
  srand(SEED);

  // prints config file to log file
  fprintf(log_file, "Initialized constants:\n");
  fprintf(log_file, "\tSEED %d\n", SEED);
  fprintf(log_file, "\tINIT_TIME %d\n", INIT_TIME);
  fprintf(log_file, "\tFIN_TIME %d\n", FIN_TIME);
  fprintf(log_file, "\tARRIVE_MIN %d\n", ARRIVE_MIN);
  fprintf(log_file, "\tARRIVE_MAX %d\n", ARRIVE_MAX);
  fprintf(log_file, "\tQUIT_PROB %lf\n", QUIT_PROB);
  fprintf(log_file, "\tCPU_MIN %d\n", CPU_MIN);
  fprintf(log_file, "\tCPU_MAX %d\n", CPU_MAX);
  fprintf(log_file, "\tDISK1_MIN %d\n", DISK1_MIN);
  fprintf(log_file, "\tDISK1_MAX %d\n", DISK1_MAX);
  fprintf(log_file, "\tDISK2_MIN %d\n", DISK2_MIN);
  fprintf(log_file, "\tDISK2_MAX %d\n", DISK2_MAX);
  
  // creates heap for job time ordering
  hnode * heap = create_heap();

  // initializes heap job1 and end time
  heap = push(heap, num_jobs, JOB_ENTERS, INIT_TIME);
  heap = push(heap, -1, SIM_FIN, FIN_TIME);

  // log successful heap
  fprintf(log_file, "Initialized Heap\n");
  
  // initializes simulation timer
  sim_time = INIT_TIME;


  // creates cpu subsystem
  sub_system * cpu = create_sub(JOB_FIN_CPU, CPU_MIN, CPU_MAX);

  // creates disk1 subsystem
  sub_system * disk1 = create_sub(JOB_FIN_D1, DISK1_MIN, DISK1_MAX);

  // creates disk2 subsystem
  sub_system * disk2 = create_sub(JOB_FIN_D2, DISK2_MIN, DISK2_MAX);

  // log successful creation of subsystems
  fprintf(log_file, "Initialized Subsystems\n");

  // log simulation start
  fprintf(log_file, "Running Simulation...\n\n");
  
  // simulation loop
  while(1){
    // gets next process
    hnode * next_process = pop(heap);
    
    // increments sim_time
    sim_time = next_process->time;
    
    // calls handler function based on process type
    if(next_process->type == SIM_FIN){
      fprintf(log_file, "\nSimulation complete %d\n", sim_time);

      fprintf(log_file, "\nStatistical Data\n");

      int throughput_time = 100;
      
      // CPU Statistics
      fprintf(log_file, "\nCPU:\n");
      fprintf(log_file, "\tAverage queue size %lf\n",
	      ((double)cpu_q_sum)/cpu_q_count);      
      fprintf(log_file, "\tMax queue size %d\n", cpu_q_max);
      fprintf(log_file, "\tUtilization %lf\n",
	      ((double)cpu_busy) / (FIN_TIME-INIT_TIME));
      fprintf(log_file, "\tAverage response time %lf seconds\n",
	      ((double)cpu_response) / cpu_count);
      fprintf(log_file, "\tMax response time %d seconds\n", cpu_max_response);
      fprintf(log_file, "\tThroughput: %lf jobs every %d seconds\n",
	      throughput_time * ((double)cpu_count) / (FIN_TIME-INIT_TIME),
	      throughput_time);
      
      // D1 Statistics
      fprintf(log_file, "\nD1:\n");
      fprintf(log_file, "\tAverage queue size %lf\n",
	      ((double)d1_q_sum)/d1_q_count);
      fprintf(log_file, "\tMax queue size %d\n", d1_q_max);
      fprintf(log_file, "\tUtilization %lf\n",
	      ((double)d1_busy) / (FIN_TIME-INIT_TIME));
      fprintf(log_file, "\tAverage response time %lf seconds\n",
	      ((double)d1_response) / d1_count);
      fprintf(log_file, "\tMax response time %d seconds\n", d1_max_response);
      fprintf(log_file, "\tThroughput: %lf jobs every %d seconds\n",
	      throughput_time * ((double)d1_count) / (FIN_TIME-INIT_TIME),
	      throughput_time);
      
      // D2 Statistics
      fprintf(log_file, "\nD2:\n");
      fprintf(log_file, "\tAverage queue size %lf\n",
	      ((double)d2_q_sum)/d2_q_count);
      fprintf(log_file, "\tMax queue size %d\n", d2_q_max);
      fprintf(log_file, "\tUtilization %lf\n",
	      ((double)d2_busy) / (FIN_TIME-INIT_TIME));
      fprintf(log_file, "\tAverage response time %lf seconds\n",
	      ((double)d2_response) / d2_count);
      fprintf(log_file, "\tMax response time %d seconds\n", d2_max_response);
      fprintf(log_file, "\tThroughput: %lf jobs every %d seconds\n",
	      throughput_time * ((double)d2_count) / (FIN_TIME-INIT_TIME),
	      throughput_time);

      
      // frees allocated memory
      free(heap);
      free(cpu);
      free(disk1);
      free(disk2);
      
      // closes log file
      fclose(log_file);
      
      return 0;
    }else if(next_process->type == JOB_ENTERS){
      // logs event
      fprintf(log_file, "Job %d entered simulation at %d\n", next_process->id,
	      next_process->time);

      // handles job arrival event
      heap = job_arrives_cpu(next_process->id, cpu, heap, log_file,
			     ARRIVE_MIN, ARRIVE_MAX);
    }else if(next_process->type == JOB_FIN_CPU){
      // logs event
      fprintf(log_file, "Job %d finished at CPU at %d\n", next_process->id,
	      next_process->time);
      
      //handles job finishes at cpu event
      heap = job_finish_cpu(cpu, disk1, disk2, heap, QUIT_PROB, ARRIVE_MIN,
			    ARRIVE_MAX, log_file);
    }else if(next_process->type == JOB_FIN_D1){
      // logs event
      fprintf(log_file, "Job %d finished at DISK 1 at %d\n", next_process->id,
	      next_process->time);
      
      //adds finished disk1 job to cpu or cpu queue
      heap = job_arrives_cpu(disk1->cur_id, cpu, heap, log_file, ARRIVE_MIN,
			     ARRIVE_MAX);

      // finds next job or sits in idle
      heap = job_finish_disk(disk1, cpu, heap, log_file);
    }else if(next_process->type == JOB_FIN_D2){
      // logs event
      fprintf(log_file, "Job %d finished at DISK 2 at %d\n", next_process->id,
	      next_process->time);
      
      //adds finished disk1 job to cpu or cpu queue
      heap = job_arrives_cpu(disk2->cur_id, cpu, heap, log_file, ARRIVE_MIN,
			     ARRIVE_MAX);

      // finds next job or sits in idle
      heap = job_finish_disk(disk2, cpu, heap, log_file);
    }else{
      printf("Error: Process of unknown type %d.\n", next_process->type);
      exit(-1);
    }
  }
}

// handles job finishing at a disk
// retrieves next job from corresponding disk queue
hnode * job_finish_disk(sub_system * disk, sub_system * cpu, hnode * heap,
			FILE * log_file){
  if(isEmpty(disk->queue)){
    // sits in idle
    disk->is_busy = 0;
    disk->cur_id = -1;
  }else{
    // retrieves next job
    disk->cur_id = dequeue(disk->queue);
    disk->is_busy = 1;

    // logs event
    fprintf(log_file, "Job %d enters disk %d from queue at %d.\n",
	    disk->cur_id, disk->fin_type, sim_time);
    
    // add new job fin time to heap
    heap = create_job_fin(disk, heap, disk->min_time, disk->max_time);

    // tracks disk queue statistics
    if(disk->fin_type == JOB_FIN_D1){
      // disk 1 average
      int cur_q_count = count_queue(disk->queue);
      d1_q_count++;
      d1_q_sum += cur_q_count;
    }else{
      // disk 2 average
      int cur_q_count = count_queue(disk->queue);
      d2_q_count++;
      d2_q_sum += cur_q_count;
    }

  }

  return heap;
}

// handles job arriving at disk by appending to queue or inserting
// job directly into disk
hnode * job_arrive_disk(sub_system * d1, sub_system * d2, hnode * heap,
			int id, FILE * log_file){
  // determines where to place job for disks
  if(d1->is_busy && d2->is_busy){
    // appends job to smallest queue
    if(compare_qs(d2->queue, d1->queue)){
      // append to d1
      enqueue(d1->queue, id);

      // tracks d1 average
      int cur_q_count = count_queue(d1->queue);
      d1_q_count++;
      d1_q_sum += cur_q_count;

      // tracks init times for d1
      enqueue(d1->init_job_times, sim_time);
      
      // tracks cpu queue max
      if(cur_q_count > d1_q_max){
	d1_q_max = cur_q_count;
      }
      
      // log event
      fprintf(log_file, "Job %d placed into DISK 1 queue at %d\n", id,
	      sim_time);
    }else{
      // tracks init times for d2
      enqueue(d2->init_job_times, sim_time);
      
      // append to d2
      enqueue(d2->queue, id);

      // tracks d2 average
      int cur_q_count = count_queue(d2->queue);
      d2_q_count++;
      d2_q_sum += cur_q_count;
      
      // tracks cpu queue max
      if(cur_q_count > d2_q_max){
	d2_q_max = cur_q_count;
      }
      
      // log event
      fprintf(log_file, "Job %d placed into DISK 2 queue at %d\n", id,
	      sim_time);
    }
      
  }else if(d1->is_busy == 0){
    // tracks init times for d1
    enqueue(d1->init_job_times, sim_time);

    // add job directly to d1
    d1->is_busy = 1;
    d1->cur_id = id;

    // add fin time to heap
    heap = create_job_fin(d1, heap, d1->min_time, d1->max_time);

    // log event
    fprintf(log_file, "Job %d placed directly into DISK 1 at %d\n",
	    id, sim_time);
  }else{
    // tracks init times for d2
    enqueue(d2->init_job_times, sim_time);

    // add job directly to d2
    d2->is_busy = 1;
    d2->cur_id = id;

    // add fin time to heap
    heap = create_job_fin(d2, heap, d2->min_time, d2->max_time);

    // log event
    fprintf(log_file, "Job %d placed directly into DISK 2 at %d\n", id,
	    sim_time);
  }

  return heap;
}

// handles job finish at cpu event. Checks if job can exit. If not,
// sends job to disk. Checks if a new job is waiting in cpu queue.
// creates a new job to send to heap for linear job growth.
hnode * job_finish_cpu(sub_system * cpu, sub_system * d1, sub_system * d2,
		       hnode * heap, double quit_prob, int arr_min,
		       int arr_max, FILE * log_file){
  if(can_exit(quit_prob)){
    // log job leaving simulation.
    fprintf(log_file, "Job %d leaving simulation at %d.\n", cpu->cur_id,
	    sim_time);
  }else{
    // add job to disk.
    heap = job_arrive_disk(d1, d2, heap, cpu->cur_id, log_file);
  }
  
  // sets subsystem to not busy if queue is empty, else grabs
  // next item in queue and updates subsystem
  if(isEmpty(cpu->queue)){
    cpu->is_busy = 0;
    cpu->cur_id = -1;
  }else{
    //dequeues next job
    int next_id = dequeue(cpu->queue);

    // updates cpu with new job info
    cpu->cur_id = next_id;
    cpu->is_busy = 1;

    // creates job finish time for new job
    heap = create_job_fin(cpu, heap, cpu->min_time, cpu->max_time);

    // logs event
    fprintf(log_file, "Job %d enters CPU from queue at %d.\n", cpu->cur_id,
	    sim_time);
    
    // creates new job to add to heap
    int arr_time = gen_rand_int(arr_min, arr_max) + sim_time;
      
    // increments global job count
    num_jobs++;

    // pushes new job to heap, this will make jobs grow linearly
    heap = push(heap, num_jobs, JOB_ENTERS, arr_time);

    // tracks cpu queue average
    int cur_q_count = count_queue(cpu->queue);
    cpu_q_count++;
    cpu_q_sum += cur_q_count;
  }

  return heap;
}

// returns 1 if left q is larger than right, returns 0 otherwise
int compare_qs(qnode * q1, qnode * q2){
  int q1_size = count_queue(q1);
  int q2_size = count_queue(q2);

  if(q1_size > q2_size){
    return 1;
  }else{
    return 0;
  }
}

// creates a job finish event for specific subsystem. finish time
// is current system time + a random integer constrained by min and
// max for the system.
hnode * create_job_fin(sub_system * sub, hnode * heap, int min_time,
		       int max_time){
  int fin_time = gen_rand_int(min_time, max_time) + sim_time;

  // pushes job finish time onto heap
  heap = push(heap, sub->cur_id, sub->fin_type, fin_time);

  // gets response time
  int response = fin_time - dequeue(sub->init_job_times);
  
  // calculates utilization of each server and counts num jobs processed
  if(fin_time <= time_end){
    if(sub->fin_type == JOB_FIN_CPU){
      // for cpu
      cpu_busy += fin_time - sim_time;

      cpu_count++;

      // average response
      cpu_response += response;

      // max response
      if(response >= cpu_max_response){
	cpu_max_response = response;
      }
      
      
    }else if(sub->fin_type == JOB_FIN_D1){
      // for d1
      d1_busy += fin_time - sim_time;
      d1_count++;

      // average response
      d1_response += response;

      // max response
      if(response >= d1_max_response){
	d1_max_response = response;
      }
      
    }else{
      // for d2
      d2_busy += fin_time - sim_time;

      d2_count++;

      // average response
      d2_response += response;

      // max response
      if(response >= d2_max_response){
	d2_max_response = response;
      }
      
    }
  }
  
  return heap;
}

// adds job to the cpu or the cpu queue.
hnode * job_arrives_cpu(int id, sub_system * sub, hnode * heap,
		    FILE * log_file, int arr_min, int arr_max){
  // tracks init times for cpu
  enqueue(sub->init_job_times, sim_time);

  // places job in corresponding queue
  if(sub->is_busy){
    enqueue(sub->queue, id);

    // tracks cpu queue average
    int cur_q_count = count_queue(sub->queue);
    cpu_q_count++;
    cpu_q_sum += cur_q_count;

    // tracks cpu queue max
    if(cur_q_count > cpu_q_max){
      cpu_q_max = cur_q_count;
    }
    
    // logs enqueueing
    fprintf(log_file, "job %d placed in cpu queue at %d\n", id, sim_time);
  }else{
    fprintf(log_file, "job %d placed directly into cpu at %d\n", id, sim_time);

    // add job directly to subsystem.
    sub->cur_id = id;

    // set sub_system to busy
    sub->is_busy = 1;

    // append job finish time to heap
    heap = create_job_fin(sub, heap, sub->min_time, sub->max_time);

    // creates new job to add to heap
    int arr_time = gen_rand_int(arr_min, arr_max) + sim_time;
    
    // increments global job count
    num_jobs++;

    // pushes new job to heap, this will make jobs grow linearly
    heap = push(heap, num_jobs, JOB_ENTERS, arr_time);
  }

  // returns altered heap
  return heap;
}

// creates a subsystem and initializes values. (CPU, Disk1, Disk2)
sub_system * create_sub(int type, int min_time, int max_time){
  sub_system * temp;

  temp = (sub_system*)malloc(sizeof(sub_system));

  if(temp == NULL){
    printf("ERROR: Failed to allocate subsystem.\n");
    exit(-1);
  }
  // creates fifo queue for subsystem  
  qnode * sub_q = make_queue();

  temp->cur_id = -1;
  temp->is_busy = 0;
  temp->fin_type = type;
  temp->queue = sub_q;
  temp->min_time = min_time;
  temp->max_time = max_time;

  // creates init job time tracker for subsystem statistics  
  temp->init_job_times = make_queue();

  return temp;
}

// generates a random integer between a specified min and max
int gen_rand_int(int min, int max){
  return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

// returns 1 if generated number (1-100) is <= probability*100, 0 otherwise
int can_exit(double prob){
  // sanity check for probability, also handles obvious cases
  if(prob <= 0){
    return 0;
  }else if(prob > 1){
    printf("ERROR: Invalid probability %lf\n", prob);
    exit(-1);
  }else if((int)prob == 1){
    return 1;
  }

  // converts prob to a comparable int for random number generator
  int min = 1;
  int max = 100;
  int int_prob = (int)(prob*100);

  int rand_num = gen_rand_int(min, max);

  // if generated number is less than probability, returns 1
  if(rand_num <= int_prob){
    return 1;
  }else{
    return 0;
  }
}
