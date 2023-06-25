#ifndef __SEARCH__
#define __SEARCH__

#include "floodit.h"

typedef struct action_t {
    unsigned char cor;
    unsigned char col;
} action_t;

typedef struct search_node_t {
    board_t *board;
    struct search_node_t *parent;

    action_t action;
    unsigned f; // Cost function
    unsigned d; // Depth
} search_node_t;

typedef struct search_t {
    unsigned bf;           // Branching factor
    unsigned ng;           // Nodes generated

    flood_t *flood;
    search_node_t *root;
} search_t;


search_t *create_search(flood_t *flood);

void free_search(search_t *search);

search_node_t *create_search_node(
    search_t *search,
    search_node_t *parent
);

void free_search_node(search_node_t *node);

void free_search_nodes_n(search_node_t *node, int n);

void free_search_nodes(search_node_t *node);

search_node_t *next(
    search_t *search,
    search_node_t *parent,
    int i,
    int corner
);

search_node_t *deepening(
    search_t *search,
    search_node_t *node,
    int corner,
    unsigned depth
);

search_node_t *search(search_t *search, int depth, char print);

int get_distance_from_middle(search_node_t *node);

unsigned f(search_node_t *node);

void print_actions(search_node_t *node);

#endif
