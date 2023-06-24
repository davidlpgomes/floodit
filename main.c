#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "floodit.h"
#include "search.h"
#include "utils.h"


int main() {
    srand(time(NULL));

    // Disable buffer for stdout
    setvbuf(stdout, 0, _IONBF, 0);

    unsigned n, m, k;

    #ifdef DEBUG
    printf("[FL] Flood it initialized\n");
    #endif

    scanf("%u %u %u", &n, &m, &k);
    #ifdef DEBUG
    printf("[FL] Parameters read: n=%u m=%u k=%u\n", n, m, k);
    #endif

    flood_t *fl = create_flood(n, m, k);
    read_board(fl, fl->board);

    // clear_terminal();
    // print_board(fl, fl->board);
    // unsigned color;
    // unsigned corner;

    // while (!is_flooded(fl, fl->board)) {
    //     for (int i = 1; i <= fl->k; i++) {
    //         printf("    %d) ", i);
    //         printf(ansi_bgs[i - 1]);
    //         printf("  " ANSI_COLOR_RESET);
    //     }
    //     printf("\nChoose color: ");
    //     scanf("%u", &color);

    //     printf("\n1) UL  2) UR  3) LR  4) LL, choose: ");
    //     scanf("%u", &corner);

    //     usleep(100000);
    //     int r = 0, c = 0;
    //     if (corner == 2 || corner == 3)
    //         c = fl->m - 1;
    //     if (corner == 3 || corner == 4)
    //         r = fl->n - 1;
    //     flood(fl, fl->board, color, r, c);
    //     clear_terminal();
    //     print_board(fl, fl->board);
    // }
    // return 0;

    clear_terminal();
    search_t *search = create_search(fl);
    search_node_t *best = search_node(search);

    if (!best)
        fprintf(stderr, "Could not find path\n");
    else {
        printf("Busca (baseline) - passos: %d\n", (int) best->g);
        printf("    Nodos gerados: %u\n", search->ng);
    }

    free_search(search);
    free_flood(fl);

    #ifdef DEBUG
    printf("[FL] Flood it finished\n");
    #endif

    return 0;
}
