#include "csa.h"
#include "mydefs.h"

csa* csa_init(void){
   csa* c = (csa*)calloc(1,sizeof(csa));
   c->b = NULL;
   c->n = 0;
   return c;
}

bool csa_get(csa* c, int idx, int* val){
   if(c == NULL || val == NULL || idx < 0){
      return false;
   }
   int block_idx = block_idx_get(*c, idx);
   // Check if there is corresponding block or not
   if (block_idx < 0){
      return false;
   }
   block* target_block = &c->b[block_idx];
   int msk_idx = idx - target_block->offset;
   
   if((target_block->msk & (mask_t)1<<msk_idx) == 0){
      return false;
   }
   int val_idx = val_idx_get(*target_block, msk_idx);
   *val = target_block->vals[val_idx];
   return true;
}

bool csa_set(csa* c, int idx, int val){
   if(c == NULL || idx<0){
      return false;
   }
   int block_idx = block_idx_get(*c, idx);
   // If there is no corresponding block, creat a new one.
   if (block_idx < 0){
      return block_create(c, idx, val);
   }
   // Otherwise, update the target block.
   else{
      block* target_block = &c->b[block_idx];
      int msk_idx = idx - (target_block->offset);
      return block_update(target_block, msk_idx, val);
   }
}

void csa_tostring(csa* c, char* s){
   if(s == NULL){
      return;
   }
   if(c == NULL){
      s[0] = '\0'; 
      return;
   }
   int blocks_num = c->n;
   if(blocks_num == 0){
      strcpy(s, "0 blocks");
      return;
   }
   else{
      char* p = s;
      if(blocks_num == 1){
         p += sprintf(p, "1 block ");
      }
      else{
         p += sprintf(p, "%i blocks ", blocks_num);
      }
      for(int i=0; i<blocks_num; i++){
         block* target_block = &c->b[i];
         p = block_tostring(*target_block, p);
      }
   }
}

void csa_free(csa** l){
   if(l==NULL || *l==NULL){
      return;
   }
   if((*l)->b != NULL){
      for(int i=0; i<(*l)->n; i++){
         free((*l)->b[i].vals);
         (*l)->b[i].vals = NULL;
      }
      free((*l)->b);
      (*l)->b = NULL;
   }
   free(*l);
   *l=NULL;
}

void test(void){
}

#ifdef EXT
void csa_foreach(void (*func)(int* p, int* ac), csa* c, int* ac){
   if(func==NULL || c==NULL || ac==NULL || c->b==NULL){
      return;
   }
   for(int j=0; j<c->n; j++){
      if(c->b[j].vals==NULL){
         return;
      }
      int vals_count = block_vals_count(c->b[j]);
      for(int i=0; i<vals_count; i++){
         func(&c->b[j].vals[i], ac);
      }
   }
   return;
}

bool csa_delete(csa* c, int idx){
   if(c==NULL || idx<0){
      return false;
   }
   int delete_block_idx = block_idx_get(*c, idx);
   // Check if there is corresponding block or not
   if (delete_block_idx < 0){
      return false;
   }
   block* target_block = &c->b[delete_block_idx];
   // Check if there is corresponding idx or not
   int msk_idx = idx - target_block->offset;
   if((target_block->msk & (mask_t)1<<msk_idx) == 0){
      return false;
   }
   int vals_count = block_vals_count(*target_block);
   // Delete the target value if there are more than one value
   if(vals_count>1){
      return val_delete(target_block, msk_idx, vals_count);
   }
   // Delete the whole block if there is only one value left
   else{
      return block_delete(c, delete_block_idx);
   }
}
#endif

int block_idx_get(const csa c, int idx){
   if (idx < 0 || c.b == NULL) {
      return -1;
   }
   for(int i=0; i<c.n; i++){
      if((unsigned int)idx >= c.b[i].offset &&
      (unsigned int)idx < c.b[i].offset+MSKLEN){
         return i;
      }
   }
   return -1;
}

int val_idx_get(const block b, int msk_idx){
   int count=0;
   for(int i=0; i<msk_idx; i++){
      if((b.msk & (mask_t)1<<i) != 0){
         count++;
      }
   }
   return count;
}

int block_vals_count(const block b){
   int count=0;
   for(int i=0; i<MSKLEN; i++){
      if((b.msk & (mask_t)1<<i) != 0){
         count++;
      }
   }
   return count;
}

int insert_idx_get(const csa c, unsigned int new_offset){
   int insert_block_idx = 0;
   while(insert_block_idx < c.n &&
      c.b[insert_block_idx].offset < new_offset){
      insert_block_idx++;
   }
   return insert_block_idx;
}

bool block_create(csa* c, int idx, int val){
   unsigned int new_offset = (idx/MSKLEN)*MSKLEN;
   int insert_block_idx = insert_idx_get(*c, new_offset);
   int new_blocks_num = c->n + 1;
   block* new_blocks = (block*)realloc(c->b,sizeof(block)*new_blocks_num);
   if(new_blocks==NULL){
      return false;
   }
   c->b = new_blocks;
   for(int i=new_blocks_num-1; i > insert_block_idx; i--){
      c->b[i] = c->b[i-1];
   }
   block* newblock = &c->b[insert_block_idx];
   newblock->offset = new_offset;
   newblock->msk = 0;
   newblock->vals = (int*)calloc(1, sizeof(int));
   if(newblock->vals==NULL){
      return false;
   }
   int msk_idx = idx - newblock->offset;
   newblock->msk = (mask_t)1<<msk_idx;
   newblock->vals[0]=val;
   c->n = new_blocks_num;
   return true;
}

bool block_update(block* target_block, int msk_idx, int val){
   // Overwrite target_block->vals[val_idx]
   if((target_block->msk & ((mask_t)1<<msk_idx)) != 0){
      int val_idx = val_idx_get(*target_block, msk_idx);
      target_block->vals[val_idx]=val;
      return true;
   }
   // Add new value into target_block->vals
   else{
      target_block->msk |= (mask_t)1<<msk_idx;
      int insert_val_idx = val_idx_get(*target_block, msk_idx);
      int vals_count = block_vals_count(*target_block);
      int* new_vals = (int*)realloc(target_block->vals, sizeof(int)*vals_count);
      if(new_vals==NULL){
         return false;
      }
      target_block->vals=new_vals;
      for(int i=vals_count-1; i>insert_val_idx; i--){
         target_block->vals[i] = target_block->vals[i-1];
      }
      new_vals[insert_val_idx] = val;
      return true;
   }
}

char* block_tostring(const block target_block, char* p){
   if(p == NULL || target_block.msk == 0){
      return false;
   }
   int vals_count = block_vals_count(target_block);
   p += sprintf(p, "{%i|", vals_count);
   int val_idx = 0;
   for(int i=0; i<MSKLEN; i++){
      if((target_block.msk&(mask_t)1<<i) != 0){
         p += sprintf(p, "[%i]=%i", target_block.offset+i, target_block.vals[val_idx]);
         if(val_idx != vals_count-1){
            p += sprintf(p, ":");
         }
         else{
            p += sprintf(p, "}");
         }
         val_idx++;
      }
   }
   return p;
}

bool val_delete(block* target_block, int msk_idx, int vals_count){
   if(target_block == NULL){
      return false;
   }
   int delete_val_idx = val_idx_get(*target_block, msk_idx);
   int* new_vals = (int*)calloc(vals_count-1, sizeof(int));
   if(new_vals==NULL){
      return false;
   }
   for(int i=0; i<delete_val_idx; i++){
      new_vals[i]=target_block->vals[i];
   }
   for(int i=delete_val_idx; i<vals_count-1; i++){
      new_vals[i]=target_block->vals[i+1];
   }
   free(target_block->vals);
   target_block->vals = new_vals;
   target_block->msk &= ~((mask_t)1<<msk_idx);
   return true;
}

bool block_delete(csa* c, int delete_block_idx){
   if(c == NULL || delete_block_idx < 0 || delete_block_idx >= c->n){
      return false;
   }
   int blocks_count = c->n;
   block* new_blocks = (block*)calloc(blocks_count-1, sizeof(block));
   if(new_blocks==NULL){
      return false;
   }
   for(int i=0; i<delete_block_idx; i++){
      new_blocks[i]=c->b[i];
   }
   for(int i=delete_block_idx; i<blocks_count-1; i++){
      new_blocks[i]=c->b[i+1];
   }
   free(c->b[delete_block_idx].vals);
   free(c->b);
   c->b = new_blocks;
   c->n--;
   return true;
}
