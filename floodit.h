#ifndef __FLOODIT__
#define __FLOODIT__

#define MAX_STATES 20000

#define ANSI_COLOR_RESET   "\x1b[0m"

#include "tree.h"
#include "queue.h"

typedef enum { UL, UR, LR, LL } corner_t;

typedef enum { UNEXAMINED, EXISTS, FORGOTTEN } board_state_t; 

typedef struct cell_t {
    unsigned char c;   // Color
    char f;            // Flooded
    unsigned char cor; // Indicates which corner it was flooded by
} cell_t;

typedef struct board_t {
    struct flood_t* fl;    // Pointer to Floodit game
    unsigned p;            // Number os steps, used as depth
    
    unsigned char cor;     // Action's corner that lead to this state
    unsigned char col;     // Action's color that lead to this state
   
    // Data used for heuristic
    unsigned perimeter;
    unsigned char* colors_presence;
    unsigned q_count[4];
    
    cell_t** matrix;       // Matrix with the board cells

    tree_node_t* open_node;
    tree_node_t* leaves_node;

    struct board_t* parent;

    struct board_t** succs; // Successors pointers array
    unsigned succs_size;    // Successors array size being used
    
    char* succs_states;     // Array indicating succs states

    double g;  // Cost of begin state to this one
    double h;  // Heuristic of board
    double f;  // F-cost (may be different from g + h because of the backup)
} board_t; // Should be of size 21KB each

typedef struct flood_t {
    unsigned n;           // Number of lines
    unsigned m;           // Number of columns
    unsigned k;           // Number of colors
    
    unsigned bf;           // Branching factor
    unsigned ng;           // Nodes generated

    queue_t* cells_queue; // Queue to paint cells used by all boards
} flood_t;

flood_t* create_flood(unsigned n, unsigned m, unsigned k);

void free_flood(flood_t* fl);

board_t* create_board(flood_t* fl);

void free_board(board_t* board);

void read_board(board_t* board);

void copy_board(board_t* destination, board_t* source);

void print_board(board_t* board);

void print_board_colors(board_t* board);

void print_board_numbers(board_t* board);

void paint_corner(board_t* board, corner_t corner, unsigned color);

void flood(board_t* board, unsigned char color, unsigned r, unsigned c);

void floodN(board_t* board, unsigned char color, unsigned char cor, unsigned r, unsigned c);

void floodC(board_t* board, unsigned char color, unsigned char cor, unsigned r, unsigned c);

int is_flooded(board_t* board);

board_t* sma(board_t* board_start);

board_t* next_successor(board_t* parent);

void remove_from_parent_succs(board_t* board);

int is_completed(board_t* board);

int is_all_succs_in_mem(board_t* board);

void sma_backup(board_t* board, tree_t* open_tree);

double heuristic(board_t* board);

#endif
