#include<stdio.h>
#include<stdlib.h>

typedef struct qnode{
  int id;
  struct qnode *end;
  struct qnode *next;
}qnode;

qnode *make_queue();
void print_node(qnode * node);
void enqueue(qnode * root, int id);
int dequeue(qnode * root);
int isEmpty(qnode * root);
int count_queue(qnode * root);

//returns 1 if empty, 0 if not
int isEmpty(qnode * root){
  return(root->next == NULL);
}

// removes last node, returns id within removed node or -1 if queue is empty
int dequeue(qnode * root){
  
  int id;

  // returns -1 if queue is empty
  if(root->next == NULL){
    printf("ERRROR: Unable to dequeue, queue is empty\n");
    exit(-1);
  }

  id = root->next->id;

  // holds node to be freed
  qnode * temp;

  // points root to next in line
  temp = root->next;
  root->next = root->next->next;

  // detaches temp and frees
  temp->next = NULL;
  free(temp);

  // points end to null if queue was only 1 in length
  if(root->next == NULL){
    root->end = NULL;
  }

  // returns id of dequeued node
  return id;
}

void enqueue(qnode * root, int id){
  // allocates new node
  qnode * node;
  node = (qnode*)malloc(sizeof(qnode));

  // checks for successful allocation
  if(node == NULL){
    printf("ERROR: Failed to malloc new node\n");
    exit(-1);
  }

  // sets new node's info
  node->id = id;
  node->next = NULL;
  
  // if queue is empty, sets end to node and next to node
  // if queue has something, points final node to new node and adds node to end
  if(root->end == NULL){
    root->end = node;
    root->next = node;
  }else{
    root->end->next = node;
    root->end = node;
  }
}

// creates the root of a queue, initializing to -1/NULL
qnode *make_queue(){
  qnode *root;

  // allocates memory for root node
  root = (qnode*)malloc(sizeof(qnode));

  // checks for failure
  if(root == NULL){
    printf("ERROR: Root failed to malloc\n");
    exit(-1);
  }

  // initializes root node
  root->end = NULL;
  root->next = NULL;
  root->id = -1;

  // returns root node
  return root;
}

// prints node info
void print_node(qnode * node){
  printf("------------------\n");
  printf("end: %p\n", node->end);
  printf("next: %p\n", node->next);
  printf("id: %d\n", node->id);
  printf("------------------\n");
}

// returns size of queue
int count_queue(qnode * root){
  int count = 0;

  // if queue is empty, returns 0
  if(root->next == NULL){
    return count;
  }

  // tracking variable
  qnode * temp = root->next;

  // iterates through queue to end
  while(temp != NULL){
    temp = temp->next;
    count++;
  }

  // returns size of queue
  return count;
}
