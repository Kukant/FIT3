// xkukan00

#include "tmal.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

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
        return NULL;

    blks_table[tid].heap_size = theap;
    blks_table[tid].nblks = nblks;

    for (unsigned i = 0; i < nblks; i++)
        blk_ctor(&BLK(tid,i));

    // allocation of first blk
    BLK(tid,0).ptr = malloc(theap);
    if (BLK(tid,0).ptr == NULL)
    {
        free(blks_table[tid].blks);
        return NULL;
    }




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
    struct blk_t to_split = BLK(tid,blk_idx);
    if (to_split.used == true || to_split.size <= req_size)
        return -1;

    // find first unused block
    struct blk_t *first_unused = NULL;
    int index = -1;
    for (unsigned i = 0; i < blks_table[tid].nblks; i++)
    {
        if (BLK(tid,i).ptr == NULL)
        {
            index = i;
            first_unused = &BLK(tid,i);
            break;
        }
    }


    if (first_unused == NULL) // no more free blocks
    {
        return -1;
    }

    int next_index = BLK(tid,blk_idx).next_idx;

    BLK(tid, next_index).prev_idx = index;

    first_unused->ptr = to_split.ptr + req_size;
    first_unused->used = false;
    first_unused->prev_idx = blk_idx;
    first_unused->next_idx = to_split.next_idx;
    first_unused->size = to_split.size - req_size;

    BLK(tid,blk_idx).next_idx = index;
    BLK(tid,blk_idx).used = true;
    BLK(tid,blk_idx).size = req_size;

    return index;
}

/**
 * Merge two blocks.
 * @param tid       thread index
 * @param left_idx  index of the left block
 * @param right_idx index of the right block
 */
void tal_blk_merge(unsigned tid, int left_idx, int right_idx)
{
    BLK(tid, left_idx).next_idx = BLK(tid, right_idx).next_idx;
    if (BLK(tid, right_idx).next_idx >= 0)
        BLK(tid, BLK(tid, right_idx).next_idx).prev_idx = left_idx;
    BLK(tid, left_idx).size += BLK(tid, right_idx).size;
    BLK(tid, left_idx).used = false;
    blk_ctor(&BLK(tid, right_idx));
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
    // align
    if (size % 8 != 0)
        size += (8 - size % 8);

    // find first free block with enough space
    int index = 0;
    for (unsigned i = 0; i < blks_table[tid].nblks ; i++)
    {
        if (BLK(tid, index).used == false && BLK(tid, index).size >= size)
        {
            if (BLK(tid, index).size == size)
            {
                BLK(tid, index).used = true;
                return BLK(tid, index).ptr;
            }

            int j = tal_blk_split(tid, index, size);
            if (j < 0) // nekde se stala chyba
                return NULL;
            else
                return BLK(tid, index).ptr;
        }

        if (BLK(tid, index).next_idx < 0)
            break;
        else
            index = BLK(tid, index).next_idx;
    }

    return NULL;
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
    // align
    if (size % 8 != 0)
        size += (8 - size % 8);

    int index = 0;
    for (unsigned i = 0; i < blks_table[tid].nblks ; i++)
    {
        if (BLK(tid, index).ptr == ptr)
        {
            int next = BLK(tid, index).next_idx;
            size_t diff = BLK(tid, index).size < size ? size - BLK(tid, index).size : BLK(tid, index).size - size;
            if (size < BLK(tid, index).size) // will be reduced
            {
                if (next > 0 && BLK(tid, next).used == false) // if next is empty
                {
                    BLK(tid, next).ptr -= diff;
                    BLK(tid, next).size += diff;
                }
                else // create new block
                {
                    int new_blk_index = -1;
                    for (unsigned j = 0; j < blks_table[tid].nblks; j++)
                    {
                        if (BLK(tid, j).ptr == NULL) // volny blok
                        {
                            new_blk_index = j;
                            break;
                        }
                    }

                    if (new_blk_index < 0) // empty not found
                        return  NULL;

                    BLK(tid, new_blk_index).size = diff;
                    BLK(tid, new_blk_index).used = false;
                    BLK(tid, new_blk_index).ptr = BLK(tid, next).ptr - diff;
                    BLK(tid, new_blk_index).next_idx = next;
                    BLK(tid, new_blk_index).prev_idx = index;

                    BLK(tid, index).next_idx = new_blk_index;
                    BLK(tid, next).prev_idx = new_blk_index;
                }

                BLK(tid, index).size -= diff;
                return BLK(tid, index).ptr;
            }
            else if (size == BLK(tid, index).size) // same size
            {
                return BLK(tid, index).ptr;
            }
            else // size larger than actual
            {
                if (next >= 0 && BLK(tid, next).used == false && BLK(tid, next).size + BLK(tid, index).size >= size) // can be extended
                {
                    if (BLK(tid, next).size + BLK(tid, index).size == size)
                    {
                        BLK(tid, next).ptr = NULL;
                        BLK(tid, BLK(tid, next).next_idx).prev_idx = index;
                        if (BLK(tid, next).next_idx > 0)
                            BLK(tid, index).next_idx = BLK(tid, next).next_idx;
                    }
                    else
                    {
                        BLK(tid, next).size -= diff;
                        BLK(tid, next).ptr += diff;
                    }

                    BLK(tid, index).size += diff;
                    return BLK(tid, index).ptr;
                }
                else // is the last one or cannot be extended
                {
                    void *new_memory = tal_alloc(tid, size);
                    if (new_memory != NULL)
                    {
                        memcpy(new_memory, ptr, size);
                        tal_free(tid, ptr);
                    }

                    return new_memory;
                }
            }
        } // end of if

        if (BLK(tid, index).next_idx < 0)
            break;
        else
            index = BLK(tid, index).next_idx;
    } // end of for cycle

    return NULL;
}

/**
 * Free memory for a given thread.
 * @param tid   thread index
 * @param ptr   pointer to memory allocated by tal_alloc or tal_realloc.
 *              NULL = do nothing.
 */
void tal_free(unsigned tid, void *ptr)
{
    int index = 0;
    for (unsigned i = 0; i < blks_table[tid].nblks ; i++)
    {
        if (BLK(tid, index).ptr == ptr)
        {
            BLK(tid, index).used = false;
            int prev = BLK(tid, index).prev_idx;
            int next = BLK(tid, index).next_idx;
            if (prev >= 0 && BLK(tid, prev).used == false)
            {
                tal_blk_merge(tid, prev, index);
                if (next >= 0 && BLK(tid, next).used == false)
                    tal_blk_merge(tid, prev, next);
            }
            else if (next >= 0 && BLK(tid, next).used == false)
                tal_blk_merge(tid, index, next);
            return;
        }

        if (BLK(tid, index).next_idx < 0)
            break;
        else
            index = BLK(tid, index).next_idx;
    }
}