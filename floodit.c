#include "floodit.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "tree.h"
#include "utils.h"


double time_sma = 0;
double time_if = 0;
double time_ns = 0;
double time_paint = 0;
double time_h = 0;
double time_bk = 0;
double time_hc = 0;


flood_t* create_flood(unsigned n, unsigned m, unsigned k) {
    flood_t* fl = malloc(sizeof(flood_t));
    test_alloc(fl, "flood");

    fl->n = n;
    fl->m = m;
    fl->k = k;

    // Calculate branching factor
    fl->bf = 4 * (k - 1);
    fl->ng = 0;

    fl->cells_queue = malloc(sizeof(queue_t));
    test_alloc(fl->cells_queue, "cells queue");

    #ifdef DEBUG
    printf("[FL] Flood created\n");
    #endif

    return fl;
}

void free_flood(flood_t* fl) {
    if (!fl)
        return;

    if (fl->cells_queue)
        free(fl->cells_queue);

    free(fl);

    #ifdef DEBUG
    printf("[FL] Flood freed\n");
    #endif

    return;
}

board_t* create_board(flood_t* fl) {
    if (!fl) {
        fprintf(stderr, "Error: flood does not exist\n");
        exit(1);
    }

    board_t* board = malloc(sizeof(board_t));
    test_alloc(board, "board");

    board->fl = fl;
    board->p = 0;
    board->fldd = 4;

    board->cor = 64;
    board->col = 0;

    board->matrix = malloc(fl->n * sizeof(int*));
    test_alloc(board->matrix, "matrix pointers");

    board->matrix[0] = calloc(fl->n * fl->m, sizeof(cell_t));
    test_alloc(board->matrix[0], "board cells matrix");

    // Adjust pointers to the beggining of each line
    for (int i = 1; i < fl->n; i++)
        board->matrix[i] = board->matrix[0] + i * fl->m;

    // Set corners as flooded
    board->matrix[0][0].f = 1;
    board->matrix[0][fl->m - 1].f = 1;
    board->matrix[fl->n - 1][0].f = 1;
    board->matrix[fl->n - 1][fl->m - 1].f = 1;

    board->open_node = NULL;
    board->leaves_node = NULL;
    board->parent = NULL;
    
    board->succs = malloc(board->fl->bf * sizeof(board_t*));
    test_alloc(board->succs, "board successors array");
    board->succs_size = 0;

    board->succs_states = calloc(board->fl->bf, sizeof(char));
    test_alloc(board->succs_states, "board sucessors states array");

    board->g = 0;
    board->h = 0;
    board->f = 0;

    #ifdef DEBUG
    printf("[FL] Board created\n");
    #endif

    return board;
}

void free_board(board_t *board) {
    if (!board)
        return;

    if (board->matrix[0])
        free(board->matrix[0]);
   
    if (board->matrix)
        free(board->matrix);

    if (board->succs)
        free(board->succs);

    if (board->succs_states)
        free(board->succs_states);

    free(board);

    #ifdef DEBUG
    printf("[FL] Board freed\n");
    #endif

    return;
}

void read_board(board_t* board) {
    for (int i = 0; i < board->fl->n; i++)
        for (int j = 0; j < board->fl->m; j++)
            scanf("%u", &board->matrix[i][j].c);

    #ifdef DEBUG
    printf("[FL] Board data read\n");
    #endif

    return; 
}

void copy_board(board_t* destination, board_t* source) {
    destination->p = source->p;
    destination->fldd = source->fldd;

    int matrix_size = source->fl->n * source->fl->m;
    for (int i = 0; i < matrix_size; i++)
        destination->matrix[0][i] = source->matrix[0][i];

    return;
}

void print_board(board_t* board) {
    #ifdef DEBUG
    printf("[FL] Printing board...\n");
    #endif

    if (board->fl->k > 16)
        print_board_numbers(board);
    else
        print_board_colors(board);

    return;
}

void print_board_colors(board_t* board) {
    if (board->fl->k > 16)
        return;

    for (int i = 0; i < board->fl->n; i++) {
        for (int j = 0; j < board->fl->m; j++) {
            printf(ansi_bgs[board->matrix[i][j].c - 1]);
            printf("  " ANSI_COLOR_RESET);
        }

        printf("\n");
    }

    return;
}

void print_board_numbers(board_t* board) {
    for (int i = 0; i < board->fl->n; i++) {
        for (int j = 0; j < board->fl->m; j++)
            printf("%2d ", board->matrix[i][j].c);

        printf("\n");
    }
    return;
}

void paint_corner(board_t* board, corner_t corner, unsigned color) {
    #ifdef DEBUG
    printf(
        "[FL][PC] Executing action %c%u in board p=%u\n",
        board->cor,
        board->col,
        board->p
    );
    #endif

    unsigned ei = board->fl->n - 1; // End i
    unsigned ej = board->fl->m - 1; // End j

    if (
        (corner == UL && color == board->matrix[0][0].c) ||
        (corner == UR && color == board->matrix[0][ej].c) ||
        (corner == LR && color == board->matrix[ei][ej].c) ||
        (corner == LL && color == board->matrix[ei][0].c) ||
        color > board->fl->k
    ) {
        #ifdef DEBUG
        printf("[FL][PC] Trying to paint corner with its color\n");
        #endif
        return;
    }

    unsigned i, j, old_color;

    if (corner == UL) {
        old_color = board->matrix[0][0].c;
        queue_append(board->fl->cells_queue, 0, 0);
    } else if (corner == UR) {
        old_color = board->matrix[0][ej].c;
        queue_append(board->fl->cells_queue, 0, ej);
    } else if (corner == LR) {
        old_color = board->matrix[ei][ej].c;
        queue_append(board->fl->cells_queue, ei, ej);
    } else if (corner == LL) {
        old_color = board->matrix[ei][0].c;
        queue_append(board->fl->cells_queue, ei, 0);
    } else return;

    board->fldd = 0;
    while (board->fl->cells_queue->size) {
        dequeue(board->fl->cells_queue, &i, &j);

        if (board->matrix[i][j].c != old_color)
            continue;

        board->matrix[i][j].c = color;
        board->matrix[i][j].f = 1;
        board->fldd += 1;

        if (j > 0 && board->matrix[i][j - 1].c == old_color)
            queue_append(board->fl->cells_queue, i, j - 1);

        if (j < ej && board->matrix[i][j + 1].c == old_color)
            queue_append(board->fl->cells_queue, i, j + 1);

        if (i > 0 && board->matrix[i - 1][j].c == old_color)
            queue_append(board->fl->cells_queue, i - 1, j);

        if (i < ei && board->matrix[i + 1][j].c == old_color)
            queue_append(board->fl->cells_queue, i + 1, j);
    }

    board->p += 1;

    return;
}

void floodC(board_t* board, unsigned color, unsigned r, unsigned c) {
    if (board->matrix[r][c].f) return;

    if (board->matrix[r][c].c == color) {
        board->matrix[r][c].f = 1;
        floodN(board, color, r, c);
    }

    return;
} 

void floodN(board_t* board, unsigned color, unsigned r, unsigned c) {
    if (r > 0) floodC(board, color, r - 1, c);
    if (r < board->fl->n - 1) floodC(board, color, r + 1, c);
    if (c > 0) floodC(board, color, r, c - 1);
    if (c < board->fl->m - 1) floodC(board, color, r, c + 1);

    return;
}

void flood(board_t* board, unsigned color, unsigned r, unsigned c) {
    #ifdef DEBUG
    printf(
        "[FL][PC] Flooding board, action %c%u p=%u\n",
        board->cor,
        board->col,
        board->p
    );
    #endif

    // It assumes that the color, row and columns params 
    // are valid for efficiency reasons

    if (board->matrix[r][c].c == color) return;

    for (int i = 0; i < board->fl->n; i++)
        for (int j = 0; j < board->fl->m; j++) {
            if (board->matrix[i][j].f) {
                board->matrix[i][j].c = color;
                floodN(board, color, i, j);
            }
        }

    board->p += 1;

    return;
}

int is_flooded(board_t* board) {
    unsigned color = board->matrix[0][0].c;

    for (int i = 0; i < board->fl->n; i++)
        for (int j = 0; j < board->fl->m; j++)
            if (board->matrix[i][j].c != color)
                return 0;

    return 1;
}

board_t* sma(board_t* board_start) {
    tree_t* open = tree_create();
    tree_t* leaves = tree_create();

    int used = 1;
    int state = 0;

    board_start->h = heuristic(board_start);
    board_start->f = board_start->h;

    board_start->open_node = tree_insert(
        open,
        board_start->f,
        board_start->p,
        board_start
    );

    board_start->leaves_node = tree_insert(
        leaves,
        board_start->f,
        board_start->p,
        board_start
    );

    board_t *best, *succ, *highest_cost;
    double start;

    #ifdef DEBUG
    printf("[FL] Initializing Simple Memory-bounded A*\n");
    #endif

    int i = 0;

    double start_sma = timestamp();
    for (;;) {
        if (tree_empty(open)) {
            #ifdef DEBUG
            printf("[FL][SMA*][i=%d] Tree is empty, returning NULL\n", i);
            #endif
            state = -1;
            break;
        }

        #ifdef DEBUG
        printf(
            "\n[FL][SMA*][i=%d] Tree has size %d, used is %d\n",
            i,
            open->size,
            used
        );
        #endif

        best = (board_t*) tree_minimum(open, open->root)->v;
        #ifdef DEBUG
        printf(
            "[FL][SMA*] Best is %c%u p=%u fldd=%u g=%d h=%d f=%d\n",
            best->cor,
            best->col,
            best->p,
            best->fldd,
            best->g,
            best->h,
            best->f
        );
        #endif

        start = timestamp();
        if (is_flooded(best)) {
            #ifdef DEBUG
            printf("[FL][SMA*][i=%d] Best is flooded, returning best\n", i);
            #endif
            state = 0;
            break;
        }
        time_if += timestamp() - start;

        board_start->fl->ng += 1;

        start = timestamp();
        succ = next_successor(best);
        time_ns += timestamp() - start;

        if (!succ) {
            fprintf(stderr, "Error: succ is NULL\n");
            exit(1);
        }

        #ifndef DEBUG
        clear_terminal();
        print_board(succ);
        usleep(1000);
        printf("\n");
        #endif


        start = timestamp();
        succ->h = heuristic(succ);
        time_h += timestamp() - start;

        succ->g = best->g + 1;
        succ->f = succ->g + succ->h;
        #ifdef DEBUG
        printf(
            "[FL][SMA*] Succ created %c%u p=%u fldd=%u g=%d h=%d f=%d\n",
            succ->cor,
            succ->col,
            succ->p,
            succ->fldd,
            succ->g,
            succ->h,
            succ->f
        );
        #endif

        if (best->succs_size && best->leaves_node) {
            tree_delete(leaves, best->leaves_node);
            best->leaves_node = NULL;
        }

        if (is_completed(best)) {
            #ifdef DEBUG
            printf("[FL][SMA*][i=%d] Best is completed, backing up\n", i);
            #endif

            start = timestamp();
            sma_backup(best, open);
            time_bk += timestamp() - start;
        }

        if (is_all_succs_in_mem(best)) {
            #ifdef DEBUG
            printf("[FL][SMA*][i=%d] Best succs all in memory, deleting\n", i);
            #endif
            tree_delete(open, best->open_node);
            best->open_node = NULL;
        }

        used++;

        start = timestamp();
        if (used > MAX_STATES) {
            #ifdef DEBUG
            printf("[FL][SMA*] Leaves tree size is %d\n", leaves->size);
            #endif
            highest_cost = (board_t*) tree_maximum(leaves, leaves->root)->v;

            if (!highest_cost) {
                fprintf(stderr, "[FL][SMA*] Error: highest cost is NULL, i.e., max states is not enough to solve the instance\n");
                exit(1);
            }

            #ifdef DEBUG
            printf(
                "[FL][SMA*] HC is %c%u p=%u fldd=%u g=%d h=%d f=%d\n",
                highest_cost->cor,
                highest_cost->col,
                highest_cost->p,
                highest_cost->fldd,
                highest_cost->g,
                highest_cost->h,
                highest_cost->f
            );
            #endif

            if (highest_cost->open_node) {
                tree_delete(open, highest_cost->open_node);
                highest_cost->open_node = NULL;
            }

            tree_delete(leaves, highest_cost->leaves_node);
            highest_cost->leaves_node = NULL;

            remove_from_parent_succs(highest_cost);

            board_t* parent = highest_cost->parent;
            if (!parent->succs_size) {
                parent->open_node = tree_insert(
                    open,
                    parent->f,
                    parent->p,
                    parent
                );

                parent->leaves_node = tree_insert(
                    leaves,
                    parent->f,
                    parent->p,
                    parent
                );
            }

            free_board(highest_cost);

            #ifdef DEBUG
            printf("[FL][SMA*] Max states, highest cost removed\n");
            #endif

            used--;
        }
        time_hc += timestamp() - start;

        #ifdef DEBUG
        printf("[FL][SMA*] Inserting succ on open/leaves trees\n");
        #endif

        succ->open_node = tree_insert(open, succ->f, succ->p, succ);
        succ->leaves_node = tree_insert(leaves, succ->f, succ->p, succ);

        i++;
    }

    tree_free(leaves);
    tree_free(open);

    time_sma = timestamp() - start_sma;

    printf("\n\n---- TIMES SMA*\n");
    printf("\tTotal: %.15g\n", time_sma);
    printf("\tIs Fl: %.15g\n", time_if);
    printf("\tNx sc: %.15g\n", time_ns - time_paint);
    printf("\tPaint: %.15g\n", time_paint);
    printf("\tHeuri: %.15g\n", time_h);
    printf("\tBckup: %.15g\n", time_bk);
    printf("\tHighc: %.15g\n", time_hc);
    printf("[FL][SMA*] Time to paint: %.15g\n", time_paint);

    if (state)
        return NULL;

    return best;
}

board_t* next_successor(board_t* parent) {
    int last_existing_succ = -1;
    for (int i = 0; i < parent->fl->bf; i++)
        if (parent->succs_states[i] == EXISTS) {
            last_existing_succ = i;
        }

    int index_to_generate = last_existing_succ + 1;

    if (index_to_generate == parent->fl->bf)
        index_to_generate = 0;

    #ifdef DEBUG
    printf(
        "[FL][SMA*][NS] Index to generate is %d\n",
        index_to_generate
    );
    #endif

    int corner = index_to_generate / (parent->fl->k - 1);
    unsigned color = index_to_generate - corner * (parent->fl->k - 1) + 1;

    unsigned ei = parent->fl->n - 1;
    unsigned ej = parent->fl->m - 1;
    if (
        (corner == UL && color >= parent->matrix[0][0].c) ||
        (corner == UR && color >= parent->matrix[0][ej].c) ||
        (corner == LR && color >= parent->matrix[ei][ej].c) ||
        (corner == LL && color >= parent->matrix[ei][0].c)
    )
        color++;

    #ifdef DEBUG
    printf(
        "[FL][SMA*][NS] Generating succ i=%d cor=%d col=%u\n",
        index_to_generate,
        corner,
        color
    );
    #endif

    board_t* succ = create_board(parent->fl);
    copy_board(succ, parent);

    parent->succs_states[index_to_generate] = EXISTS;
    parent->succs[index_to_generate] = succ;
    parent->succs_size++;

    succ->parent = parent;
    succ->cor = corner + 97;
    succ->col = color;

    int r = -1;
    int c = -1;

    if (corner == UL || corner == UR) r = 0;
    else r = parent->fl->n - 1;

    if (corner == UL || corner == LL) c = 0;
    else c = parent->fl->m - 1;

    double start_pc = timestamp();
    // paint_corner(succ, corner, color);
    flood(succ, color, r, c);
    time_paint += timestamp() - start_pc;

    return succ;
}

void remove_from_parent_succs(board_t *board) {
    if (!board->parent)
        return;

    int i;
    for (i = 0; i < board->fl->bf; i++)
        if (board->parent->succs[i] == board)
            break;

    if (board->parent->succs[i] != board) {
        #ifdef DEBUG
        printf("[FL][SMA*] Board is not registered in parent\n");
        #endif
        return;
    }

    board->parent->succs_states[i] = FORGOTTEN;
    board->parent->succs[i] = NULL;
    board->parent->succs_size -= 1;


    return;
}

int is_completed(board_t* board) {
    for (int i = 0; i < board->fl->bf; i++)
        if (board->succs_states[i] == UNEXAMINED)
            return 0;

    return 1;
}

int is_all_succs_in_mem(board_t *board) {
    for (int i = 0; i < board->fl->bf; i++)
        if (board->succs_states[i] != EXISTS)
            return 0;

    return 1;
}

void sma_backup(board_t* board, tree_t* open_tree) {
    if (!board->parent) return;

    int min_f = INT_MAX;

    for (int i = 0; i < board->fl->bf; i++)
        if (board->succs_states[i] == EXISTS && board->succs[i]->f < min_f)
            min_f = board->succs[i]->f;

    if (min_f == INT_MAX)
        return;

    if (min_f != board->f)
        sma_backup(board->parent, open_tree);

    board->f = min_f;

    if (board->open_node) {
        tree_delete(open_tree, board->open_node);
        board->open_node = tree_insert(
            open_tree,
            board->f,
            board->p,
            board
        );
    }

    return;
}

int heuristic(board_t* board) {
    int h;
    int total_cells = board->fl->n * board->fl->m;

    int cells_flooded = 0;
    for (int i = 0; i < board->fl->n; i++)
        for (int j = 0; j < board->fl->m; j++)
            if (board->matrix[i][j].f)
                cells_flooded++;

    h = total_cells - cells_flooded;

    return h;
}
