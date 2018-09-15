#include<stdio.h>
#include<stdlib.h>

typedef struct hnode{
  int id;
  int type;
  int time;
  int size;
  int max_length;
}hnode;

#define INIT_SIZE 3

hnode * create_heap();
void print_heap(hnode * node);
hnode * push(hnode * root, int id, int type, int time);
int sort_element(hnode * root, int level, int flag);
hnode * pop(hnode * root);
void swap_nodes(hnode * root, int nodeA, int nodeB);
void heap_sort(hnode * root);

// iteratively sort heap
void heap_sort(hnode * root){
  // initializes required values for heap sort
  int smallest = 1;
  int cur_index = smallest;
  int left = 2*smallest;
  int right = 2*smallest+1;

  // flag to check if no swap was performed
  int flag = 0;

  // swaps until end of heap or no swaps were performed
  while(flag == 0 && (smallest < root->size)){
    // compares upper node to its left node
    if(left < root->size && root[smallest].time > root[left].time){
      smallest = left;
    }

    // compares upper node to its right node
    if(right < root->size && root[smallest].time > root[right].time){
      smallest = right;
    }

    // swaps nodes and initializes values for next subtree in the heap
    if(smallest != cur_index){
      swap_nodes(root, smallest, cur_index);
      left = 2*smallest;
      right = 2*smallest+1;
      cur_index = smallest;
    }else{
      // sets flag to 1 if no swaps were performed
      flag = 1;
    }
  }
}

hnode * pop(hnode * root){
  if(root->size == 1){
    printf("ERROR: Unable to pop, heap is empty\n");
    exit(-1);
  }

  // return node
  hnode * ret_node = (hnode*)malloc(sizeof(hnode)*2);

  // initializes return node with root values
  ret_node->id = root[1].id;
  ret_node->type = root[1].type;
  ret_node->time = root[1].time;
  ret_node->size = 2;
  ret_node->max_length = 2;

  // decrements root size
  root->size--;

  // swaps first and last node
  swap_nodes(root, 1, root->size);

  // sorts heap if more than one node exists
  if(root->size > 2){
    heap_sort(root);
  }

  // returns popped node
  return ret_node;
}

// swaps two nodes based on index
void swap_nodes(hnode * root, int nodeA, int nodeB){
    int id_temp = root[nodeA].id;
    int type_temp = root[nodeA].type;
    int time_temp = root[nodeA].time;

    root[nodeA].id = root[nodeB].id;
    root[nodeA].type = root[nodeB].type;
    root[nodeA].time = root[nodeB].time;
    
    root[nodeB].id = id_temp;
    root[nodeB].type = type_temp;
    root[nodeB].time = time_temp;

}

int sort_element(hnode * root, int level, int flag){
  // one node higher
  int new_level = level/2;

  // swaps nodes if lower node is less than higher node
  if(root[level].time < root[new_level].time){
    swap_nodes(root, level, new_level);
  }else{
    // sets flag to 1 if nodes were not swapped
    flag = 1;
  }

  // returns flag
  return flag;
}

hnode * push(hnode * root, int id, int type, int time){
  // doubles size of heap if full
  if(root->size == root->max_length){
    root->max_length*=2;
    root = (hnode*)realloc(root, sizeof(hnode)*(root->max_length));
  }

  // adds item to end of heap
  root[root->size].id = id;
  root[root->size].type = type;
  root[root->size].time = time;

  // grabs final node index
  int cur_index = root->size;

  // flag to determine if swap occurred
  int flag = 0;

  // bubbles up small values inserted into heap
  while(cur_index > 1 && flag == 0){
    flag = sort_element(root, cur_index, flag);
    cur_index/=2;
    }

  // increments heap size
  root->size++;
  
  // neccessary to return root since the pointer may be reset from realloc.
  return root;
}

hnode * create_heap(){
  hnode * root;

  // allocates root node on heap
  root = (hnode*)malloc(sizeof(hnode)*(INIT_SIZE));

  // checks for successful allocation
  if(root == NULL){
    printf("ERROR: Failed to create pqueue");
    exit(-1);
  }

  // initializes pqueue nodes
  int i = 0;
  for(i = 0; i < INIT_SIZE; i++){
    root[i].id = -1;
    root[i].type = -1;
    root[i].time = -1;
    root[i].size = 1;
    root[i].max_length = INIT_SIZE;
  }

  // returns pointer to root node
  return root;
}


void print_heap(hnode *node){
  // if heap is empty, prints size and max_length only
  if(node->size == 1){
    printf("Heap is empty\n");
    printf("\tHeap Size : [%d]\n\tMax Heap Length : %d\n", node->size,
	   node->max_length);
    return;
  }

  // prints each node's data
  int i = 0;
  for(i = 0; i < node->size; i++){
    printf("NODE %d\n", i);
    printf("\tID : [%d]\n", node[i].id);
    printf("\tTYPE : [%d]\n", node[i].type);
    printf("\tTIME : [%d]\n", node[i].time);
    printf("\n");
  }

  // prints pqueue size and max length
  printf("Heap Size : [%d]\nMax Heap Length : %d\n", node->size,
	 node->max_length);
}
