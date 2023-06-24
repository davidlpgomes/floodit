#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "floodit.h"
#include "search.h"
#include "utils.h"

double time_ns = 0;
double time_paint = 0;
double time_searx = 0;
double time_h = 0;


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
        free_search_nodes(search, search->root);

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

    node->g = 0;
    node->h = 0;
    node->f = 0;

    node->succs = calloc(search->bf, sizeof(search_node_t*));
    test_alloc(node->succs, "node succs");

    node->succs_size = 0;

    node->succs_states = calloc(search->bf, sizeof(char));
    test_alloc(node->succs_states, "node succs states");

    node->colors_presence = malloc(sizeof(unsigned char) * search->flood->k);
    test_alloc(node->colors_presence, "node olors presence");

    for (int i = 0; i < 4; i++)
        node->q_count[i] = 1;

    #ifdef DEBUG
    printf("[FL] Search node created\n");
    #endif

    return node;
}

void free_search_nodes(search_t *search, search_node_t *node) {
    if (!node)
        return;

    for (int i = 0; i < search->bf; i++)
        if (node->succs[i])
            free_search_nodes(search, node->succs[i]);

    free_board(node->board);

    if (node->succs)
        free(node->succs);

    if (node->succs_states)
        free(node->succs_states);

    if (node->colors_presence)
        free(node->colors_presence);

    free(node);

    return;
}

search_node_t *next(search_t *search, search_node_t *parent, int i, int corner) {
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

    parent->succs_states[i] = EXISTS;
    parent->succs[i] = succ;
    parent->succs_size++;

    succ->parent = parent;
    succ->action.cor = corner + 97;
    succ->action.col = color;

    int r = -1;
    int c = -1;

    if (corner == UL || corner == UR) r = 0;
    else r = search->flood->n - 1;

    if (corner == UL || corner == LL) c = 0;
    else c = search->flood->m - 1;

    double start = timestamp();
    flood(search->flood, succ->board, color, r, c);
    time_paint += timestamp() - start;

    start = timestamp();
    succ->h = heuristic(succ);
    time_h += timestamp() - start;

    succ->g = parent->g + 1;
    succ->f = succ->g + succ->h;

    return succ;
}

search_node_t *test_ramification(
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
    best_down = test_ramification(search, succ, corner, depth - 1);
    best = best_down;

    double start;

    for (int i = 1; i < search->bf; i++) {
        start = timestamp();
        succ = next(search, node, i, corner);
        time_ns += timestamp() - start;

        if (!succ) break;

        if (!succ->h) {
            best = succ;
            break;
        }

        search->ng++;

        /*
        #ifndef DEBUG
        clear_terminal();
        print_board(search->flood, succ->board);
        usleep(100000);
        printf("\n");
        #endif
        */

        best_down = test_ramification(search, succ, corner, depth - 1);

        if (best_down->h < best->h) {
            free_search_nodes(search, best);
            best = best_down;
        } else
            free_search_nodes(search, best_down);

        #ifdef DEBUG
        printf(
            "[RAM] Best is d=%u h=%f g=%f cor=%d col=%d\n",
            depth, best->h, best->g, corner, best->action.col
        );
        #endif

        if (!best->h)
            break;
    }

    return best;
}

search_node_t *searx(search_t *search, int depth) {
    search_node_t *root = search->root, *best;

    #ifdef DEBUG
    printf("[FL][SX] Running searx\n");
    #endif

    double start = timestamp();

    int corner = UL;
    search_node_t *cor = test_ramification(search, root, UL, depth);
    best = cor;

    cor = test_ramification(search, root, UR, depth);
    if (cor->f < best->f) {
        free_search_nodes(search, best);
        best = cor;
        corner = UR;
    }

    cor = test_ramification(search, root, LR, depth);
    if (cor->f < best->f) {
        free_search_nodes(search, best);
        best = cor;
        corner = LR;
    }
        
    cor = test_ramification(search, root, LL, depth);
    if (cor->f < best->f) { 
        free_search_nodes(search, best);
        best = cor;
        corner = LL;
    }

    while (best->h) {
        #ifndef DEBUG
        clear_terminal();
        print_board(search->flood, best->board);
        usleep(50000);
        printf("\n");
        #endif
        best = test_ramification(search, best, corner, depth);
    }

    #ifndef DEBUG
    clear_terminal();
    print_board(search->flood, best->board);
    printf("\n");
    #endif

    time_searx = timestamp() - start;

    #ifdef DEBUG
    printf("\n\n---- TIMES SEARX\n");
    printf("\tTotal: %.15g\n", time_searx);
    printf("\tNx sc: %.15g\n", time_ns - time_paint);
    printf("\tPaint: %.15g\n", time_paint);
    printf("\tHeuri: %.15g\n", time_h);
    #endif

    return best;
}

search_node_t *search_node(search_t *search) {
    search_node_t *best = searx(search, 3);
    print_actions(best);

    return best;
}

int get_distance_from_corner(search_node_t *node, int corner) {
    board_t *b = node->board;

    int c_i = 0;
    int c_j = 0;

    int corner_painted = node->action.cor - 97; 

    if (corner == LR || corner == LL)
        c_i = b->fl->n - 1;

    if (corner == UR || corner == LR)
        c_j = b->fl->m - 1;

    if (b->matrix[c_i][c_j].cor == corner_painted)
        return 0;

    int min_distance = INT_MAX, dist;

    for (int i = 0; i < b->fl->n; i++) {
        for (int j = 0; j < b->fl->m; j++) {
            if (b->matrix[i][j].f && i != c_i && j != c_j) {
                dist = abs(i - c_i) + abs(j - c_j);

                if (dist < min_distance)
                    min_distance = dist;
            }
        }
    }

    return min_distance;
}

int get_distance_from_middle(search_node_t *node) {
    board_t *b = node->board;

    int i_m = b->fl->n / 2;
    int j_m = b->fl->m / 2;

    unsigned dist, min_distance = i_m + j_m;

    for (int i = 0; i < b->fl->n; i++) {
        for (int j = 0; j < b->fl->m; j++) {
            if (b->matrix[i][j].f) {
                dist = abs(i - i_m) + abs(j - j_m);

                if (dist < min_distance)
                    min_distance = dist;    
            }
        }
    }

    return min_distance;
}

double heuristic(search_node_t *node) {
    board_t *b = node->board;

    int total_cells = b->fl->n * b->fl->m;
    int cells_flooded = 0;

    for (int i = 0; i < b->fl->n; i++)
        for (int j = 0; j < b->fl->m; j++)
            if (b->matrix[i][j].f)
                cells_flooded++;

    int cells_not_flooded = total_cells - cells_flooded;
    int total_dist = get_distance_from_middle(node);

    if (total_dist)
        return total_cells * total_dist;

    return cells_not_flooded;
}

void print_actions(search_node_t *node) {
    printf("%d\n", (int) node->g);

    unsigned size = 2 * (node->g);

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
