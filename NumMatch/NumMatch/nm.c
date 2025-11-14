#include "nm.h"

bool solve(int seed){
   board boards_list[BIGSIZE];
   int f=0, c=0;
   boards_list[f] = randfill(seed);

   while(f<=c){
      if(is_final(&boards_list[f])){
         return true;
      }
      //boards_list[f] get into function, try pairs and take
      for(int j1=0;j1<HEIGHT;j1++){
         for(int i1=0;i1<WIDTH;i1++){
            for(int j2=0;j2<HEIGHT;j2++){
               for(int i2=0;i2<WIDTH;i2++){
                  pair z;
                  z.x1 = i1;
                  z.x2 = i2;
                  z.y1 = j1;
                  z.y2 = j2;
                  board tmp = boards_list[f];
                  if(take(&tmp, z)){
                     if(is_unique(tmp, boards_list,f,c)){
                        if (c + 1 >= BIGSIZE) {
                           return false;
                        }
                        c++;
                        boards_list[c] = tmp;
                     }
                  }
               }
            }
         }
      }
      //after tried every pairs
      f++;
   }

   return false;
}

bool take(board* p, pair z){
   if(is_connected(p, z)){
      if(MATCH){
         p->b[z.y1][z.x1] = 0;
         p->b[z.y2][z.x2] = 0;

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
   board b = randfill(17);
   board init1 = b;
   board init2 = b;
   //85995
   //92193
   //57647
   //61477

   //6 4 Horizontal
   assert(check_horizontal_gap(&b, (pair){2,2,3,2}));
   assert(take(&b, (pair){2,2,3,2}));
   //7 7 Vertical
   assert(check_vertical_gap(&b, (pair){4,2,4,3}));
   assert(take(&b, (pair){4,2,4,3}));
   //1 9 diagonal
   assert(check_diagonal_gap(&b, (pair){2,1,3,0}));
   assert(take(&b, (pair){2,1,3,0}));
   
   //1 9 diagonal across gap
   assert(check_diagonal_gap(&b, (pair){1,3,3,1}));
   assert(is_connected(&b, (pair){1,3,3,1}));
   //1 9 fails
   assert(!check_diagonal_gap(&b, (pair){0,0,3,4}));
   assert(!is_connected(&b, (pair){1,3,3,0}));
   
   //is a duplicate? ignore it
   assert(is_unique(init1, &b, 0, 0));
   assert(!is_unique(init1, &init2, 0, 0));

   //is the ‘final’ board? stop
   assert(!is_final(&b));
}

bool check_horizontal_gap(board* p, pair z){
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

bool check_vertical_gap(board* p, pair z){
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

bool check_diagonal_gap(board* p, pair z){
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

bool is_connected(board* p, pair z){
   //can not connet with itself
   if(z.x1 == z.x2 && z.y1 == z.y2){
      return false;
   }
   int dx=(z.x1-z.x2)*(z.x1-z.x2);
   int dy=(z.y1-z.y2)*(z.y1-z.y2);
   //are they 8-connected directly?
   if((dx+dy) <= 2){
      return true;
   }
   //Vertical: are there other numbers between them?
   if(z.x1 == z.x2){
      return check_vertical_gap(p,z);
   }
   //Horizontal: are there other numbers between them?
   if(z.y1 == z.y2 ){
      return check_horizontal_gap(p, z);
   }
   //Diagonal: are there other numbers between them?
   if(dx == dy){
      return check_diagonal_gap(p, z);
   }
   //not connected
   return false;
}

bool is_unique(board tmp, board* p, int start, int end){
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
