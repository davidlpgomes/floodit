#include "queue.h"

#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

queue_t* queue_create() {
    queue_t* queue = malloc(sizeof(queue_t));
    test_alloc(queue, "queue");

    queue->beg = NULL;
    queue->end = NULL;
    queue->size = 0;

    return queue;
}

void queue_free(queue_t* queue) {
    node_t* node = queue->beg;
    node_t* next;

    while (node) {
        next = node->next;

        free(node);
        node = next;
    }

    queue->size = 0;
    free(queue);

    return;
}

int queue_append(queue_t* queue, unsigned i, unsigned j) {
    if (!queue) {
        fprintf(stderr, "Error: queue does not exist\n");
        return -1;
    }

    node_t* node = malloc(sizeof(node_t));
    test_alloc(node, "node");

    node->i = i;
    node->j = j;
    node->next = NULL;

    if (queue->size)
        queue->end->next = node;
    else
        queue->beg = node;

    queue->end = node;
    queue->size += 1;

    return 1;
}

int dequeue(queue_t* queue, unsigned* i, unsigned* j) {
    if (!queue) {
        fprintf(stderr, "Error: queue does not exist\n");
        return -1;
    }

    if (!queue->size)
        return 0;

    *i = queue->beg->i;
    *j = queue->beg->j;

    node_t* next = queue->beg->next;
    free(queue->beg);
    queue->beg = next;

    queue->size -= 1;

    if (!queue->beg)
        queue->end = NULL;
        

    return 1;
}
