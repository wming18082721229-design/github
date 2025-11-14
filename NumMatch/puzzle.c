#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#define HEIGHT 4
#define WIDTH 5
#define BIGSIZE 50000
#define MATCH ((p->b[z.y1][z.x1] == p->b[z.y2][z.x2]) || (p->b[z.y1][z.x1]+p->b[z.y2][z.x2] == 10) && (p->b[z.y1][z.x1]!=0 && p->b[z.y2][z.x2]!=0))

struct pair {
   int x1, y1, x2, y2;
};
typedef struct pair pair;

struct board {
   int b[HEIGHT][WIDTH];
};
typedef struct board board;

//--------------------------------------------------------------------
/* n is used to seed srand(). Then numbers are
generated to fill the board row-wise,
left to right, top down. Each number = rand()%9+1.
rand() is compiler dependent, but I've tested this
with gcc and clang on WSL2 and the lab machines.
*/
board randfill(int n);

/* Returns true if the pair of numbers
in the board p are a valid match or not i.e.
they are the same or sum to ten, and are either
touching, or are on a straight line with no other
numbers between. If true, removes that pair.
*/
bool take(board* p, pair z);

/* Returns true if the board can be solved using
the exact (breadth-first) method outlined in the
exercise, false if not.
*/
bool solve(int seed);
//--------------------------------------------------------------------

//check the 8-connected cells
//check bound
//z[0] = x1; z[1] = y1; z[2] = x2; z[3] = y2

//--------------------------------------------------------------------
//helper function
bool is_connected(board* p, pair z);
bool is_unique(board tmp, board* p, int start, int end);
bool is_final(board* p);
pair paircpy(int x1, int y1, int x2, int y2);

bool boards_print(board p);


int main(void){
    printf("start\n");

    if(solve(3648)){
        printf("finish\n");
    }
}


bool solve(int seed){
    board boards_list[BIGSIZE];
    int f=0, c=0;
    boards_list[f] = randfill(seed);

    int count_match;
    while(f<=c){
        if(is_final(&boards_list[f])){
            for(int j=0;j<HEIGHT;j++){
                for(int i=0;i<WIDTH;i++){
                    printf("%i",boards_list[f].b[j][i]);
                }
                printf("\n");
            }
            printf("\n");
            return true;
        }
        //boards_list[f] get into function, try pairs and take
        count_match = 0; 
        for(int j1=0;j1<HEIGHT;j1++){
            for(int i1=0;i1<WIDTH;i1++){
                for(int j2=0;j2<HEIGHT;j2++){
                    for(int i2=0;i2<WIDTH;i2++){
                        pair z = paircpy(i1,j1,i2,j2);
                        board tmp = boards_list[f];
                        if(take(&tmp, z)){
                            count_match++;
                            if(is_unique(tmp, boards_list,f,c)){
                                /*if (c + 1 >= BIGSIZE) {
                                    return false;
                                }*/
                                c++;
                                boards_list[c] = tmp;
                                
                            }
                        }
                    }
                }
            }
        }
        //after tried every pairs
        if(count_match == 0){
            return false;
        }
        f++;
    }

    return false;
}

/* Returns true if the pair of numbers
in the board p are a valid match or not i.e.
they are the same or sum to ten, and are either
touching, or are on a straight line with no other
numbers between. If true, removes that pair.
*/
bool take(board* p, pair z){
    if(is_connected(p, z)){
        if(MATCH){
            printf("match\n");
            board tmp = *p;
            tmp.b[z.y1][z.x1] = 0;
            tmp.b[z.y2][z.x2] = 0;
            
            *p = tmp;
            return true;
        }
    }

    return false;
}


board randfill(int n){
    srand(n);
    board p;
    for(int j=0;j<HEIGHT;j++){
        for(int i=0;i<WIDTH;i++){
            p.b[j][i] = rand()%9+1;
        }
    }
    return p;
}


void test(void){

}

//----------------------------------------------------------------------------------
//helper function

bool is_connected(board* p, pair z){
    int dx=(z.x1-z.x2)*(z.x1-z.x2);
    int dy=(z.y1-z.y2)*(z.y1-z.y2);
    
    //can not connet with itself
    if(z.x1 == z.x2 && z.y1 == z.y2){
        return false;
    }
    //are they 8-connected directly?
    if((dx+dy)<=2){
        return true;
    }

    //Vertical: are there other numbers between them?
    if(z.x1 == z.x2){
        int low_y  = (z.y1 < z.y2) ? z.y1 : z.y2;
        int high_y = (z.y1 > z.y2) ? z.y1 : z.y2;
        //check
        for(int y=low_y+1; y<high_y; y++){
            if(p->b[y][z.x1]!=0){
                return false;
            }
        }
        return true;
    }

    //Horiz: are there other numbers between them?
    if(z.y1 == z.y2 ){
        int low_x  = (z.x1 < z.x2) ? z.x1 : z.x2;
        int high_x = (z.x1 > z.x2) ? z.x1 : z.x2;
        //check
        for(int x=low_x+1; x<high_x; x++){
            if(p->b[z.y1][x]!=0){
                return false;
            }
        }
        return true;
    }
    
    //diagonal
    if(dx==dy){
        //z1->z2
        int step_x = (z.x1 < z.x2) ? 1 : -1;
        int step_y = (z.y1 < z.y2) ? 1 : -1;

        int x = z.x1 + step_x;
        int y = z.y1 + step_y;
        while(x!=z.x2 && y!= z.y2){
            if(p->b[y][x]!=0){
                return false;
            }
            x = x + step_x;
            y = y + step_y;
        }

        return true;
    }

    //not connected
    return false;
}

bool is_unique(board tmp, board* p, int start, int end){
    printf("from %i to %i\n",start,end);
    for(int n = start; n <= end; n++){
        int diff = 0;
        for(int j=0;j<HEIGHT && diff==0;j++){
            for(int i=0;i<WIDTH;i++){
                if(tmp.b[j][i] != p[n].b[j][i]){
                    diff++;
                    break;
                }
            }
        }
        if(diff==0){
            return false;
        }
    }

    return true;
}

bool is_final(board* p){
    for(int j=0;j<HEIGHT;j++){
            for(int i=0;i<WIDTH;i++){
                if(p->b[j][i] != 0){
                    return false;
                }
            }
        }
        return true;
}

pair paircpy(int x1, int y1, int x2, int y2){
    pair takencells;
    takencells.x1 = x1;
    takencells.x2 = x2;
    takencells.y1 = y1;
    takencells.y2 = y2;
    return takencells;
}