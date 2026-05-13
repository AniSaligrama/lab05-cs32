# Malloc Lab: Dynamic Memory Allocator

In this lab, you will implement your own version of malloc, free, realloc, and calloc using an implicit free list.

## Getting Started

1. Read through `src/mm-implicit.c` to understand the provided starter code
2. Build the project: `make`
3. Run the test driver: `./bin/mdriver-implicit`

The starter code provides:
- Basic heap initialization (`mm_init`)
- Simple first-fit malloc (`mm_malloc`)
- Basic free (`mm_free`)
- Block structure with headers
- Helper functions for block manipulation

## Your Tasks

### Task 1: Implement realloc and calloc (Correctness)

The `mm-implicit.c` file implements a very simple allocator discussed in class. Unfortunately, it's not quite complete. In particular, the realloc and calloc implementations are missing, which makes the tester fail due to "correctness".

**TODO:** Write `mm_realloc()` and `mm_calloc()` to make the tester pass the correctness tests.

**Implementation hints:**
- `mm_realloc(ptr, size)`: Allocate a new block, copy data from old block, free old block
  - Handle edge cases: NULL ptr (act like malloc), size 0 (act like free)
- `mm_calloc(nmemb, size)`: Allocate `nmemb * size` bytes and zero the memory
  - Check for multiplication overflow
  - Use `memset()` to zero the allocated memory

**Expected outcome:** All tests pass for correctness

---

### Task 2: Add Block Splitting (Efficiency)

Now that your `mm-implicit.c` is passing the correctness tests, you should add **block splitting** as discussed in class.

Currently, when we find a free block that's large enough, we use the entire block even if it's much larger than needed. This wastes space (internal fragmentation).

**TODO:** Modify `mm_malloc()` to split free blocks when appropriate.

**Implementation hints:**
- When you find a free block with `find_fit()`, check if it's significantly larger than needed
- If yes, split it into two blocks:
  1. An allocated block of the requested size
  2. A free block with the remaining space
- Only split if the remainder is at least `ALIGNMENT` bytes (minimum block size)
- Update `mm_heap_last` if you split the last block

**Expected outcome:** Score around 23/30

---

### Task 3: Add Delayed Coalescing (Performance)

Finally, improve the implicit list further by adding **delayed coalescing** (coalescing in malloc instead of free) as discussed in class.

Currently, when we free a block, we just mark it as free. Over time, this creates many small free blocks that can't satisfy larger allocations (external fragmentation).

**TODO:** Implement coalescing in `mm_malloc()` before looking for a fit.

**Implementation hints:**
- Add a helper function `coalesce_free_blocks()` that loops through the heap
- For each free block, check if the next block is also free
- If yes, merge them into one larger free block by:
  - Adding their sizes together
  - Updating the header of the first block
  - Adjusting `mm_heap_last` if necessary
- Call this function at the start of `mm_malloc()` before `find_fit()`
- This is called "delayed" coalescing because we wait until malloc time

**Expected outcome:** Score 30/30

---

## Testing

Run the test driver to check your implementation:

```bash
make
./bin/mdriver-implicit
```

The driver runs 12 trace files that simulate various allocation patterns:
- `corners.rep` - Edge cases
- `short2.rep` - Simple allocations
- `malloc.rep` - Basic malloc patterns
- `coalescing-bal.rep` - Tests coalescing (Task 3)
- `fs.rep`, `hostname.rep`, `login.rep`, `ls.rep`, `perl.rep`, `random-bal.rep`, `rm.rep`, `xterm.rep` - Real program traces

**Score breakdown:**
- Correctness: Must pass all tests (Task 1)
- Utilization: Space efficiency (Tasks 2 & 3)
- Throughput: Speed (less important)
- Final score: 0-30 points

## Tips

- Start with Task 1 - get correctness working first
- Test after each task to ensure you didn't break anything
- Use the debug flag: `make DEBUG=1` for more error checking
- Add `mm_checkheap()` assertions to catch bugs early
- Draw diagrams of your heap structure on paper

## Submission

Submit your completed `mm-implicit.c` file.

**Expected scores:**
- Task 1 complete: Tests pass, but low score (~0-15/30)
- Task 2 complete: Score around 23/30
- Task 3 complete: Score 30/30

Good luck!
