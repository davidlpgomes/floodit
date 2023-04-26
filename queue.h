#ifndef __QUEUE__
#define __QUEUE__

typedef struct node_t {
    struct node_t* next;

    unsigned i;
    unsigned j;
} node_t;

typedef struct {
    node_t* beg;
    node_t* end;

    unsigned size;
} queue_t;

queue_t* queue_create();

void queue_free(queue_t* queue);

int queue_append(queue_t* queue, unsigned i, unsigned j);

int dequeue(queue_t* queue, unsigned* i, unsigned* j);

#endif

