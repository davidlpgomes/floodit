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

    //clear_terminal();
    //print_board(board);
    //unsigned color;
    //unsigned corner;

    //board_t* rand_board = create_board(fl);
    //copy_board(rand_board, board);

    //printf("\nExecutando com aleatoriedade de ações...\n");
    //sleep(1);
    //while (!is_flooded(rand_board)) {
    //    color = rand() % fl->k + 1;
    //    corner = rand() % 5;
    //    usleep(10000);
    //    paint_corner(rand_board, corner, color);
    //    clear_terminal();
    //    print_board(rand_board);
    //}

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
