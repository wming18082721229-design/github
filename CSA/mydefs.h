#pragma once

#define BIGSTR 100000

// Prototypes for other "private" functions etc.

// Get the idx of the target block.
int block_idx_get(const csa c, int idx);

// Get the idx of the target value in the array.
int val_idx_get(const block b, int msk_idx);

// Get the total number of values in the particular block.
int block_vals_count(const block b);

// Get the corresponding block index in the array is going to be inserted.
int insert_idx_get(const csa c, unsigned int new_offset);

// Creat a new block and write the first value in it.
bool block_create(csa* c, int idx, int val);

// Write the given value into the target block.
bool block_update(block* target_block, int msk_idx, int val);

// Write values in the target block to string.
char* block_tostring(const block target_block, char* p);

// Delete the target value in the target block.
bool val_delete(block* target_block, int msk_idx, int vals_count);

// Delete the target block which has not held ant value.
bool block_delete(csa* c, int delete_block_idx);
