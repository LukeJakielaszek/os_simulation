#include<stdio.h>
#include<stdlib.h>
#include<string.h>

double *read_config(char * fname);

#define BUFF_SIZE 50
#define NUM_PARAMS 12

// Reads ordered config file params into array of double. Returns array.
// Requirements to be read are param and value must be leftmost in line.
// Therefore |SEED 5 asdasdf| works, but |asdas SEED 5| does not. 
double *read_config(char * fname){
  // keys for initialization
  char keys[BUFF_SIZE][NUM_PARAMS] =
  {
    "SEED", "INIT_TIME", "FIN_TIME", "ARRIVE_MIN", "ARRIVE_MAX", "QUIT_PROB",
    "CPU_MIN", "CPU_MAX", "DISK1_MIN", "DISK1_MAX", "DISK2_MIN", "DISK2_MAX"
  };

  // values to keep track of parameter pairs
  double * values = (double*)malloc(sizeof(double)*NUM_PARAMS);

  // opens file in read
  FILE * config;
  config = fopen(fname, "r");
  if(config == NULL){
    printf("ERROR: File failed to open\n");
    exit(-1);
  }

  // buffer for reading line
  char buffer[BUFF_SIZE];

  // reads file line by line
  int count = 0;
  while(fgets(buffer, BUFF_SIZE, config) != NULL){

    // string to search for in file
    char search[BUFF_SIZE];

    // copies param key to search
    strcpy(search, keys[count]);

    // appends scanner variable to search
    strcat(search, " %lf");
      
    // if it successfully reads a key's value, store it in value
    // and increment count.
    if(sscanf(buffer, search, &values[count]) != 0){
      count++;
    }

  }

  // closes file
  fclose(config);

  // ensures the correct number of parameters were read
  if(count != NUM_PARAMS){
    printf("ERROR: Read [%d] params from file but expected [%d].\n",
	   count, NUM_PARAMS);
    exit(-1);
  }
  
  // returns config values
  return values;
}
