#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "pqueue.h"
#include "read_config.h"

#define JOB_ENTERS 0
#define JOB_FIN_CPU 1
#define JOB_FIN_D1 2
#define JOB_FIN_D2 3
#define SIM_FIN 4

// global job counter
int num_jobs = 1;

// global simulation time counter
int sim_time = 0;

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
hnode * job_finish_cpu(sub_system * cpu, sub_system * d1, sub_system * d2,
		       hnode * heap, int quit_prob);
hnode * job_arrive_disk(sub_system * d1, sub_system * d2, hnode * heap,
			int id);

int main(char argc, char ** argv){
  // reads config values from txt file
  double * config_vals = read_config(argv[1]);

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

  // frees config val array
  free(config_vals);

  // initializes random number generators.
  srand(SEED);
  
  // creates heap for job time ordering
  hnode * heap = create_heap();

  // initializes heap job1 and end time
  heap = push(heap, num_jobs, JOB_ENTERS, INIT_TIME);
  heap = push(heap, -1, SIM_FIN, FIN_TIME);

  // initializes simulation timer
  sim_time = INIT_TIME;

  // opens log file
  FILE *log_file;

  // clears log_file
  log_file = fopen(argv[2], "w");
  fclose(log_file);
  
  // opens log file for append
  log_file = fopen(argv[2], "a");

  // prints error if log file fails to open
  if(log_file == NULL){
    printf("ERROR: Failed to open [%s]\n", argv[2]);
    exit(-1);
  }

  // creates cpu subsystem
  sub_system * cpu = create_sub(JOB_FIN_CPU, CPU_MIN, CPU_MAX);

  // creates disk1 subsystem
  sub_system * disk1 = create_sub(JOB_FIN_D1, DISK1_MIN, DISK1_MAX);

  // creates disk2 subsystem
  sub_system * disk2 = create_sub(JOB_FIN_D2, DISK2_MIN, DISK2_MAX);

  // simulation loop
  while(1){
    print_heap(heap);
    
    // gets next process
    hnode * next_process = pop(heap);

    // increments sim_time
    sim_time = next_process->time;

    // calls handler function based on process type
    if(next_process->type == SIM_FIN){
      printf("SIMULATION COMPLETE\n");

      // frees allocated memory
      free(heap);
      free(cpu);
      free(disk1);
      free(disk2);
      
      // closes log file
      fclose(log_file);
      
      return 0;
    }else if(next_process->type == JOB_ENTERS){
      // handles job arrival event
      heap = job_arrives_cpu(next_process->id, cpu, heap, log_file,
			 ARRIVE_MIN, ARRIVE_MAX);
    }else if(next_process->type == JOB_FIN_CPU){
      heap = job_finish_cpu(cpu, disk1, disk2, heap, QUIT_PROB);
    }else{
      printf("Error: Process of unknown type %d.\n", next_process->type);
      exit(-1);
    }
  }
}

hnode * job_arrive_disk(sub_system * d1, sub_system * d2, hnode * heap,
			int id){
  if(d1->is_busy && d2->is_busy){
    // append to smallest queue
  }else if(d1->is_busy == 0){
    // append to d1 queue
  }else{
    // append to d2 queue
  }

  return heap;
}

hnode * job_finish_cpu(sub_system * cpu, sub_system * d1, sub_system * d2,
		       hnode * heap, int quit_prob){
  if(can_exit(quit_prob)){
    // log job leaving simulation.
    printf("Job %d leaving simulation.\n", cpu->cur_id);
  }else{
    // add job to disk.
    heap = job_arrive_disk(d1, d2, heap, cpu->cur_id);
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

hnode * create_job_fin(sub_system * sub, hnode * heap, int min_time,
		       int max_time){
  int fin_time = gen_rand_int(min_time, max_time) + sim_time;

  // pushes job finish time onto heap
  heap = push(heap, sub->cur_id, sub->fin_type, fin_time);

  return heap;
}

// adds job to the subsystem or its corresponding queue
hnode * job_arrives_cpu(int id, sub_system * sub, hnode * heap,
		    FILE * log_file, int arr_min, int arr_max){
  // places job in corresponding queue
  if(sub->is_busy){
    enqueue(sub->queue, id);
  }else{
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

sub_system * create_sub(int type, int min_time, int max_time){
  sub_system * temp;

  temp = (sub_system*)malloc(sizeof(sub_system));

  // creates fifo queue for subsystem  
  qnode * sub_q = make_queue();

  temp->cur_id = -1;
  temp->is_busy = 0;
  temp->fin_type = type;
  temp->queue = sub_q;
  temp->min_time = min_time;
  temp->max_time = max_time;

  return temp;
}

// generates a random integer between a specified min and max
int gen_rand_int(int min, int max){
  return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

// returns 1 if generated number (1-100) is <= probability*100, 0 otherwise
int can_exit(double prob){
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
