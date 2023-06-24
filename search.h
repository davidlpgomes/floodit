#ifndef __SEARCH__
#define __SEARCH__

#include "floodit.h"

#define MAX_STATES 100000

typedef enum { UNEXAMINED, EXISTS, FORGOTTEN } board_state_t;

typedef struct action_t {
    unsigned char cor;
    unsigned char col;
} action_t;

typedef struct search_node_t {
    board_t *board;
    struct search_node_t *parent;

    action_t action;

    double g; // Cost of begin state to this one
    double h; // Heuristic of board
    double f; // F-cost (may be different from g + h because of the backup)

    struct search_node_t **succs;      // Successors pointers array
    unsigned succs_size;  // Successors array size being used
    char *succs_states;   // Array indicating succs states

    unsigned perimeter;
    unsigned char *colors_presence;
    unsigned q_count[4];
} search_node_t;

typedef struct search_t {
    unsigned bf;           // Branching factor
    unsigned ng;           // Nodes generated

    flood_t *flood;
    search_node_t *root;
} search_t;


search_t *create_search(flood_t *flood);

void free_search(search_t *search);

search_node_t *create_search_node(search_t *search, search_node_t *parent);

void free_search_nodes(search_t *search, search_node_t *node);

search_node_t *next(search_t *search, search_node_t *parent, int i, int corner);

search_node_t *test_ramification(search_t *search, search_node_t *node, int corner, unsigned depth);

search_node_t *searx(search_t *search, int depth);

search_node_t *search_node(search_t *search);

int get_distance_from_middle(search_node_t *node);

double heuristic(search_node_t *node);

unsigned find_closest_flooded_cell(search_node_t *node, int i, int j, int cor);

void print_actions(search_node_t *node);

#endif
