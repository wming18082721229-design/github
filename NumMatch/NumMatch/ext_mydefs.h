#pragma once

/* Put other #includes here,
   your struct board, helper function
   prototypes etc.
*/

#include <string.h>
#define HEIGHT 4
#define WIDTH 5
#define BIGSIZE 100000
#define MATCH ((p->b[z.y1][z.x1] == p->b[z.y2][z.x2]) || (p->b[z.y1][z.x1]+p->b[z.y2][z.x2] == 10)) && (p->b[z.y1][z.x1]!=0 && p->b[z.y2][z.x2]!=0)
#define MAX (1 << HEIGHT * WIDTH)

struct board {
   int b[HEIGHT][WIDTH];
   int overview;
};
typedef struct board board;

bool check_horizontal_gap(board* p, pair z);
bool check_vertical_gap(board* p, pair z);
bool check_diagonal_gap(board* p, pair z);
bool is_connected(board* p, pair z);
bool is_unique(const board *tmp);
bool is_final(board* p);
