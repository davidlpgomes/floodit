#include <stdio.h>
#include <stdlib.h>

#include "floodit.h"
#include "utils.h"

flood_t* create_flood(unsigned n, unsigned m, unsigned k) {
    flood_t* fl = malloc(sizeof(flood_t));
    test_alloc(fl, "flood");

    fl->n = n;
    fl->m = m;
    fl->k = k;

    fl->board = create_board(fl);

    #ifdef DEBUG
    printf("[FL] Flood created\n");
    #endif

    return fl;
}

void free_flood(flood_t* fl) {
    if (!fl)
        return;

    if (fl->board)
        free_board(fl->board);

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

    board->matrix = malloc(fl->n * sizeof(int*));
    test_alloc(board->matrix, "matrix pointers");

    board->matrix[0] = calloc(fl->n * fl->m, sizeof(cell_t));
    test_alloc(board->matrix[0], "board cells matrix");

    // Adjust pointers to the beggining of each line
    for (int i = 1; i < fl->n; i++)
        board->matrix[i] = board->matrix[0] + i * fl->m;

    // Set corners as flooded and mark the corner number
    board->matrix[0][0].f = 1;
    board->matrix[0][0].cor = 0;

    board->matrix[0][fl->m - 1].f = 1;
    board->matrix[0][fl->m - 1].cor = 1;

    board->matrix[fl->n - 1][0].f = 1;
    board->matrix[fl->n - 1][0].cor = 2;

    board->matrix[fl->n - 1][fl->m - 1].f = 1;
    board->matrix[fl->n - 1][fl->m - 1].cor = 3;

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

    free(board);

    #ifdef DEBUG
    printf("[FL] Board freed\n");
    #endif

    return;
}

void read_board(flood_t *flood, board_t* board) {
    unsigned c;

    for (int i = 0; i < flood->n; i++)
        for (int j = 0; j < flood->m; j++) {
            scanf("%u", &c);
            board->matrix[i][j].c = (unsigned char) c;
        }

    #ifdef DEBUG
    printf("[FL] Board data read\n");
    #endif

    return; 
}

void copy_board(flood_t *flood, board_t *destination, board_t *source) {
    int matrix_size = flood->n * flood->m;

    for (int i = 0; i < matrix_size; i++)
        destination->matrix[0][i] = source->matrix[0][i];

    return;
}

void print_board(flood_t *flood, board_t *board) {
    #ifdef DEBUG
    printf("[FL] Printing board...\n");
    #endif

    if (flood->k > 16)
        print_board_numbers(flood, board);
    else
        print_board_colors(flood, board);

    return;
}

void print_board_colors(flood_t *flood, board_t *board) {
    if (flood->k > 16)
        return;

    for (int i = 0; i < flood->n; i++) {
        for (int j = 0; j < flood->m; j++) {
            printf(ansi_bgs[board->matrix[i][j].c - 1]);
            printf("  " ANSI_COLOR_RESET);
        }

        printf("\n");
    }

    return;
}

void print_board_numbers(flood_t *flood, board_t *board) {
    for (int i = 0; i < flood->n; i++) {
        for (int j = 0; j < flood->m; j++)
            printf("%2u ", (unsigned) board->matrix[i][j].c);

        printf("\n");
    }

    return;
}

int is_flooded(flood_t *flood, board_t* board) {
    unsigned color = board->matrix[0][0].c;

    for (int i = 0; i < flood->n; i++)
        for (int j = 0; j < flood->m; j++)
            if (board->matrix[i][j].c != color)
                return 0;

    return 1;
}

void flood(
    flood_t *flood,
    board_t *board,
    unsigned char color,
    unsigned r,
    unsigned c
) {
    #ifdef DEBUG
    printf("[FL][PC] Flooding board, action %u:%u %u\n", r, c, color);
    #endif

    // It assumes that the color, row and columns params 
    // are valid for efficiency reasons

    if (board->matrix[r][c].c == color) return;

    unsigned char cor = board->matrix[r][c].cor;

    for (int i = 0; i < flood->n; i++)
        for (int j = 0; j < flood->m; j++) {
            if (board->matrix[i][j].f && board->matrix[i][j].cor == cor) {
                board->matrix[i][j].c = color;
                floodN(flood, board, color, cor, i, j);
            }
        }

    return;
}

void floodN(
    flood_t *flood,
    board_t *board,
    unsigned char color,
    unsigned char cor,
    unsigned r,
    unsigned c
) {
    if (r > 0)
        floodC(flood, board, color, cor, r - 1, c);

    if (r < flood->n - 1)
        floodC(flood, board, color, cor, r + 1, c);

    if (c > 0)
        floodC(flood, board, color, cor, r, c - 1);

    if (c < flood->m - 1)
        floodC(flood, board, color, cor, r, c + 1);

    return;
}

void floodC(
    flood_t *flood,
    board_t *board,
    unsigned char color,
    unsigned char cor,
    unsigned r,
    unsigned c
) {
    if (board->matrix[r][c].f && board->matrix[r][c].cor == cor)
        return;

    if (board->matrix[r][c].c == color) {
        board->matrix[r][c].f = 1;
        board->matrix[r][c].cor = cor;

        floodN(flood, board, color, cor, r, c);
    }

    return;
}
