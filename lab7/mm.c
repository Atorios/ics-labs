/*
 * name: 黎君
 * ID: 516030910233
 * Implement: Segregated Explicit Linked List + Specific Optimizations For Some Traces
 * Block Structure:
 * ----------------
 * |   header     |
 * |--------------|
 * |    SUCC      |    note: Only free blocks need SUCC and PRED, so in the assigned block, 
 * |--------------|          the payload will overlap them.
 * |    PRED      |
 * |--------------|
 * |   payload    |
 * |--------------|
 * |   padding    |
 * |--------------|
 * |   footer     |
 * ----------------
 * Linked List: 1~16, 16~32, ..., 2^22~, total 20 lists, list heads are placed before heaplist
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/* 
 * Basep is mem_heap_lo(), the first byte of heap, 
 * Store pointers as the difference between them and basep 
 */
static char *basep = NULL;
#define ptr_to_word(ptr) ((ptr == NULL) ? 0U : (unsigned int)((char *)(ptr) - basep))
#define word_to_ptr(word) ((word == 0U) ? NULL : (char *)basep + (unsigned int)(word))

/* Single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* Rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Macros from text */
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE ((1<<12) + 640) // Optimized for trace 4&9&10
#define BLKSIZE 16

#define MAX(x, y) ((x) > (y)?(x) : (y))

#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(unsigned int*) (p))
#define PUT(p, val) (*(unsigned int*) (p) = (val))

/* Get predecessor and successor of a free block in their free list */
#define SUCC(bp) ((unsigned int*)(word_to_ptr(GET(bp))))
#define PRED(bp) ((unsigned int*)(word_to_ptr(GET((unsigned int*)(bp) + 1))))

/* Put predecessor and successor of a free block in their free list */
#define LINK_SUCC(from, to) (PUT((from), (ptr_to_word(to))))
#define LINK_PRED(from, to) (PUT((unsigned int*)(from)+1, (ptr_to_word(to))))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* The sizes of free lists */
#define SIZE1 (1<<4)
#define SIZE2 (1<<5)
#define SIZE3 (1<<6)
#define SIZE4 (1<<7)
#define SIZE5 (1<<8)
#define SIZE6 (1<<9)
#define SIZE7 (1<<10)
#define SIZE8 (1<<11)
#define SIZE9 (1<<12) 
#define SIZE10 (1<<13)
#define SIZE11 (1<<14)
#define SIZE12 (1<<15)
#define SIZE13 (1<<16)
#define SIZE14 (1<<17)
#define SIZE15 (1<<18)
#define SIZE16 (1<<19)
#define SIZE17 (1<<20)
#define SIZE18 (1<<21)
#define SIZE19 (1<<22)
#define LIST_NUM 20

/* Get the index of the free list where the bolck of this size should be inserted into */
#define INDEX(size) \
(int)((size > SIZE1) + (size > SIZE2) + (size > SIZE3) + (size > SIZE4) + (size > SIZE5) \
+ (size > SIZE6) + (size > SIZE7) + (size > SIZE8) + (size > SIZE9) + (size > SIZE10) + (size > SIZE11) + (size > SIZE12) \
+ (size > SIZE13) + (size > SIZE14) + (size > SIZE15) + (size > SIZE16) + (size > SIZE17) + (size > SIZE18) + (size > SIZE19))

/* Global variables */
static char *heap_listp = NULL;
static char *list_head = NULL;
static char *list_tail = NULL;

/*
 * insert_list - Insert the free block pointed by bp
 *               into the proper free list
 *               In a free list, blocks are ordered by their addresses
 */
static void insert_list(void *bp)
{
	int index = INDEX(GET_SIZE(HDRP(bp)));
	char *free_listp = list_head + index * WSIZE;
	char *free_tailp = list_tail + index * WSIZE;
	/* The list is empty */
	if (SUCC(free_listp) == NULL) {
		LINK_SUCC(free_listp, bp);
		LINK_SUCC(free_tailp, bp);
		LINK_SUCC(bp, NULL);
		LINK_PRED(bp, NULL);
	} else {
		/* Insert into the list head */
		if ((char *)bp < (char *)SUCC(free_listp)) {
			/* The address of bp is greater than 
			 * the one of the first element of free list */
			LINK_SUCC(bp, SUCC(free_listp));
			LINK_PRED(SUCC(free_listp), bp);
			LINK_PRED(bp, NULL);
			LINK_SUCC(free_listp, bp);
		} 
		/* Insert into the list tail */
		else if ((char *)bp > (char *)SUCC(free_tailp)) {
			LINK_PRED(bp, SUCC(free_tailp));
			LINK_SUCC(SUCC(free_tailp), bp);
			LINK_SUCC(bp, NULL);
			LINK_SUCC(free_tailp, bp);
		} 
		/* Internal */
		else {
			char *pred = (char *)SUCC(free_listp);
			while (SUCC(pred) && (char *)SUCC(pred) < (char *)bp) {
				pred = (char *)SUCC(pred);
			}
			LINK_PRED(bp, pred);
			LINK_SUCC(bp, SUCC(pred));
			LINK_PRED(SUCC(pred), bp);
			LINK_SUCC(pred, bp);
		}
	}
}

/*
 * remove_list - Remove the free block pointed by bp
 *               from its free list
 *               And set the free_tailp(global variable) point to the end of list
 */
static void remove_list(void *bp)
{ 
	int index = INDEX(GET_SIZE(HDRP(bp)));
	char *free_listp = list_head + index * WSIZE;
	char *free_tailp = list_tail + index * WSIZE;
	/* Empty list */
    if (SUCC(bp) == NULL && PRED(bp) == NULL) { 
        LINK_SUCC(free_listp, NULL);  
		LINK_SUCC(free_tailp, NULL);
    } 
	/* The tail of list */
	else if (SUCC(bp) == NULL && PRED(bp) != NULL) {  
		LINK_SUCC(free_tailp, PRED(bp));
        LINK_SUCC(PRED(bp), NULL);
    } 
	/* The head of list */
	else if (SUCC(bp) != NULL && PRED(bp) == NULL) {   
        LINK_SUCC(free_listp, SUCC(bp));  
        LINK_PRED(SUCC(bp), NULL);  
    } 
	/* Internal */
	else if (SUCC(bp) != NULL && PRED(bp) != NULL) {  
        LINK_SUCC(PRED(bp), SUCC(bp));  
        LINK_PRED(SUCC(bp), PRED(bp)); 
    } 
}

/* 
 * coalesce - Coalesce a free block immediately
 *            Based on the code from text
 */
static void *coalesce(void *bp)
{
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t size = GET_SIZE(HDRP(bp));

	if (prev_alloc && next_alloc) {
		;
	}

	else if (prev_alloc && !next_alloc) {
		remove_list(NEXT_BLKP(bp));
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
	}

	else if (!prev_alloc && next_alloc) {
		remove_list(PREV_BLKP(bp));
		size += GET_SIZE(HDRP(PREV_BLKP(bp)));
		PUT(FTRP(bp), PACK(size, 0));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		bp = PREV_BLKP(bp);
	}
	else {
		remove_list(NEXT_BLKP(bp));
		remove_list(PREV_BLKP(bp));
		size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
		bp = PREV_BLKP(bp);
	}
	insert_list(bp);
	return bp;
}

/*
 * extend_heap - Extend head when no fit found
 *               Code from text
 */
static void *extend_heap(size_t words)
{
	char *bp;
	size_t size;

	size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
	/* Optimized for the case where the last block is free, seems useless for the traces */
	// char *lpftr = mem_heap_hi() - 3;
	// if (!GET_ALLOC(lpftr)) {
	// 	size -= GET_SIZE(lpftr);
	// }
	/*----------------------------------------------------------------------------------*/
	if ((long)(bp = mem_sbrk(size)) == -1)
		return NULL;

	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

	return coalesce(bp);
}

/* 
 * mm_init - Initialize the malloc package.
 *           Including: Prologue Block, Epilogue, Free List Head Array and Free List Tail Array
 */
int mm_init(void)
{
	basep = (char *)mem_heap_lo();
	if ((heap_listp = (char *)mem_sbrk((2 * LIST_NUM + 4)*WSIZE)) == (void *)-1)
		return -1;
	list_head = heap_listp;
	list_tail = heap_listp + LIST_NUM * WSIZE;
	heap_listp += 2 * LIST_NUM * WSIZE;
	PUT(heap_listp, 0);
	PUT(heap_listp + 1*WSIZE, PACK(DSIZE, 1));
	PUT(heap_listp + 2*WSIZE, PACK(DSIZE, 1));
	PUT(heap_listp + 3*WSIZE, PACK(0, 1));
	heap_listp += 2 * WSIZE;

	for (int i = 0; i < LIST_NUM; i++) {
		LINK_SUCC(list_head + i * WSIZE, NULL);
		LINK_SUCC(list_tail + i * WSIZE, NULL);
	}

	if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
		return -1;
	return 0;
}

/* 
 * find_fit - Use first fit in the exact list and larger lists
 */
static void *find_fit(size_t asize)
{
	unsigned int *ptr;
	
	for (int index = INDEX(asize); index < LIST_NUM; index++) {
		ptr = SUCC(list_head + index * WSIZE);
		while (ptr != NULL) {
			if (GET_SIZE(HDRP(ptr)) >= asize) {
				return (void *)ptr;
			}
			ptr = SUCC(ptr);
		}
	}
	
	return NULL;
}

/*
 * place - Place asize in the block pointed by bp
 *         Make some optimizations
 */
static void *place(void *bp, size_t asize)
{
	/* 
	 * // Ordinary version //
	 *	size_t csize = GET_SIZE(HDRP(bp));
	 *	remove_list(bp);
	 *	if ((csize - asize) >= BLKSIZE) {
	 *		PUT(HDRP(bp), PACK(asize, 1));
	 *		PUT(FTRP(bp), PACK(asize, 1));
	 *		void *n_bp = NEXT_BLKP(bp);
	 *		PUT(HDRP(n_bp), PACK(csize-asize, 0));
	 *		PUT(FTRP(n_bp), PACK(csize-asize, 0));
	 *		insert_list(n_bp);
	 *	} else {
	 *		PUT(HDRP(bp), PACK(csize, 1));
	 *		PUT(FTRP(bp), PACK(csize, 1));
	 *	}
	 *	return bp; 
	 */
	
	/* 
	 * Optimized for trace7&8 
	 * Place the smaller block in the left of the free block
	 * The bigger one the right
	 * But may result small fragments
	 */
	size_t prev_size = GET_SIZE(FTRP(PREV_BLKP(bp)));
	size_t csize = GET_SIZE(HDRP(bp));
	remove_list(bp);
	if ((csize - asize) < BLKSIZE) {
		PUT(HDRP(bp), PACK(csize, 1));
		PUT(FTRP(bp), PACK(csize, 1));
	} else {
		if (prev_size == DSIZE || asize <= prev_size) {
			/* Previous block is Prologue Block or palced size is bigger the prev_size */
			/* Left assigned right free */
			PUT(HDRP(bp), PACK(asize, 1));
			PUT(FTRP(bp), PACK(asize, 1));
			void *n_bp = NEXT_BLKP(bp);
			PUT(HDRP(n_bp), PACK(csize-asize, 0));
			PUT(FTRP(n_bp), PACK(csize-asize, 0));
			insert_list(n_bp);
		} else {
			/* Right assigned left free */
			PUT(HDRP(bp), PACK(csize-asize, 0));
			PUT(FTRP(bp), PACK(csize-asize, 0));
			insert_list(bp);
			bp = NEXT_BLKP(bp);
			PUT(HDRP(bp), PACK(asize, 1));
			PUT(FTRP(bp), PACK(asize, 1));
		}
	}
	return bp;
}

/* 
 * mm_malloc - Allocate a block of size
 *             Code from text
 */
void *mm_malloc(size_t size)
{
	size_t asize;
	size_t extendsize;
	char *bp;

	if (size == 0)
		return NULL;

	if (size <= DSIZE)
		asize = BLKSIZE;
	else
		asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);

	if ((bp = find_fit(asize)) != NULL) {
		/* Due to the optimizations in place(), bp may change */
		return place(bp, asize);
	}

	extendsize = MAX(asize, CHUNKSIZE);
	if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
		return NULL;
	return place(bp, asize);
}

/*
 * mm_free - Free and coalesce a block
 */
void mm_free(void *ptr)
{
	size_t size = GET_SIZE(HDRP(ptr));

	PUT(HDRP(ptr), PACK(size, 0));
	PUT(FTRP(ptr), PACK(size, 0));
	coalesce(ptr);
}

/*
 * mm_realloc - Realloc a block
 *              Use the method of coalesce, when the next or previous block is free and the space is enough,
 *              coalesce and split to realloc, otherwise call mm_malloc
 */
void *mm_realloc(void *ptr, size_t size)
{
	size_t asize;
    void *oldptr = ptr;
    void *newptr;
	if (ptr == NULL) {
		return mm_malloc(size);
	}
	if (size == 0) {
		mm_free(ptr);
		return NULL;
	}

	if (size <= DSIZE)
		asize = BLKSIZE;
	else
		asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);

	size_t oldSize = GET_SIZE(HDRP(ptr));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
	size_t next_size = GET_SIZE(HDRP(NEXT_BLKP(ptr)));
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(ptr)));
	size_t prev_size = GET_SIZE(FTRP(PREV_BLKP(ptr)));

	size_t copySize = oldSize - DSIZE;

	if (asize == oldSize) {
		return ptr;
	}

	else if (asize > oldSize) {
		if (!next_alloc && asize <= oldSize + next_size) {
			/* The next block is free and the space is enough */
			remove_list(NEXT_BLKP(ptr));
			if (oldSize + next_size - asize >= BLKSIZE) {
				PUT(HDRP(ptr), PACK(asize, 1));
				PUT(FTRP(ptr), PACK(asize, 1));
				void *n_ptr = NEXT_BLKP(ptr);
				PUT(HDRP(n_ptr), PACK(oldSize + next_size - asize, 0));
				PUT(FTRP(n_ptr), PACK(oldSize + next_size - asize, 0));
				insert_list(n_ptr);
			} else {
				PUT(HDRP(ptr), PACK(oldSize + next_size, 1));
				PUT(FTRP(ptr), PACK(oldSize + next_size, 1));
			}
			return ptr;
		}

		if (!prev_alloc && asize <= oldSize + prev_size) {
			/* The previous block is free and the space is enough */
			newptr = PREV_BLKP(ptr);
			remove_list(newptr);
			memmove(newptr, oldptr, copySize); // replace memcpy
			if (oldSize + prev_size - asize >= BLKSIZE) {
				PUT(HDRP(newptr), PACK(asize, 1));
				PUT(FTRP(newptr), PACK(asize, 1));
				void *n_ptr = NEXT_BLKP(newptr);
				PUT(HDRP(n_ptr), PACK(oldSize + prev_size - asize, 0));
				PUT(FTRP(n_ptr), PACK(oldSize + prev_size - asize, 0));
				insert_list(n_ptr);
			} else {
				PUT(HDRP(newptr), PACK(oldSize + prev_size, 1));
				PUT(FTRP(newptr), PACK(oldSize + prev_size, 1));
			}
			return newptr;
		}

		/* Ordinary case */
		newptr = mm_malloc(size);
		if (newptr == NULL)
			return NULL;
		memcpy(newptr, oldptr, copySize);
		mm_free(oldptr);
		return newptr;
	}

	else if (asize < oldSize) {
		/* No need to copy */
		if (oldSize - asize >= BLKSIZE) {
			PUT(HDRP(ptr), PACK(asize, 1));
			PUT(FTRP(ptr), PACK(asize, 1));
			void *n_ptr = NEXT_BLKP(ptr);
			PUT(HDRP(n_ptr), PACK(oldSize - asize, 0));
			PUT(FTRP(n_ptr), PACK(oldSize - asize, 0));
			insert_list(coalesce(n_ptr));
		} 
		return ptr;
	}
	return NULL;
}

#define IS_ALIGN(bp) (ALIGN((size_t)(bp)) == (size_t)(bp))

static int out_of_bound(void *bp)
{
	if (bp == NULL) {
		return 1;
	}
	return (((char *)(bp) < (char *)mem_heap_lo()) || ((char *)(bp) > (char *)mem_heap_hi()));
}

/*
 * check_block - Check a block:
 *               1. All blocks are in the heap.
 *               2. All blocks are aligned.
 *               3. Header is the same as footer.
 */
static int check_block(void *bp) 
{
	int flag = 1;
	if (out_of_bound(bp)) {
		printf("Block out of boundary.\n");
		flag = 0;
	}

	if (!IS_ALIGN(bp)) {
		printf("Bad block alignment.\n");
		flag = 0;
	}

	if (GET(HDRP(bp)) != GET(FTRP(bp))) {
		printf("Header differs from footer.\n");
		flag = 0;
	}

	return flag;
}

/* 
 * check_list - Check the free list:
 *              1. All lists are in the heap.
 *              2. Every blocks are checked.
 *              3. No Allocated block in free list.
 *              4. Every block is the predecessor of its successor
 */
static int check_list()
{
	int flag = 1;
	for (int i = 0; i < LIST_NUM; i++) {
		char *free_listp = list_head + i * WSIZE;
		char *free_tailp = list_tail + i * WSIZE;
		if (free_listp == NULL || free_tailp == NULL) 
			continue;
		if (out_of_bound(SUCC(free_listp)) || out_of_bound(SUCC(free_tailp))) {
			printf("Free list out of boundary.\n");
			flag = 0;
		} else {
			for (char *fp = (char *)SUCC(free_listp); fp != (char *)SUCC(free_tailp) ;fp = (char *)SUCC(fp)) {
			check_block(fp);
            if (GET_ALLOC(HDRP(fp)))
                printf("Allocated block in free list.\n");
				flag = 0;
            if (fp != (char *)SUCC(free_tailp) && (char *)PRED(SUCC(fp)) != fp)
                printf("Bad link.\n");
				flag = 0;
			}
		}
	}
	return flag;
}

/*
 * mm_check - Check all:
 *            1. Validity of prologue and epilogue.
 *            2. All blocks are valid.
 *            3. No free block adjacent to another free block.
 *            4. Validity of free list.
 */
int mm_check(void)
{
	int flag = 1;
	if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || (!GET_ALLOC(HDRP(heap_listp)))) {
		printf("Bad prologue block.\n");
		flag = 0;
	}
	flag = check_block(heap_listp);
	char *bp;
	for (bp = (char *)NEXT_BLKP(heap_listp); GET_SIZE(HDRP(bp)) > 0; bp = (char *)NEXT_BLKP(bp)) {
		flag = check_block(bp);
		if (!GET_ALLOC(HDRP(bp)) && !GET_ALLOC(HDRP(NEXT_BLKP(bp)))) {
			printf("Free block escaping coalescing.\n");
			flag = 0;
		}
	}
	if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp)))) {
		 printf("Bad epilogue block.\n");
		 flag = 0;
	}

	flag = check_list();
    return flag;
}
