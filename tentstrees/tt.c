#include "tentstrees.h"
#define EDGE (BSIZE - 1)

bool check_input(char* s, char* top, char* side, board* p);
bool check_tents(board* p);
bool check_equal(board* new_board, board* old_board);
bool check_solve(board* p);
void rule1(board* p);
void rule2(board* p);
void solve_inner_rule2(board* p);
void solve_edges_rule2(board* p);
void solve_corners_rule2(board* p);
void rule3(board* p);
void rule4(board* p);
void solve_inner_rule4(board* p);
void solve_edges_rule4(board* p);
void solve_corners_rule4(board* p);
void rule5(board* p);
void solve_inner_rule5(board* p);
void solve_edges_rule5(board* p);
void solve_corners_rule5(board* p);

void test(void){
   #if BSIZE == 2
      board test_b;
      char test_s[BSIZE*(BSIZE+1)];

      assert(inputboard("*N|  ", "11", "11", &test_b)==false);
      assert(inputboard("*.|  ", "11", "11", &test_b)==false);
      assert(inputboard("* | *", "11", "10", &test_b)==false);

      assert(inputboard("* |  ", "10", "01", &test_b)==true);
      assert(solve(&test_b)==true);
      assert(board2str(test_b,test_s));
      assert(strcmp("*.|+.", test_s)==0);

      assert(inputboard("**|**", "22", "22", &test_b)==true);
      assert(solve(&test_b)==false);
   #endif


   #if BSIZE == 3
      board test_b;
      char test_s[BSIZE*(BSIZE+1)];

      assert(inputboard(" * | N |   ", "111", "111", &test_b)==false);
      assert(inputboard(" * | . |   ", "111", "111", &test_b)==false);
      assert(inputboard(" * | * |   ", "111", "112", &test_b)==false);

      assert(inputboard("*  |  *|   ", "101", "110", &test_b)==true);
      assert(solve(&test_b)==true);
      assert(board2str(test_b,test_s));
      assert(strcmp("*.+|+.*|...", test_s)==0);

      assert(inputboard("   |   |   ", "000", "000", &test_b)==true);
      assert(solve(&test_b)==true);
      assert(board2str(test_b,test_s));
      assert(strcmp("...|...|...", test_s)==0);

      assert(inputboard("***|***|***", "111", "111", &test_b)==true);
      assert(solve(&test_b)==false);
   #endif

   #if BSIZE == 6
      board test_b;
      char test_s[BSIZE*(BSIZE+1)];

      assert(inputboard(NULL, "302002", "111211", &test_b)==false);
      assert(inputboard("   *  |      |*   * | * *  |*   * |      ", NULL, "111211", &test_b)==false);
      assert(inputboard("   *  |      |*   * | * *  |*   * |      ", "302002", "111211", NULL)==false);

      assert(inputboard("   *  |      |*   * | * *  |*   * |      ", "302002", "111211", &test_b)==true);
      assert(solve(&test_b)==true);
      assert(board2str(test_b,test_s));
      assert(strcmp("..+*..|+.....|*...*+|+*+*..|*...*+|+.....", test_s)==0);

      assert(inputboard("      |      |      |      |      |      ", "000000", "000000", &test_b)==true);
      assert(solve(&test_b)==true);
      assert(board2str(test_b,test_s));
      assert(strcmp("......|......|......|......|......|......", test_s)==0);
   #endif

   #if BSIZE == 8
      board b;
      char s[BSIZE * BSIZE + (BSIZE - 1) + 1];

      assert(inputboard("    *   |   *  * |**      |       *|        |  *   * |    *   |  * **  ", "12131022", "21211203", &b) == true);
      assert(solve(&b) == true);
      assert(board2str(b, s));
      assert(strcmp("...+*.+.|.+.*..*.|**.+...+|+......*|..+.....|..*.+.*+|....*...|.+*+**+.", s) == 0);
   #endif
}

bool inputboard(char* s, char* top, char* side, board* p){
   //check the validity of inputs
   if(check_input(s, top, side, p) == false){
      return false;
   }
   //pass inputs to the board
   for(int i=0; i<BSIZE; i++){
      p->tents_col[i] = top[i] - '0';
      p->tents_row[i] = side[i] - '0';
   }
   for(int i=0, j=0, k=0; i<(int)strlen(s); i++){
      if(s[i] != '|'){
         p->f[j][k] = s[i];
         k++;
      }
      if(s[i] == '|'){
         if(k != BSIZE){
            return false;
         }
         j++;
         k = 0;
      }
   }
   /*check if the total number of tents in the rows 
   is different to that in the columns*/
   if(check_tents(p) == 0){
      return false;
   }

   return true;
}

bool solve(board* p){
   board old_board;
   /*solve the puzzle by appling the five rules repeatedly 
   untill no change has been made*/
   do{
      old_board = *p;
      rule1(p);
      rule2(p);
      rule3(p);
      rule4(p);
      rule5(p);
   }while(check_equal(p, &old_board) == false);
   //check if the puzzle was solved correctly
   if(check_solve(p) == true){
      return true;
   }

   return false;
}

bool board2str(board b, char* s){
   if(s == NULL){
      return false;
   }

   int i=0;
   for(int j=0; j<BSIZE; j++){
      for(int k=0; k<BSIZE; k++){
         s[i] = b.f[j][k];
         i++;
      }
      if(j != BSIZE-1){
         s[i] = '|';
         i++;
      }
   }
   //a NULL string causing a failure
   if(s == NULL){
      return false;
   }

   return true;
}

bool check_input(char* s, char* top, char* side, board* p){
   /*check if the inputs are NULL string/incorrect number of characters
   /invalid characters combination/invalid square*/
   if(s == NULL || top == NULL || side == NULL || p == NULL){
      return false;
   }

   int len = strlen(s);
   if(len != BSIZE*(BSIZE+1)-1){
      return false;
   }

   for(int i=0; i<len; i++){
      if(s[i] != ' ' && s[i] != '*' && s[i] != '|'){
         return false;
      }
      if(s[i] == '|' && (i+1)%(BSIZE+1) != 0){
         return false;
      }
   }

   if(strlen(top) != BSIZE || strlen(side) != BSIZE){
      return false;
   }

   for(int i=0; i<BSIZE; i++){
      if(isdigit(top[i]) == 0 || isdigit(side[i]) == 0) {
         return false;
      }
   }

   return true;
}

/*check if the total number of tents in the rows 
is different to that in the columns*/
bool check_tents(board* p){
   int total_col = 0, total_row = 0;
   for(int i=0; i<BSIZE; i++){
      total_col = total_col + p->tents_col[i];
      total_row = total_row + p->tents_row[i];
   }

   if(total_col != total_row){
      return false;
   }

   return true;
}

//check if the two last processed boards are the same
bool check_equal(board* new_board, board* old_board){
   for(int i = 0; i < BSIZE; i++){
      for(int j = 0; j < BSIZE; j++){
         if(new_board->f[i][j] != old_board->f[i][j]){
            return false;
         }
      }
   }

   return true;
}

bool check_solve(board* p){
   //check if there is any unknown cell
   for(int j=0; j<BSIZE; j++){
      for(int k=0; k<BSIZE; k++){
         if(p->f[j][k] == ' '){
            return false;
         }
      }
   }
   //check if the number of tents is correct
   int total_col = 0, total_row = 0, total_tents = 0, total_trees = 0;
   for(int i=0; i<BSIZE; i++){
      total_col = total_col + p->tents_col[i];
      total_row = total_row + p->tents_row[i];
   }
   for(int j=0; j<BSIZE; j++){
      for(int k=0; k<BSIZE; k++){
         if(p->f[j][k] == '+'){
            total_tents++;
         }
         if(p->f[j][k] == '*'){
            total_trees++;
         }
      }
   }
   if(total_tents == total_col && total_tents == total_row && total_tents == total_trees){
      return true;
   }

   return false;
}

/*If a row or column already has
 the correct number of tents, all unknowns → grass*/
void rule1(board* p){
   for(int j=0; j<BSIZE; j++){
      int count_tents_col = 0, count_tents_row = 0;
      for(int k=0; k<BSIZE; k++){
         if(p->f[j][k] == '+'){
            count_tents_row ++;
         }
         if(p->f[k][j] == '+'){
            count_tents_col ++;
         }
      }

      if(count_tents_row == p->tents_row[j]){
         for(int i=0; i<BSIZE; i++){
            if(p->f[j][i] == ' '){
               p->f[j][i] = '.';
            }
         }
      }
      if(count_tents_col == p->tents_col[j]){
         for(int i=0; i<BSIZE; i++){
            if(p->f[i][j] == ' '){
               p->f[i][j] = '.';
            }
         }
      }
   }
}

//Any unknown cell not 4-connected to a tree → grass
void rule2(board* p){
   solve_inner_rule2(p);
   solve_edges_rule2(p);
   solve_corners_rule2(p);
}
 
void solve_inner_rule2(board* p){
   for(int j=1; j<=EDGE-1; j++){
       for(int k=1; k<=EDGE-1; k++){
            if(p->f[j][k] == ' ' && p->f[j+1][k] != '*' && p->f[j-1][k] != '*' && p->f[j][k+1] != '*' && p->f[j][k-1] != '*'){
               p->f[j][k] = '.';
            }
       }
   }
}

void solve_edges_rule2(board* p){
   for(int i=1; i<EDGE; i++){
       if(p->f[0][i] == ' ' && p->f[0][i+1] != '*' && p->f[0][i-1] != '*' && p->f[1][i] != '*'){
            p->f[0][i] = '.';
       }
       if(p->f[i][0] == ' ' && p->f[i-1][0] != '*' && p->f[i+1][0] != '*' && p->f[i][1] != '*'){
            p->f[i][0] = '.';
       }
       if(p->f[EDGE][i] == ' ' && p->f[EDGE][i-1] != '*' && p->f[EDGE][i+1] != '*' && p->f[EDGE-1][i] != '*'){
            p->f[EDGE][i] = '.';
       }
       if(p->f[i][EDGE] == ' ' && p->f[i-1][EDGE] != '*' && p->f[i+1][EDGE] != '*' && p->f[i][EDGE-1] != '*'){
            p->f[i][EDGE] = '.';
       }
   }
}

void solve_corners_rule2(board* p){
   if(p->f[0][0] == ' ' && p->f[0][1] != '*' && p->f[1][0] != '*'){
       p->f[0][0] = '.';
   }
   if(p->f[0][EDGE] == ' ' && p->f[0][EDGE-1] != '*' && p->f[1][EDGE] != '*'){
       p->f[0][EDGE] = '.';
   }
   if(p->f[EDGE][0] == ' ' && p->f[EDGE-1][0] != '*' && p->f[EDGE][1] != '*'){
       p->f[EDGE][0] = '.';
   }
   if(p->f[EDGE][EDGE] == ' ' && p->f[EDGE-1][EDGE] != '*' && p->f[EDGE][EDGE-1] != '*'){
       p->f[EDGE][EDGE] = '.';
   }
}
/*if the number of unknown cells equals 
the number of tents required, all unknowns → tents*/
void rule3(board* p){
   for(int j=0; j<BSIZE; j++){
      int count_unknown_col = 0, count_unknown_row = 0, count_tents_col = 0, count_tents_row = 0;
         for(int k=0; k<BSIZE; k++){
            if(p->f[j][k] == ' '){
               count_unknown_row ++;
            }
            if(p->f[k][j] == ' '){
               count_unknown_col ++;
            }
            if(p->f[j][k] == '+'){
               count_tents_row ++;
            }
            if(p->f[k][j] == '+'){
               count_tents_col ++;
            }
         }

      if(count_unknown_row == p->tents_row[j] - count_tents_row){
         for(int i=0; i<BSIZE; i++){
            if(p->f[j][i] == ' '){
               p->f[j][i] = '+';
            }
         }
      }
      if(count_unknown_col == p->tents_col[j] - count_tents_col){
         for(int i=0; i<BSIZE; i++){
            if(p->f[i][j] == ' '){
               p->f[i][j] = '+';
            }
         }
      }
   }
}

//Any unknown cell 8-connected to a tent → grass
void rule4(board* p){
   solve_inner_rule4(p);
   solve_edges_rule4(p);
   solve_corners_rule4(p);
}

void solve_inner_rule4(board* p){
   for(int j=1; j<=EDGE-1; j++){
      for(int k=1; k<=EDGE-1; k++){
         if(p->f[j][k] == ' ' && (p->f[j-1][k-1] == '+' || p->f[j-1][k] == '+' || p->f[j-1][k+1] == '+' || p->f[j][k-1] == '+' || p->f[j][k+1] == '+' || p->f[j+1][k-1] == '+' || p->f[j+1][k] == '+' || p->f[j+1][k+1] == '+')){
            p->f[j][k] = '.';
         }
      }
   }
}

void solve_edges_rule4(board* p){
   for(int i=1; i<=EDGE-1; i++){
      if(p->f[0][i] == ' ' && (p->f[0][i-1] == '+' || p->f[0][i+1] == '+' || p->f[1][i-1] == '+' || p->f[1][i] == '+' || p->f[1][i+1] == '+')){
         p->f[0][i] = '.';
      }
      if(p->f[i][0] == ' ' && (p->f[i-1][0] == '+' || p->f[i+1][0] == '+' || p->f[i-1][1] == '+' || p->f[i][1] == '+' || p->f[i+1][1] == '+')){
         p->f[i][0] = '.';
      }
      if(p->f[EDGE][i] == ' ' && (p->f[EDGE][i-1] == '+' || p->f[EDGE][i+1] == '+' || p->f[EDGE-1][i-1] == '+' || p->f[EDGE-1][i] == '+' || p->f[EDGE-1][i+1] == '+')){
         p->f[EDGE][i] = '.';
      }
      if(p->f[i][EDGE] == ' ' && (p->f[i-1][EDGE] == '+' || p->f[i+1][EDGE] == '+' || p->f[i-1][EDGE-1] == '+' || p->f[i][EDGE-1] == '+' || p->f[i+1][EDGE-1] == '+')){
         p->f[i][EDGE] = '.';
      }
   }
}

void solve_corners_rule4(board* p){
   if(p->f[0][0] == ' ' && (p->f[0][1] == '+' || p->f[1][0] == '+' || p->f[1][1] == '+')){
      p->f[0][0] = '.';
   }
   if(p->f[0][EDGE] == ' ' && (p->f[0][EDGE-1] == '+' || p->f[1][EDGE-1] == '+' || p->f[1][EDGE] == '+')){
      p->f[0][EDGE] = '.';
   }
   if(p->f[EDGE][0] == ' ' && (p->f[EDGE-1][0] == '+' || p->f[EDGE-1][1] == '+' || p->f[EDGE][1] == '+')){
      p->f[EDGE][0] = '.';
   }
   if(p->f[EDGE][EDGE] == ' ' && (p->f[EDGE-1][EDGE-1] == '+' || p->f[EDGE-1][EDGE] == '+' || p->f[EDGE][EDGE-1] == '+')){
      p->f[EDGE][EDGE] = '.';
   }
}

/*For any tree and its 4-connected cells, 
if there is exactly one unknown and zero tents, then that unknown → tent*/
void rule5(board* p){
   solve_inner_rule5(p);
   solve_edges_rule5(p);
   solve_corners_rule5(p);
}

void solve_inner_rule5(board* p){
   for(int j=1; j<=EDGE-1; j++){
      for(int k=1; k<=EDGE-1; k++){
         if(p->f[j][k] == '*'){
            int count_unknown = 0, count_tents = 0, a, b;
            if(p->f[j-1][k] == ' '){
               count_unknown++;
               a = j - 1;
               b = k;
               }
            if(p->f[j+1][k] == ' '){
               count_unknown++;
               a = j + 1;
               b = k;
            }
            if(p->f[j][k-1] == ' '){
               count_unknown++;
               a = j;
               b = k - 1;
            }
            if(p->f[j][k+1] == ' '){
               count_unknown++;
               a = j;
               b = k + 1;
            }
            if(p->f[j-1][k] == '+' || p->f[j+1][k] == '+' || p->f[j][k-1] == '+' || p->f[j][k+1] == '+'){
               count_tents++;
            }
            if(count_unknown == 1 && count_tents == 0){
               p->f[a][b] = '+';
            }
         }
      }
   }
}

void solve_edges_rule5(board* p){
   for(int i=1; i<=EDGE-1; i++){
      if(p->f[0][i] == '*'){
         int count_unknown = 0, count_tents = 0, a, b;
         if(p->f[0][i-1] == ' '){
            count_unknown++;
            a = 0;
            b = i-1;
         }
         if(p->f[0][i+1] == ' '){
            count_unknown++;
            a = 0;
            b = i+1;
         }
         if(p->f[1][i] == ' '){
            count_unknown++;
            a = 1;
            b = i;
         }
         if(p->f[0][i-1] == '+' || p->f[0][i+1] == '+' || p->f[1][i] == '+'){
            count_tents++;
         }
         if(count_unknown == 1 && count_tents == 0){
            p->f[a][b] = '+';
         }
      }
      if(p->f[i][0] == '*'){
         int count_unknown = 0, count_tents = 0, a, b;
         if(p->f[i-1][0] == ' '){
            count_unknown++;
            a = i-1;
            b = 0;
         }
         if(p->f[i+1][0] == ' '){
            count_unknown++;
            a = i+1;
            b = 0;
         }
         if(p->f[i][1] == ' '){
            count_unknown++;
            a = i;
            b = 1;
         }
         if(p->f[i-1][0] == '+' || p->f[i+1][0] == '+' || p->f[i][1] == '+'){
            count_tents++;
         }
         if(count_unknown == 1 && count_tents == 0){
            p->f[a][b] = '+';
         }
      }
      if(p->f[EDGE][i] == '*'){
         int count_unknown = 0, count_tents = 0, a, b;
         if(p->f[EDGE][i-1] == ' '){
            count_unknown++;
            a = EDGE;
            b = i-1;
         }
         if(p->f[EDGE][i+1] == ' '){
            count_unknown++;
            a = EDGE;
            b = i+1;
         }
         if(p->f[EDGE-1][i] == ' '){
            count_unknown++;
            a = EDGE-1;
            b = i;
         }
         if(p->f[EDGE][i-1] == '+' || p->f[EDGE][i+1] == '+' || p->f[EDGE-1][i] == '+'){
            count_tents++;
         }
         if(count_unknown == 1 && count_tents == 0){
            p->f[a][b] = '+';
         }
      }
      if(p->f[i][EDGE] == '*'){
         int count_unknown = 0, count_tents = 0, a, b;
         if(p->f[i-1][EDGE] == ' '){
            count_unknown++;
            a = i-1;
            b = EDGE;
         }
         if(p->f[i+1][EDGE] == ' '){
            count_unknown++;
            a = i+1;
            b = EDGE;
         }
         if(p->f[i][EDGE-1] == ' '){
            count_unknown++;
            a = i;
            b = EDGE-1;
         }
         if(p->f[i-1][EDGE] == '+' || p->f[i+1][EDGE] == '+' || p->f[i][EDGE-1] == '+'){
            count_tents++;
         }
         if(count_unknown == 1 && count_tents == 0){
            p->f[a][b] = '+';
         }
      }
   }
}

void solve_corners_rule5(board* p){
   if(p->f[0][0] == '*'){
      if(p->f[0][1] != '+' && p->f[1][0] != '+'){
         if(p->f[0][1] == ' ' && p->f[1][0] != ' '){
            p->f[0][1] = '+';
         }
         if(p->f[0][1] != ' ' && p->f[1][0] == ' '){
            p->f[1][0] = '+';
         }
      }
   }
   if(p->f[0][EDGE] == '*'){
      if(p->f[0][EDGE-1] != '+' && p->f[1][EDGE] != '+'){
         if(p->f[0][EDGE-1] == ' ' && p->f[1][EDGE] != ' '){
            p->f[0][EDGE-1] = '+';
         }
         if(p->f[0][EDGE-1] != ' ' && p->f[1][EDGE] == ' '){
            p->f[1][EDGE] = '+';
         }
      }
   }
   if(p->f[EDGE][0] == '*'){
      if(p->f[EDGE-1][0] != '+' && p->f[EDGE][1] != '+'){
         if(p->f[EDGE-1][0] == ' ' && p->f[EDGE][1] != ' '){
            p->f[EDGE-1][0] = '+';
         }
         if(p->f[EDGE-1][0] != ' ' && p->f[EDGE][1] == ' '){
            p->f[EDGE][1] = '+';
         }
      }
   }

   if(p->f[EDGE][EDGE] == '*'){
      if(p->f[EDGE-1][EDGE] != '+' && p->f[EDGE][EDGE-1] != '+'){
         if(p->f[EDGE-1][EDGE] == ' ' && p->f[EDGE][EDGE-1] != ' '){
            p->f[EDGE-1][EDGE] = '+';
         }
         if(p->f[EDGE-1][EDGE] != ' ' && p->f[EDGE][EDGE-1] == ' '){
            p->f[EDGE][EDGE-1] = '+';
         }
      }
   }
}