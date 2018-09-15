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
  
}sub_system;

// prototypes
int gen_rand_int(int min, int max);
int can_exit(double prob);
hnode * job_arrives(int id, sub_system * cpu, hnode * root,
		    FILE * log_file, int min_time, int max_time, int arr_min,
		    int arr_max);
sub_system * create_sub(int type);

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

  // creates fifo queue for cpu
  qnode * cpu_q = make_queue();

  // creates fifo queue for disk1
  qnode * disk1_q = make_queue();
  
  // creates fifo queue for disk2
  qnode * disk2_q = make_queue();

  // initializes heap job1 and end time
  heap = push(heap, 1, JOB_ENTERS, INIT_TIME);
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
  sub_system * cpu = create_sub(JOB_FIN_CPU);

  // creates disk1 subsystem
  sub_system * disk1 = create_sub(JOB_FIN_D1);

  // creates disk2 subsystem
  sub_system * disk2 = create_sub(JOB_FIN_D2);

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

      // closes log file
      fclose(log_file);
      
      return 0;
    }else if(next_process->type == JOB_ENTERS){
      // handles job arrival event
      heap = job_arrives(next_process->id, cpu, heap, log_file,
			 CPU_MIN, CPU_MAX, ARRIVE_MIN, ARRIVE_MAX);
    }
    
  }
}

// adds job to the subsystem or its corresponding queue
hnode * job_arrives(int id, sub_system * sub, hnode * heap,
		    FILE * log_file, int min_time, int max_time, int arr_min,
		    int arr_max){
  if(sub->is_busy){

  }else{
    // if sub_system is not busy, add job directly to sub.
    sub->cur_id = id;

    // set sub_system to busy
    sub->is_busy = 1;

    //determines finished time for job
    int fin_time;
    if(sub->fin_type == JOB_FIN_CPU){
      // for cpu subsystem
      fin_time = gen_rand_int(min_time, max_time) + sim_time;
      
    }else if(sub->fin_type == JOB_FIN_D1){
      // for d1 subsystem
      fin_time = gen_rand_int(min_time, max_time) + sim_time;

    }else{
      // for d2 subsystem
      fin_time = gen_rand_int(min_time, max_time) + sim_time;
    }
    
    // pushes job finish time onto heap
    heap = push(heap, sub->cur_id, sub->fin_type, fin_time);

    // creates new job to add to heap
    int arr_time = gen_rand_int(arr_min, arr_max);
  }
  
}

sub_system * create_sub(int type){
  sub_system * temp;

  temp = (sub_system*)malloc(sizeof(sub_system));

  temp->cur_id = -1;
  temp->is_busy = 0;
  temp->fin_type = type;

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
