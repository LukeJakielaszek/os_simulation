#ifndef PQUEUE_H
#define PQUEUE_H

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

#endif /** PQUEUE_H */
