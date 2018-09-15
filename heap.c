#include<stdio.h>
#include<stdlib.h>

#define INIT_SIZE 3

typedef struct hnode{
  int id;
  int type;
  int time;
}hnode;

typedef struct heap{
  hnode * root;
  int size;
  int max_size;
}heap;

heap * create_heap();
void print_heap(heap * pqueue);
void insert(heap *pqueue, int id, int type, int time);

int main(){

  heap *pqueue = create_heap();

  print_heap(pqueue);

  insert(pqueue, 123, 5, 1);
  insert(pqueue, 456, 10, 2);
  insert(pqueue, 789, 15, 3);
  insert(pqueue, 432, 20, 4);
  
  print_heap(pqueue);
  
  return 0;
}

void insert(heap *pqueue, int id, int type, int time){
  if(pqueue->size == 0){
    hnode * new_node = (hnode*)malloc(sizeof(hnode)*INIT_SIZE);
    new_node->id = id;
    new_node->type = type;
    new_node->time = time;
    
    pqueue->root = new_node;
    pqueue->size++;
  }else if(pqueue->size < pqueue->max_size){
    pqueue->root[pqueue->size].id = id;
    pqueue->root[pqueue->size].id = id;
    pqueue->root[pqueue->size].id = id;

    pqueue->size++;
  }else{
    pqueue->max_size*=2;
    pqueue->root = (hnode*)realloc(pqueue->root,
				   sizeof(hnode)*pqueue->max_size);

    pqueue->root[pqueue->size].id = id;
    pqueue->root[pqueue->size].id = id;
    pqueue->root[pqueue->size].id = id;
    
    pqueue->size++;
  }
}

heap * create_heap(){
  heap *pqueue;
  pqueue = (heap*)malloc(sizeof(heap));
  pqueue->size = 0;
  pqueue->max_size = INIT_SIZE;
  pqueue->root = NULL;

  return pqueue;
}

void print_heap(heap * pqueue){
  int i = 0;
  for(i = 0; i < pqueue->size; i++){
    printf("Node %d:\n", i);
    printf("\tID: %d\n:", pqueue->root[i].id);
    printf("\tType: %d\n:", pqueue->root[i].type);
    printf("\tTime: %d\n", pqueue->root[i].time);
  }

  printf("\nHeap size [%d] , current max heap size [%d]\n", pqueue->size,
	 pqueue->max_size);
}
