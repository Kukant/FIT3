// xkukan00

#include "tmal.h"
#include <stdio.h>
#include <stdlib.h>

struct blk_pool_t *blks_table = NULL;

/**
 * Allocate sparse table of blocks for several threads.
 * @param  nthreads     number of threads/items in the table
 * @return              pointer to the first block pool, NULL = failed
 */
struct blk_pool_t *tal_alloc_blks_table(unsigned nthreads)
{
    blks_table = (struct blk_pool_t *) malloc(sizeof(struct blk_pool_t) * nthreads);
    return  blks_table;
}

/**
 * Block metadata constructor (alone, not used block).
 * @param blk pointer to block metadata.
 */
void blk_ctor(struct blk_t *blk)
{
    blk->used = false;
    blk->ptr = NULL;
    blk->prev_idx = -1;
    blk->next_idx = -1;
}

/**
 * Allocates and initialize pool of blocks.
 * @param  tid      thread index.
 * @param  nblks    capacity in number of blocks in the pool.
 * @param  theap    heap capacity for a given thread.
 * @return          pointer to the first block in a pool.
 */
struct blk_t *tal_init_blks(unsigned tid, unsigned nblks, size_t theap)
{
    blks_table[tid].blks = (struct blk_t *) malloc(sizeof(struct blk_t) * nblks);
    if (blks_table[tid].blks == NULL)
        //TODO
        return NULL;

    blks_table[tid].heap_size = theap;
    blks_table[tid].nblks = nblks;

    for (unsigned i = 0; i < nblks; i++)
        blk_ctor(&BLK(tid,i));

    // allocation of first blk
    BLK(tid,0).ptr = malloc(theap);
    if (BLK(tid,0).ptr == NULL) //TODO
        return NULL;


    BLK(tid, 0).next_idx = -1;
    BLK(tid, 0).prev_idx = -1;
    BLK(tid, 0).size = theap;
    BLK(tid, 0).used = false;

    return blks_table[tid].blks;

}

/**
 * Splits one block into two.
 * @param tid       thread index
 * @param blk_idx   index of the block to be split
 * @param req_size  requested size of the block
 * @return          index of a new block created as remainder.
 */
int tal_blk_split(unsigned tid, int blk_idx, size_t req_size)
{
    if (BLK(tid,blk_idx).used == true || BLK(tid,blk_idx).size < req_size)
        return -1;

    // find first unused block
    struct blk_t *first_unused = NULL;
    for (int i = 0; i < blks_table[tid].nblks; i++)
        if (BLK(tid,blk_idx).ptr == NULL)
            first_unused = &BLK(tid,blk_idx);

    if (first_unused == NULL) // no more free blocks
        return -1;

    // find some space in the memory
    void *free_space = NULL;
    do
    {

    } while(0);

    first_unused->used = true;
    first_unused->prev_idx = blk_idx;
    first_unused->next_idx = -1;
    first_unused->size = req_size;
}

/**
 * Merge two blocks.
 * @param tid       thread index
 * @param left_idx  index of the left block
 * @param right_idx index of the right block
 */
void tal_blk_merge(unsigned tid, int left_idx, int right_idx)
{

}

/**
 * Allocate memory for a given thread. Note that allocated memory will be
 * aligned to sizeof(size_t) bytes.
 * @param  tid  thread index (in the blocks table)
 * @param  size requested allocated size
 * @return      pointer to allocated space, NULL = failed
 */
void *tal_alloc(unsigned tid, size_t size)
{

}

/**
 * Realloc memory for a given thread.
 * @param tid   thread index
 * @param ptr   pointer to allocated memory, NULL = allocate a new memory.
 * @param size  a new requested size (may be smaller than already allocated),
 *              0 = equivalent to free the allocated memory.
 * @return      pointer to reallocated space, NULL = failed.
 */
void *tal_realloc(unsigned tid, void *ptr, size_t size)
{

}

/**
 * Free memory for a given thread.
 * @param tid   thread index
 * @param ptr   pointer to memory allocated by tal_alloc or tal_realloc.
 *              NULL = do nothing.
 */
void tal_free(unsigned tid, void *ptr)
{

}