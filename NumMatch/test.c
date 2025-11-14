#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#define HEIGHT 4
#define WIDTH 5
#define BIGSIZE 100000
#define MATCH ((p->b[z.y1][z.x1] == p->b[z.y2][z.x2]) || (p->b[z.y1][z.x1]+p->b[z.y2][z.x2] == 10)) && boards_list[p->f].b[z.y1][z.x1] != 0 && boards_list[p->f].b[z.y2][z.x2] != 0

struct pair {
    int x1, y1, x2, y2;
};
typedef struct pair pair;

struct board {
    int b[HEIGHT][WIDTH];
    int f;
    int c;
};
typedef struct board board;

board randfill(int n);
bool take(board* p, pair z);
bool solve(int seed);

//helper function
bool is_connected(board* p, pair z);
bool is_final(board* p);
bool boards_print(board p);
bool is_unique(board tmp, board* p);
pair paircpy(int x1, int y1, int x2, int y2);

board boards_list[BIGSIZE];

int main(void){
//------------------------------------------------------
    //assert(is_inbound(0,0,0,0)==0);
    board p = randfill(17);

    //show the present board
    boards_print(p);
//------------------------------------------------------

    /*pair z[HEIGHT*WIDTH];
    int n = 0;
    for(int j=0;j<HEIGHT;j++){
        for(int i=0;i<WIDTH;i++){
            z[n].x1 = i;
            z[n].y1 = j;
            n++;
        }
    }

    for(int j=0; j<HEIGHT*WIDTH; j++){
        printf("x1:%i; y1:%i; x2:%i; y2:%i;",z[j].x1,z[j].y1, z[j].x2,z[j].y2);
        printf(" n:%i\n", j);
    }*/

//------------------------------------------------------

}
board randfill(int n){
    //board randfill(int n):
    srand(n);
    board p;
    for(int j=0;j<HEIGHT;j++){
        for(int i=0;i<WIDTH;i++){
            p.b[j][i] = rand()%9+1;
        }
    }

    p.f = 0;
    p.c = 1;
    boards_list[0] = p;

    return p;
}

bool take(board* p, pair z){
    if(is_connected(p, z)){
        if(MATCH){
            board tmp = *p;
            tmp.b[z.y1][z.x1] = 0;
            tmp.b[z.y2][z.x2] = 0;
            
            *p = tmp;
            return true;
        }
    }

    return false;
}

bool solve(int seed){
    board boards_list[BIGSIZE];
    int f=0, c=0;
    boards_list[f] = randfill(seed);

    int count_match;
    while(f<=c){
        if(is_final(&boards_list[f])){
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
                            if(is_unique(tmp, boards_list)){
                                if (c + 1 >= BIGSIZE) {
                                    return false;
                                }
                                c++;
                                boards_list[c] = tmp;
                                boards_list[c].c = c;
                                boards_list[c].f = f;
                                for(int j=0;j<HEIGHT;j++){
                                    for(int i=0;i<WIDTH;i++){
                                        printf("%i",tmp.b[j][i]);
                                    }
                                    printf("\n");
                                }
                                printf("\n");
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

bool is_unique(board tmp, board* p){
    for(int n = tmp.f; n < tmp.c; n++){
        for(int j=0;j<HEIGHT;j++){
            for(int i=0;i<WIDTH;i++){
                if(tmp.b[j][i] != boards_list[n].b[j][i]){
                    return true;
                }
            }
        }
        return false;
    }
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

bool boards_print(board p){
    for(int n = p.f; n < p.c; n++){
        printf("No.%i----f:%i; c: %i\n", n, p.f, p.c);
        for(int j=0;j<HEIGHT;j++){
            for(int i=0;i<WIDTH;i++){
                printf("%i",boards_list[n].b[j][i]);
            }
        printf("\n");
        }
    }
    return true;
}

bool is_inbound(int x1, int y1, int x2, int y2){
    if(x1<0 || x1>WIDTH || x2<0 || x2>WIDTH || y1<0 || y1>HEIGHT || y2<0 || y2>HEIGHT){
        return false;
    }
    return true;
}