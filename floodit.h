#ifndef __FLOODIT__
#define __FLOODIT__

typedef enum { UL, UR, LR, LL } corner_t;

typedef struct cell_t {
    unsigned char c;   // Color
    char f;            // Flooded
    unsigned char cor; // Indicates which corner it was flooded by
} cell_t;

typedef struct board_t {
    struct flood_t *fl;    // Pointer to Floodit game
    cell_t **matrix;       // Matrix with the board cells
    unsigned cells_not_flooded;
} board_t;

typedef struct flood_t {
    unsigned n;           // Number of lines
    unsigned m;           // Number of columns
    unsigned k;           // Number of colors

    board_t *board;
} flood_t;

flood_t *create_flood(unsigned n, unsigned m, unsigned k);

void free_flood(flood_t *fl);

board_t* create_board(flood_t *fl);

void free_board(board_t *board);

void read_board(flood_t *flood, board_t *board);

void copy_board(flood_t *flood, board_t *destination, board_t *source);

void print_board(flood_t *flood, board_t *board);

void print_board_colors(flood_t *flood, board_t *board);

void print_board_numbers(flood_t *flood, board_t *board);

int is_flooded(flood_t *flood, board_t *board);

void flood(
    flood_t *flood,
    board_t *board,
    unsigned char color,
    unsigned r,
    unsigned c
);

void floodN(
    flood_t *flood,
    board_t *board,
    unsigned char color,
    unsigned char cor,
    unsigned r,
    unsigned c
);

void floodC(
    flood_t *flood,
    board_t *board,
    unsigned char color,
    unsigned char cor,
    unsigned r,
    unsigned c
);

#endif
