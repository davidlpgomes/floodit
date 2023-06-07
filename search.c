#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "search.h"
#include "utils.h"

double time_sma = 0;
double time_if = 0;
double time_ns = 0;
double time_paint = 0;
double time_h = 0;
double time_bk = 0;
double time_hc = 0;


search_t *create_search(flood_t *flood) {
    search_t *search = malloc(sizeof(search_t));
    test_alloc(search, "search");

    search->bf = 4 * (flood->k - 1);
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

    node->succs = malloc(sizeof(search_node_t*) * search->bf);
    test_alloc(node->succs, "node succs");

    node->succs_size = 0;

    node->succs_states = calloc(search->bf, sizeof(char));
    test_alloc(node->succs_states, "node succs states");

    node->colors_presence = malloc(sizeof(unsigned char) * search->flood->k);
    test_alloc(node->colors_presence, "node olors presence");

    for (int i = 0; i < 4; i++)
        node->q_count[i] = 1;

    node->open_node = NULL;
    node->leaves_node = NULL;

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

    #ifdef DEBUG
    printf("[FL] Search node freed\n");
    #endif

    return;
}

double heuristic(search_node_t *node) {
    board_t *b = node->board;

    double h = 0;

    int total_cells = b->fl->n * b->fl->m;
    int cells_flooded = 0;

    for (int i = 0; i < b->fl->n; i++)
        for (int j = 0; j < b->fl->m; j++)
            if (b->matrix[i][j].f)
                cells_flooded++;

    int cells_not_flooded = total_cells - cells_flooded;

    int i_middle = (b->fl->n - 1) / 2;
    int j_middle = (b->fl->m - 1) / 2;
    unsigned total_dist = 0;

    for (int i = 0; i < 4; i++)
        total_dist += find_closest_flooded_cell(
            node, i_middle, j_middle, i
        );

    int num_cors = 0;

    if (b->matrix[0][0].cor == 0) num_cors++;
    if (b->matrix[0][b->fl->m - 1].cor == 1) num_cors++;
    if (b->matrix[b->fl->n - 1][b->fl->m - 1].cor == 2) num_cors++;
    if (b->matrix[b->fl->n - 1][0].cor == 3) num_cors++;

    // int perimeter_sum = 0;
    // for (int i = 0; i < b->fl->k; i++)
    //     perimeter_sum += node->colors_presence[i];
    // int avg = perimeter_sum / b->fl->k;
    // int perimeter_diff = 0;
    // for (int i = 0; i < b->fl->k; i++)
    //     perimeter_diff += abs(node->colors_presence[i] - avg);

    // int colors_present = 0;
    // for (int i = 0; i < b->fl->k; i++)
    //     if (node->colors_presence[i])
    //         colors_present++;

    // if (total_dist > 1)
    //     h = 100000 * (num_cors + 1) * total_dist;
    // else {
    //     double mid_game = 100 * (cells_not_flooded + 0.4 * perimeter_diff);
    //     double end_game = cells_not_flooded + 100 * (colors_present - 1);
    //     h = max(mid_game, end_game);
    // }
    h = cells_not_flooded;

    return h;
}

unsigned find_closest_flooded_cell(search_node_t *node, int i, int j, int cor) {
    board_t *b = node->board;

    int c_i = 0;
    int c_j = 0;

    if (cor == 1 || cor == 2)
        c_j = b->fl->m - 1;

    if (cor == 2 || cor == 3)
        c_i = b->fl->n - 1;

    int corner = b->matrix[c_i][c_j].cor;
    unsigned dist, min_distance = abs(c_i - i) + abs(c_j - j);

    for (int x = 0; x < b->fl->n; x++) {
        for (int y = 0; y < b->fl->m; y++) {
            if (
                b->matrix[x][y].f &&
                b->matrix[x][y].cor == corner
            ) {
                dist = abs(x - i) + abs(y - j);

                if (dist < min_distance) {
                    c_i = x;
                    c_j = y;
                    min_distance = dist;
                }
            }
        }
    }

    return min_distance;
}

search_node_t *sma(search_t *search) {
    tree_t *open = tree_create();
    tree_t *leaves = tree_create();

    int used = 1;
    int state = 0;

    search_node_t *root = search->root;

    root->h = heuristic(root);
    root->f = root->h;

    root->open_node = tree_insert(
        open,
        root->f,
        root->g,
        root
    );

    root->leaves_node = tree_insert(
        leaves,
        root->f,
        root->g,
        root
    );

    search_node_t *best, *succ, *highest_cost;
    double start;

    #ifdef DEBUG
    printf("[FL] Initializing SMA*\n");
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
            "\n[FL][SMA*][i=%d] #open is %d, #leaves is %d, used is %d\n",
            i,
            open->size,
            leaves->size,
            used
        );
        #endif

        best = (search_node_t*) tree_minimum(open, open->root)->v;

        #ifdef DEBUG
        printf(
            "[FL][SMA*] Best is %c%u g=%u h=%.5g f=%.5g\n",
            best->action.cor,
            best->action.col,
            (int) best->g,
            best->h,
            best->f
        );
        #endif

        start = timestamp();
        if (is_flooded(search->flood, best->board)) {
            #ifdef DEBUG
            printf("[FL][SMA*][i=%d] Best is flooded, returning best\n", i);
            #endif
            print_actions(best);
            state = 0;
            break;
        }
        time_if += timestamp() - start;

        search->ng += 1;

        start = timestamp();
        succ = next_successor(search, best);
        time_ns += timestamp() - start;

        if (!succ) {
            fprintf(stderr, "Error: succ is NULL\n");
            exit(1);
        }

        #ifndef DEBUG
        clear_terminal();
        print_board(search->flood, succ->board);
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
            "[FL][SMA*] Succ created %c%u g=%u h=%.5g f=%.5g\n",
            succ->action.cor,
            succ->action.col,
            (int) succ->g,
            succ->h,
            succ->f
        );
        #endif

        if (best->succs_size && best->leaves_node) {
            tree_delete(leaves, best->leaves_node);
            best->leaves_node = NULL;
        }

        if (is_completed(search, best)) {
            #ifdef DEBUG
            printf("[FL][SMA*][i=%d] Best is completed, backing up\n", i);
            #endif

            start = timestamp();
            sma_backup(search, best, open);
            time_bk += timestamp() - start;
        }

        if (is_all_succs_in_mem(search, best)) {
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
            highest_cost = (search_node_t*) tree_maximum(leaves, leaves->root)->v;

            if (!highest_cost) {
                fprintf(stderr, "[FL][SMA*] Error: highest cost is NULL, i.e., max states is not enough to solve the instance\n");
                exit(1);
            }

            #ifdef DEBUG
            printf(
                "[FL][SMA*] HC is %c%u g=%d h=%.5g f=%.5g\n",
                highest_cost->action.cor,
                highest_cost->action.col,
                (int) highest_cost->g,
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

            remove_from_parent_succs(search, highest_cost);

            search_node_t* parent = highest_cost->parent;
            if (!parent->succs_size) {
                parent->open_node = tree_insert(
                    open,
                    parent->f,
                    parent->g,
                    parent
                );

                parent->leaves_node = tree_insert(
                    leaves,
                    parent->f,
                    parent->g,
                    parent
                );
            }

            free_search_nodes(search, highest_cost);

            #ifdef DEBUG
            printf("[FL][SMA*] Max states, highest cost removed\n");
            #endif

            used--;
        }
        time_hc += timestamp() - start;

        #ifdef DEBUG
        printf("[FL][SMA*] Inserting succ on open/leaves trees\n");
        #endif

        succ->open_node = tree_insert(open, succ->f, succ->g, succ);
        succ->leaves_node = tree_insert(leaves, succ->f, succ->g, succ);

        i++;
    }

    tree_free(leaves);
    tree_free(open);

    time_sma = timestamp() - start_sma;

    #ifdef DEBUG
    printf("\n\n---- TIMES SMA*\n");
    printf("\tTotal: %.15g\n", time_sma);
    printf("\tIs Fl: %.15g\n", time_if);
    printf("\tNx sc: %.15g\n", time_ns - time_paint);
    printf("\tPaint: %.15g\n", time_paint);
    printf("\tHeuri: %.15g\n", time_h);
    printf("\tBckup: %.15g\n", time_bk);
    printf("\tHighc: %.15g\n", time_hc);
    #endif

    if (state)
        return NULL;

    return best;
}

search_node_t *next_successor(search_t *search, search_node_t *parent) {
    int index_to_generate = -1;
    for (int i = 0; i < search->bf; i++)
        if (parent->succs_states[i] == EXISTS)
            index_to_generate = i;

    index_to_generate++;

    if (index_to_generate == search->bf)
        index_to_generate = 0;

    #ifdef DEBUG
    printf(
        "[FL][SMA*][NS] Index to generate is %d\n",
        index_to_generate
    );
    #endif

    board_t *b = parent->board;

    int corner = index_to_generate / (search->flood->k - 1);
    unsigned color = index_to_generate - corner * (search->flood->k - 1) + 1;

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
        "[FL][SMA*][NS] Generating succ i=%d cor=%d col=%u\n",
        index_to_generate,
        corner,
        color
    );
    #endif

    search_node_t *succ = create_search_node(search, parent);
    copy_board(search->flood, succ->board, parent->board);

    parent->succs_states[index_to_generate] = EXISTS;
    parent->succs[index_to_generate] = succ;
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

    double start_pc = timestamp();
    flood(search->flood, succ->board, color, r, c);
    time_paint += timestamp() - start_pc;

    return succ;
}

void sma_backup(search_t *search, search_node_t *node, tree_t *open_tree) {
    if (!search || !node || !open_tree)
        return;

    if (!node->parent)
        return;

    int min_f = INT_MAX;

    for (int i = 0; i < search->bf; i++)
        if (node->succs_states[i] == EXISTS && node->succs[i]->f < min_f)
            min_f = node->succs[i]->f;

    if (min_f == INT_MAX)
        return;

    if (min_f != node->f)
        sma_backup(search, node->parent, open_tree);

    node->f = min_f;

    if (node->open_node) {
        tree_delete(open_tree, node->open_node);

        node->open_node = tree_insert(
            open_tree,
            node->f,
            node->g,
            node
        );
    }

    return;
}

int is_all_succs_in_mem(search_t *search, search_node_t *node) {
    for (int i = 0; i < search->bf; i++)
        if (node->succs_states[i] != EXISTS)
            return 0;

    return 1;
}

int is_completed(search_t *search, search_node_t *node) {
    for (int i = 0; i < search->bf; i++)
        if (node->succs_states[i] == UNEXAMINED)
            return 0;

    return 1;
}

void remove_from_parent_succs(search_t *search, search_node_t *node) {
    if (!node->parent)
        return;

    int i;
    for (i = 0; i < search->bf; i++)
        if (node->parent->succs[i] == node)
            break;

    if (node->parent->succs[i] != node) {
        #ifdef DEBUG
        printf("[FL][SMA*] Board is not registered in parent\n");
        #endif
        return;
    }

    node->parent->succs_states[i] = FORGOTTEN;
    node->parent->succs[i] = NULL;
    node->parent->succs_size -= 1;

    return;
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
