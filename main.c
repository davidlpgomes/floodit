#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "floodit.h"
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

    flood_t* fl = create_flood(n, m, k);
    board_t* board = create_board(fl);
    read_board(board);

    /*
    clear_terminal();
    print_board(board);
    unsigned color;
    unsigned corner;

    while (!is_flooded(board)) {
        for (int i = 1; i <= board->fl->k; i++) {
            printf("    %d) ", i);
            printf(ansi_bgs[i - 1]);
            printf("  " ANSI_COLOR_RESET);
        }
        printf("\nChoose color: ");
        scanf("%u", &color);

        printf("\n1) UL  2) UR  3) LR  4) LL, choose: ");
        scanf("%u", &corner);

        usleep(10000);
        int r = 0, c = 0;
        if (corner == 2 || corner == 3)
            c = board->fl->m - 1;
        if (corner == 3 || corner == 4)
            r = board->fl->n - 1;
        flood(board, color, r, c);
        clear_terminal();
        print_board(board);
    }
    return 0;
    */
    

    //clear_terminal();
    //printf("Executando com máquina de busca (heurística baseline)...\n");
    board_t* best = sma(board);
    if (!best)
        fprintf(stderr, "Could not find optimum\n");
    else {
        //printf("\nRANDOM           - passos: %d\n", rand_board->p);
        printf("Busca (baseline) - passos: %d\n", best->p);
        printf("    Nodos gerados: %u\n", fl->ng);
    }

    free_board(board);
    free_flood(fl);

    #ifdef DEBUG
    printf("[FL] Flood it finished\n");
    #endif

    return 0;
}
