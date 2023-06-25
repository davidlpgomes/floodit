#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "floodit.h"
#include "search.h"
#include "utils.h"


void flood_random(flood_t *fl) {
    if (!fl)
        return;

    board_t *b = fl->board;

    unsigned char color;
    unsigned char corner;
    unsigned r, c;

    int ei = fl->n - 1;
    int ej = fl->m - 1;

    clear_terminal();
    print_board(fl, b);

    unsigned steps = 0;

    while (!is_flooded(fl, fl->board)) {
        corner = random() % 4;

        if (corner == UL || corner == UR) r = 0;
        else r = ei;

        if (corner == UL || corner == LL) c = 0;
        else c = ej;

        color = (random() % fl->k) + 1;

        if (
            (corner == UL && color >= b->matrix[0][0].c) ||
            (corner == UR && color >= b->matrix[0][ej].c) ||
            (corner == LR && color >= b->matrix[ei][ej].c) ||
            (corner == LL && color >= b->matrix[ei][0].c)
        )
            color++;

        flood(fl, b, color, r, c); 
        steps++;

        usleep(1000);
        clear_terminal();
        print_board(fl, b);
    }

    printf("\nPassos: %u\n", steps);

    return;
}

int main(int argc, char **argv) {
    srand(time(NULL));

    // Disable buffer for stdout
    setvbuf(stdout, 0, _IONBF, 0);

    char print = 0, run_random = 0;

    int opt;
    while ((opt = getopt(argc, argv, "pr")) != -1) {
        switch (opt) {
            case 'p':
                print = 1;
                break;
            case 'r':
                run_random = 1;
                break;
            default:
                printf("Usage: %s [-p] [-r]", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    unsigned n, m, k;

    #ifdef DEBUG
    printf("[FL] Flood it initialized\n");
    #endif

    scanf("%u %u %u", &n, &m, &k);
    #ifdef DEBUG
    printf("[FL] Parameters read: n=%u m=%u k=%u\n", n, m, k);
    #endif

    if (!n || !m || !k) {
        fprintf(stderr, "Erro: parâmetros não podem ser nulos\n");
        exit(EXIT_FAILURE);
    }

    flood_t *fl = create_flood(n, m, k);
    read_board(fl, fl->board);

    if (run_random) {
        flood_random(fl);
        free_flood(fl);

        return 0;
    }

    int depth = 3;

    if (n > 70 || m > 70 || k >= 15)
        depth = 2;

    search_t *s = create_search(fl);
    search_node_t *best = search(s, depth, print);

    if (!best) {
        fprintf(stderr, "Could not find path\n");
        exit(1);
    }

    #ifdef DEBUG
    printf("Passos: %d\n", (int) best->d);
    printf("Nodos gerados: %u\n", s->ng);
    #endif

    print_actions(best);

    free_search_nodes(best);
    free_search(s);
    free_flood(fl);

    #ifdef DEBUG
    printf("[FL] Flood it finished\n");
    #endif

    return 0;
}
