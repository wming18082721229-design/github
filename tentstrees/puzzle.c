#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

#define BSIZE 6

enum validchars {outofbounds='\0', unknown=' ', grass='.', tent='+', tree='*', sep='|'};
typedef enum validchars validchars;

struct board{
   // Tent Count Across the top / per Column
   unsigned short tents_col[BSIZE];
   // The field, square
   char f[BSIZE][BSIZE];
   // Tent Count Down the side / per Row
   unsigned short tents_row[BSIZE];
};
typedef struct board board;

void test(void);
bool inputboard(char* s, char* top, char* side, board* p);
bool solve(board* p);
bool board2str(board b, char* s);

bool check_input(char* s, char* top, char* side);

bool check_tents(board* p);

bool board_equal(board* new_board,board* old_board);

bool check_solve(board* p);

bool rule1(board* p);
bool rule2(board* p);
bool rule3(board* p);
bool rule4(board* p);
bool rule5(board* p);


int main(){
   test();
    board test_b;
    char test_s[BSIZE*(BSIZE+1)];

      inputboard("*    *|    * |      |  *  *|     +| **   ","120121", "201112", &test_b);
      solve(&test_b);
      board2str(test_b,test_s);
      for(int i=0;i<BSIZE*(BSIZE+1);i++){
         printf("%c",test_s[i]);
      }
      printf("\n");
      printf("*+..+*|....*.|....+.|.+*..*|.....+|+**+..");
      if(strcmp("*+..+*|....*.|....+.|.+*..*|.....+|+**+..", test_s)==0) printf("passed\n");
      
    /*char cmp[BSIZE*(BSIZE+1)] = "*.+|+. *|...";
    for(int i=0; i<strlen(cmp); i++){
      if((i+1)%(BSIZE+1)==0){
         printf("|\n",i);
         //printf("%i row\n",i);
      }
      else{
         printf("%c",cmp[i]);
      }
      
    }*/
    printf("\nDONE\n");
}

void test(void)
{

}

bool inputboard(char* s, char* top, char* side, board* p)
{
    if(check_input(s, top, side) == 0){
        return false;
    }

    for(int i=0; i<BSIZE; i++){
        p->tents_col[i] = top[i] - '0';
        p->tents_row[i] = side[i] - '0';
    }

    for(int i=0, j=0, k=0; i<strlen(s); i++){
        if(s[i] != '|'){
            p->f[j][k] = s[i];
            printf("%c", p->f[j][k]);
            k++;
        }
        
        if(s[i] == '|'){
            printf("\n");
            if(k != BSIZE){
                return false;
            }
            j++;
            k = 0;
        }

    }
    printf("\n");

    if(check_tents(p) == 0){
        return false;
    }

    return true;
}

bool solve(board* p)
{
    //printf("enter solve\n");
    int i = 1;
    board old_board;
   do{
      old_board = *p;
      printf("第%i次遍历\n",i);
      rule1(p);
      i++;
      for(int j=0; j<BSIZE; j++){
         for(int k=0; k<BSIZE; k++){
            printf("%c",p->f[j][k]);
         }
         if(j != BSIZE-1){
            printf("|\n");
         }
      }
      printf("\ndone rule 1\n");

        rule2(p);
        for(int j=0; j<BSIZE; j++){
         for(int k=0; k<BSIZE; k++){
            printf("%c",p->f[j][k]);
         }
         if(j != BSIZE-1){
            printf("|\n");
         }
      }
      printf("\ndone rule 2\n");

        rule3(p);

        for(int j=0; j<BSIZE; j++){
         for(int k=0; k<BSIZE; k++){
            printf("%c",p->f[j][k]);
         }
         if(j != BSIZE-1){
            printf("|\n");
         }
      }
      printf("\ndone rule 3\n");
        rule4(p);

        for(int j=0; j<BSIZE; j++){
         for(int k=0; k<BSIZE; k++){
            printf("%c",p->f[j][k]);
         }
         if(j != BSIZE-1){
            printf("|\n");
         }
      
      }
      printf("\ndone rule 4\n");

        rule5(p);

        for(int j=0; j<BSIZE; j++){
         for(int k=0; k<BSIZE; k++){
            printf("%c",p->f[j][k]);
         }
         if(j != BSIZE-1){
            printf("|\n");
         }
      
      }
      printf("\ndone rule 5\n");

    }while(board_equal(p, &old_board) == false);

    if(check_solve(p) == true){
      printf("solved\n");
      return true;
   }

    printf("unsolved\n");
    return false;
}

bool board2str(board b, char* s)
{
   if(s == NULL){
      return false;
   }

   int i=0;
   for(int j=0; j<BSIZE; j++){
      for(int k=0; k<BSIZE; k++){
         s[i] = b.f[j][k];
         printf("%c",s[i]);
         i++;
      }
      if(j != BSIZE-1){
         s[i] = '|';
         printf("%c",s[i]);
         i++;
      }
      printf("\n");
   }

   return true;
}


bool check_input(char* s, char* top, char* side)
{
    if(s == NULL || top == NULL || side == NULL){
      printf("\nDEAD HERE 1\n");
        return false;
    }

    int len = strlen(s);
    if(len != BSIZE*(BSIZE+1)-1){
      printf("len=%d expected=%d\n", len, BSIZE*(BSIZE+1)-1);
for (int i = 0; i < len; i++) printf("%02d:%c\n", i, s[i]);
;
      printf("\nDEAD HERE 2\n");
        return false;
    }

    for(int i=0; i<len; i++){
        if(s[i] != ' ' && s[i] != '.' && s[i] != '+' && s[i] != '*' && s[i] != '|'){
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

    printf("input is ok\n");
    return true;
}

bool check_tents(board* p)
{
    int total_col = 0, total_row = 0;
    for(int i=0; i<BSIZE; i++){
        total_col = total_col + p->tents_col[i];
        total_row = total_row + p->tents_row[i];
        //printf("total_col: %i total_row: %i\n",total_col,total_row);
    }

    if(total_col != total_row){
        return false;
    }

    return true;
}

bool rule1(board* p)
{
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

   return true;
}

bool rule2(board* p){
   int bound = BSIZE - 1;
   for(int j=1; j<=bound-1; j++){
        for(int k=1; k<=bound-1; k++){
            if(p->f[j][k] == ' ' && p->f[j+1][k] != '*' && p->f[j-1][k] != '*' && p->f[j][k+1] != '*' && p->f[j][k-1] != '*'){
                p->f[j][k] = '.';
            }
        }
    }

    for(int i=1; i<bound; i++){
        if(p->f[0][i] == ' ' && p->f[0][i+1] != '*' && p->f[0][i-1] != '*' && p->f[1][i] != '*'){
            p->f[0][i] = '.';
        }
        if(p->f[i][0] == ' ' && p->f[i-1][0] != '*' && p->f[i+1][0] != '*' && p->f[i][1] != '*'){
            p->f[i][0] = '.';
        }
        if(p->f[bound][i] == ' ' && p->f[bound][i-1] != '*' && p->f[bound][i+1] != '*' && p->f[bound-1][i] != '*'){
            p->f[bound][i] = '.';
        }
        if(p->f[i][bound] == ' ' && p->f[i-1][bound] != '*' && p->f[i+1][bound] != '*' && p->f[i][bound-1] != '*'){
            p->f[i][bound] = '.';
        }
    }

    if(p->f[0][0] == ' ' && p->f[0][1] != '*' && p->f[1][0] != '*'){
        p->f[0][0] = '.';
    }
    if(p->f[0][bound] == ' ' && p->f[0][bound-1] != '*' && p->f[1][bound] != '*'){
        p->f[0][bound] = '.';
    }
    if(p->f[bound][0] == ' ' && p->f[bound-1][0] != '*' && p->f[bound][1] != '*'){
        p->f[bound][0] = '.';
    }
    if(p->f[bound][bound] == ' ' && p->f[bound-1][bound] != '*' && p->f[bound][bound-1] != '*'){
        p->f[bound][bound] = '.';
    }

    return true;
}

bool rule3(board* p)
{
   /*printf("enter rule 3\n");
   for(int j=0; j<BSIZE; j++){
      for(int k=0; k<BSIZE; k++){
            printf("%c",p->f[j][k]);
      }
      if(j != BSIZE - 1){
            printf("|\n");
         }
   }
   */
    
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

      if(count_unknown_row == p->tents_row[j]-count_tents_row){
         for(int i=0; i<BSIZE; i++){
            if(p->f[j][i] == ' '){
               p->f[j][i] = '+';
               printf("switch[%i][%i] in 290\n",j,i);
            }
         }
      }
      if(count_unknown_col == p->tents_col[j]-count_tents_col){
         for(int i=0; i<BSIZE; i++){
            if(p->f[i][j] == ' '){
               p->f[i][j] = '+';
              /* printf("\nfor col[%i]: count_unknown_col=%i, p->tents_col[%i]=%i\n",j,count_unknown_col,j,p->tents_col[j]);*/
               printf("switch[%i][%i] in 298\n",i,j);
            }
         }
      }
   }

   return true;
}

bool rule4(board* p)
{
   int bound = BSIZE - 1;
   for(int j=1; j<=bound-1; j++){
      for(int k=1; k<=bound-1; k++){
         if(p->f[j][k] == ' ' && (p->f[j-1][k-1] == '+' || p->f[j-1][k] == '+' || p->f[j-1][k+1] == '+' || p->f[j][k-1] == '+' || p->f[j][k+1] == '+' || p->f[j+1][k-1] == '+' || p->f[j+1][k] == '+' || p->f[j+1][k+1] == '+')){
            p->f[j][k] = '.';
         }
      }
   }

   for(int i=1; i<=bound-1; i++){
      if(p->f[0][i] == ' ' && (p->f[0][i-1] == '+' || p->f[0][i+1] == '+' || p->f[1][i+1] == '+' || p->f[1][i] == '+' || p->f[1][i+1] == '+')){
         p->f[0][i] = '.';
      }
      if(p->f[i][0] == ' ' && (p->f[i-1][0] == '+' || p->f[i+1][0] == '+' || p->f[i-1][1] == '+' || p->f[i][1] == '+' || p->f[i+1][1] == '+')){
         p->f[i][0] = '.';
      }
      if(p->f[bound][i] == ' ' && (p->f[bound][i-1] == '+' || p->f[bound][i+1] == '+' || p->f[bound-1][i-1] == '+' || p->f[bound-1][i] == '+' || p->f[bound-1][i+1] == '+')){
         p->f[bound][i] = '.';
      }
      if(p->f[i][bound] == ' ' && (p->f[i-1][bound] == '+' || p->f[i+1][bound] == '+' || p->f[i-1][bound-1] == '+' || p->f[i][bound-1] == '+' || p->f[i+1][bound-1] == '+')){
         p->f[i][bound] = '.';
      }
   }

   if(p->f[0][0] == ' ' && (p->f[0][1] == '+' || p->f[1][0] == '+' || p->f[1][1] == '+')){
      p->f[0][0] = '.';
   }
   if(p->f[0][bound] == ' ' && (p->f[0][bound-1] == '+' || p->f[1][bound-1] == '+' || p->f[1][bound] == '+')){
      p->f[0][bound] = '.';
   }
   if(p->f[bound][0] == ' ' && (p->f[bound-1][0] == '+' || p->f[bound-1][1] == '+' || p->f[bound][1] == '+')){
      p->f[bound][0] = '.';
   }
   if(p->f[bound][bound] == ' ' && (p->f[bound-1][bound-1] == '+' || p->f[bound-1][bound] == '+' || p->f[bound][bound-1] == '+')){
      p->f[bound][bound] = '.';
   }

   return true;
}

bool rule5(board* p)
{
   int bound = BSIZE - 1;
   for(int j=1; j<=bound-1; j++){
      for(int k=1; k<=bound-1; k++){
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
               printf("switch[%i][%i] in 381\n",a,b);
            }
         }
      }
   }

   for(int i=1; i<=bound-1; i++){
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
            printf("switch[%i][%i] in 410\n",a,b);
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
            printf("switch[%i][%i] in 436\n",a,b);
         }
      }
      if(p->f[bound][i] == '*'){
         int count_unknown = 0, count_tents = 0, a, b;
         if(p->f[bound][i-1] == ' '){
            count_unknown++;
            a = bound;
            b = i-1;
         }
         if(p->f[bound][i+1] == ' '){
            count_unknown++;
            a = bound;
            b = i+1;
         }
         if(p->f[bound-1][i] == ' '){
            count_unknown++;
            a = bound-1;
            b = i;
         }
         if(p->f[bound][i-1] == '+' || p->f[bound][i+1] == '+' || p->f[bound-1][i] == '+'){
            count_tents++;
         }
         if(count_unknown == 1 && count_tents == 0){
            p->f[a][b] = '+';
            printf("switch[%i][%i] in 461\n",a,b);
         }
      }
      if(p->f[i][bound] == '*'){
         int count_unknown = 0, count_tents = 0, a, b;
         if(p->f[i-1][bound] == ' '){
            count_unknown++;
            a = i-1;
            b = bound;
         }
         if(p->f[i+1][bound] == ' '){
            count_unknown++;
            a = i+1;
            b = bound;
         }
         if(p->f[i][bound-1] == ' '){
            count_unknown++;
            a = i;
            b = bound-1;
         }
         if(p->f[i-1][bound] == '+' || p->f[i+1][bound] == '+' || p->f[i][bound-1] == '+'){
            count_tents++;
         }
         if(count_unknown == 1 && count_tents == 0){
            p->f[a][b] = '+';
            printf("switch[%i][%i] in 486\n",a,b);
         }
      }
   }

   //check corners
   if(p->f[0][0] == '*'){
      if(p->f[0][1] != '+' && p->f[1][0] != '+'){
         if(p->f[0][1] == ' ' && p->f[1][0] != ' '){
            p->f[0][1] = '+';
            printf("switch[%i][%i] in 496\n",0,1);
         }
         if(p->f[0][1] != ' ' && p->f[1][0] == ' '){
            p->f[1][0] = '+';
            printf("switch[%i][%i] in 500\n",1,0);
         }
      }
   }
   if(p->f[0][bound] == '*'){
      if(p->f[0][bound-1] != '+' && p->f[1][bound] != '+'){
         if(p->f[0][bound-1] == ' ' && p->f[1][bound] != ' '){
            p->f[0][bound-1] = '+';
            printf("switch[%i][%i] in 508\n",0,bound-1);
         }
         if(p->f[0][bound-1] != ' ' && p->f[1][bound] == ' '){
            p->f[1][bound] = '+';
            printf("switch[%i][%i] in 512\n",1,bound);
         }
      }
   }
   if(p->f[bound][0] == '*'){
      if(p->f[bound-1][0] != '+' && p->f[bound][1] != '+'){
         if(p->f[bound-1][0] == ' ' && p->f[bound][1] != ' '){
            p->f[bound-1][0] = '+';
            printf("switch[%i][%i] in 520\n",bound-1,0);
         }
         if(p->f[bound-1][0] != ' ' && p->f[bound][1] == ' '){
            p->f[bound][1] = '+';
            printf("switch[%i][%i] in 524\n",bound,1);
         }
      }
   }

   if(p->f[bound][bound] == '*'){
      if(p->f[bound-1][bound] != '+' && p->f[bound][bound-1] != '+'){
         if(p->f[bound-1][bound] == ' ' && p->f[bound][bound-1] != ' '){
            p->f[bound-1][bound] = '+';
            printf("switch[%i][%i] in 533\n",bound-1,bound);
         }
         if(p->f[bound-1][bound] != ' ' && p->f[bound][bound-1] == ' '){
            p->f[bound][bound-1] = '+';
            printf("switch[%i][%i] in 537\n",bound,bound-1);
         }
      }
   }

   return true;
}

bool board_equal(board* new_board, board* old_board) {
    for (int i = 0; i < BSIZE; i++)
        for (int j = 0; j < BSIZE; j++)
            if (new_board->f[i][j] != old_board->f[i][j])
                return false;
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