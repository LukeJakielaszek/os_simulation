#ifndef QUEUE_H
#define QUEUE_H

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

#endif /** QUEUE_H */
