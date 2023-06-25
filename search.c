#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "floodit.h"
#include "search.h"
#include "utils.h"

double time_deepening = 0;
double time_next = 0;
double time_flood = 0;
double time_f = 0;


search_t *create_search(flood_t *flood) {
    search_t *search = malloc(sizeof(search_t));
    test_alloc(search, "search");

    search->bf = flood->k - 1;
    search->ng = 0;

    search->flood = flood;
    search->root = create_search_node(search, NULL);
    copy_board(flood, search->root->board, flood->board);

    #ifdef DEBUG
    printf("[FL] Search created\n");
    #endif

    return search;
}

void free_search(search_t *search) {
    if (!search)
        return;

    if (search->root)
        free_search_node(search->root);

    free(search);

    #ifdef DEBUG
    printf("[FL] Search freed\n");
    #endif

    return;
}

search_node_t *create_search_node(search_t *search, search_node_t *parent) {
    if (!search)
        return NULL;

    search_node_t *node = malloc(sizeof(search_node_t));
    test_alloc(node, "search node");

    node->board = create_board(search->flood);
    node->parent = parent;

    node->action.cor = 64;
    node->action.col = 0;

    node->f = 0;
    node->d = 0;

    #ifdef DEBUG
    printf("[FL] Search node created\n");
    #endif

    return node;
}

void free_search_node(search_node_t *node) {
    if (!node)
        return;

    free_board(node->board);

    free(node);

    return;
}

void free_search_nodes_n(search_node_t *node, int n) {
    if (!n || !node || !node->parent)
        return;

    search_node_t *p = node->parent;
    free_search_node(node);

    free_search_nodes_n(p, n - 1);

    return;
}

void free_search_nodes(search_node_t *node) {
    if (!node || !node->parent)
        return;

    search_node_t *p = node->parent;
    free_search_node(node);

    free_search_nodes(p);

    return;
}

search_node_t *next(search_t *search, search_node_t *parent, int i, int corner) {
    double start = timestamp();

    board_t *b = parent->board;

    unsigned color = i + 1;

    unsigned ei = search->flood->n - 1;
    unsigned ej = search->flood->m - 1;

    if (
        (corner == UL && color >= b->matrix[0][0].c) ||
        (corner == UR && color >= b->matrix[0][ej].c) ||
        (corner == LR && color >= b->matrix[ei][ej].c) ||
        (corner == LL && color >= b->matrix[ei][0].c)
    )
        color++;

    #ifdef DEBUG
    printf(
        "[FL][SX][NX] Generating succ i=%d cor=%d col=%u\n",
        i,
        corner,
        color
    );
    #endif

    search_node_t *succ = create_search_node(search, parent);
    copy_board(search->flood, succ->board, parent->board);

    succ->parent = parent;
    succ->action.cor = corner + 97;
    succ->action.col = color;

    int r = -1;
    int c = -1;

    if (corner == UL || corner == UR) r = 0;
    else r = search->flood->n - 1;

    if (corner == UL || corner == LL) c = 0;
    else c = search->flood->m - 1;

    succ->d = parent->d + 1;

    time_next += timestamp() - start;

    start = timestamp();
    flood(search->flood, succ->board, color, r, c);
    time_flood += timestamp() - start;

    start = timestamp();
    succ->f = f(succ);
    time_f += timestamp() - start;

    return succ;
}

search_node_t *deepening(
    search_t *search,
    search_node_t *node,
    int corner,
    unsigned depth
) {
    #ifdef DEBUG
    printf(
        "[FL][SX][TR] Running test ramification with node=%p cor=%u d=%u\n",
        node, corner, depth
    );
    #endif

    if (!node)
        return NULL;

    if (!depth)
        return node;

    search_node_t *succ = NULL;
    search_node_t *best = NULL;
    search_node_t *best_down = NULL;

    succ = next(search, node, 0, corner);
    best_down = deepening(search, succ, corner, depth - 1);
    best = best_down;

    for (int i = 1; i < search->bf; i++) {
        succ = next(search, node, i, corner);
        search->ng++;

        if (!succ->f) {
            free_search_nodes_n(best, depth);
            best = succ;
            break;
        }

        best_down = deepening(search, succ, corner, depth - 1);

        if (best_down->f < best->f) {
            free_search_nodes_n(best, depth);
            best = best_down;
        } else
            free_search_nodes_n(best_down, depth);

        #ifdef DEBUG
        printf(
            "[RAM] Best is dep_d=%u f=%u depth=%u cor=%d col=%d\n",
            depth, best->f, best->d, corner, best->action.col
        );
        #endif

        if (!best->f)
            break;
    }

    return best;
}

search_node_t *search(search_t *search, int depth, char print) {
    search_node_t *root = search->root, *best;

    #ifdef DEBUG
    printf("[FL][SX] Running searx\n");
    #endif

    double start = timestamp();

    int corner = UL;
    search_node_t *cor = deepening(search, root, UL, depth);
    best = cor;

    for (int c = UR; c <= LL; c++) {
        cor = deepening(search, root, c, depth);

        if (cor->f < best->f) {
            free_search_nodes_n(best, depth);
            best = cor;
            corner = c;
        }
    }

    while (best->f) {
        best = deepening(search, best, corner, depth);

        if (print) {
            clear_terminal();
            print_board(search->flood, best->board);
            usleep(50000);
            printf("\n");
        }
    }

    time_deepening = timestamp() - start;

    #ifdef DEBUG
    printf("\n\n---- TIMES\n");
    printf("\tTotal deep: %.15g\n", time_deepening);
    printf("\tNext: %.15g\n", time_next);
    printf("\tFlood: %.15g\n", time_flood);
    printf("\tCost fun: %.15g\n\n", time_f);
    #endif

    return best;
}

int get_distance_from_middle(search_node_t *node) {
    board_t *b = node->board;

    int i_m = b->fl->n / 2;
    int j_m = b->fl->m / 2;

    unsigned dist, min_distance = i_m + j_m;

    int corner = node->action.cor - 97;

    int i_beg = 0;
    int j_beg = 0;

    int i_end = i_m;
    int j_end = j_m;

    if (corner == LR || corner == LL) {
        i_beg = i_m;
        i_end = b->fl->n - 1;
    }

    if (corner == UR || corner == LR) {
        j_beg = j_m;
        j_end = b->fl->m - 1;
    }

    for (int i = i_beg; i <= i_end; i++) {
        for (int j = j_beg; j <= j_end; j++) {
            if (b->matrix[i][j].f) {
                dist = abs(i - i_m) + abs(j - j_m);

                if (dist < min_distance)
                    min_distance = dist;    
            }
        }
    }

    return min_distance;
}

unsigned f(search_node_t *node) {
    int total_dist = get_distance_from_middle(node);

    if (total_dist)
        return node->board->fl->n * node->board->fl->m * total_dist;

    return node->board->cells_not_flooded;
}

void print_actions(search_node_t *node) {
    printf("%d\n", (int) node->d);

    unsigned size = 2 * (node->d);

    char* action_chars = malloc(size * sizeof(unsigned char));
    test_alloc(action_chars, "actions chars");

    search_node_t *n = node;
    for (int i = 0; i < size; i += 2) {
        action_chars[i] = n->action.col;
        action_chars[i + 1] = n->action.cor;
        n = n->parent;
    }

    for (int i = size - 1; i >= 0; i -= 2)
       printf("%c %d ", action_chars[i], action_chars[i - 1]);

    printf("\n");

    free(action_chars);

    return;
}

