/*
 * mm-implicit.c - The best malloc package EVAR!
 *
 * TODO (bug): mm_realloc and mm_calloc don't seem to be working...
 * TODO (bug): The allocator doesn't re-use space very well...
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memlib.h"
#include "mm.h"

/** The required alignment of heap payloads */
const size_t ALIGNMENT = 2 * sizeof(size_t);

/** The layout of each block allocated on the heap */
typedef struct {
    /** The size of the block and whether it is allocated (stored in the low
     * bit) */
    size_t header;
    /**
     * We don't know what the size of the payload will be, so we will
     * declare it as a zero-length array.  This allow us to obtain a
     * pointer to the start of the payload.
     */
    uint8_t payload[];
} block_t;

/** The first and last blocks on the heap */
static block_t* mm_heap_first = NULL;
static block_t* mm_heap_last = NULL;

/** Rounds up `size` to the nearest multiple of `n` */
static size_t round_up(size_t size, size_t n) {
    return (size + (n - 1)) / n * n;
}

/** Set's a block's header with the given size and allocation state */
static void set_header(block_t* block, size_t size, bool is_allocated) {
    block->header = size | is_allocated;
}

/** Extracts a block's size from its header */
static size_t get_size(block_t* block) { return block->header & ~1; }

/** Extracts a block's allocation state from its header */
static bool is_allocated(block_t* block) { return block->header & 1; }

/**
 * Finds the first free block in the heap with at least the given size.
 * If no block is large enough, returns NULL.
 */
static block_t* find_fit(size_t size) {
    // Traverse the blocks in the heap using the implicit list
    for (block_t* curr = mm_heap_first;
         mm_heap_last != NULL && curr <= mm_heap_last;
         curr = (void*)curr + get_size(curr)) {
        // If the block is free and large enough for the allocation, return it
        if (!is_allocated(curr) && get_size(curr) >= size) {
            return curr;
        }
    }
    return NULL;
}

/** Gets the header corresponding to a given payload pointer */
static block_t* block_from_payload(void* ptr) {
    return ptr - offsetof(block_t, payload);
}

/**
 * mm_init - Initializes the allocator state
 */
bool mm_init(void) {
    // We want the first payload to start at ALIGNMENT bytes from the start of
    // the heap
    void* padding = mem_sbrk(ALIGNMENT - sizeof(block_t));
    if (padding == (void*)-1) {
        return false;
    }

    // Initialize the heap with no blocks
    mm_heap_first = NULL;
    mm_heap_last = NULL;
    return true;
}


void coalesce() {
    if (mm_heap_first == NULL || mm_heap_last == NULL) {
        return;
    }

    for (block_t *curr = mm_heap_first;
         curr <= mm_heap_last;
         curr = (block_t *)((char *)curr + get_size(curr))) {

        // Skip allocated blocks and isolated free blocks
        block_t *next = (block_t *)((char *)curr + get_size(curr));
        if (is_allocated(curr) || next > mm_heap_last || is_allocated(next)) {
            continue;
        }

        // Absorb all consecutive free blocks that follow into curr
        while (next <= mm_heap_last && !is_allocated(next)) {
            size_t new_size = get_size(curr) + get_size(next);
            set_header(curr, new_size, false);

            if (mm_heap_last == next) {
                mm_heap_last = curr;
            }

            next = (block_t *)((char *)curr + get_size(curr));
        }
    }
}

/**
 * mm_malloc - Allocates a block with the given size
 */
void *mm_malloc(size_t size) {
    // The block must have enough space for a header and be 16-byte aligned
    size = round_up(sizeof(block_t) + size, ALIGNMENT);
    coalesce();
    // If there is a large enough free block, use it
    block_t *block = find_fit(size);

//*(block+size)
    if (block != NULL) {
        size_t difference = get_size(block) - size;
        if(get_size(block) > (size) /*&& difference > ALIGNMENT*/){
            
            block_t* new_block = (block_t*)((void*)block+size);

            set_header(new_block, difference, false);
            if(block == mm_heap_last) mm_heap_last = new_block;
        }
        // size_t old_block_size = difference > ALIGNMENT? size: get_size(block);
        set_header(block, size, true);
        return block->payload;
    }
    

    // Otherwise, a new block needs to be allocated at the end of the heap
    block = mem_sbrk(size);
    if (block == (void *) -1) {
        return NULL;
    }

    // Update mm_heap_first and mm_heap_last since we extended the heap
    if (mm_heap_first == NULL) {
        mm_heap_first = block;
    }
    mm_heap_last = block;

    // Initialize the block with the allocated size
    set_header(block, size, true);
    return block->payload;
}

/**
 * mm_free - Releases a block to be reused for future allocations
 */
void mm_free(void* ptr) {
    // mm_free(NULL) does nothing
    if (ptr == NULL) {
        return;
    }

    // Mark the block as unallocated
    block_t* block = block_from_payload(ptr);
    set_header(block, get_size(block), false);
}

/**
 * mm_realloc - Change the size of the block by mm_mallocing a new block,
 *      copying its data, and mm_freeing the old block.
 */
void* mm_realloc(void* old_ptr, size_t size) {
    (void)old_ptr;
    (void)size;
    if (old_ptr == NULL) {
        return mm_malloc(size);
    }
    if (size == 0) {
        mm_free(old_ptr);
        return NULL;
    }

    void* new_ptr = mm_malloc(size);

    block_t* old_block = block_from_payload(old_ptr);
    size_t old_size = get_size(old_block);

    memcpy(new_ptr, old_ptr, (size < old_size ? size : old_size));

    return new_ptr;
}

/**
 * mm_calloc - Allocate the block and set it to zero.
 */
void* mm_calloc(size_t nmemb, size_t size) {
    (void)nmemb;
    (void)size;

    void* ptr = mm_malloc(size);

    memset(ptr, 0, size);

    return ptr;
}

/**
 * mm_checkheap - So simple, it doesn't need a checker!
 */
void mm_checkheap(void) {
    // for (
    //     block_t *curr = mm_heap_first;
    //     mm_heap_last != NULL && curr <= mm_heap_last;
    //     curr = (void *) curr + get_size(curr)
    // ) {

    //     if (is_allocated(curr)) {
    //         printf("F ");
    //     }else{
    //         printf("E ");
    //     }
    // }
    // printf("END\n");
}
