#include "bootpack.h"

/* Test the memory - return the size of the memory */
unsigned int memtest(unsigned int start, unsigned int end)
{
	/* We need to turn off cache when our CPU is above 486 */
	char flg486 = 0; /* Whether the cpu is above i486 */
	unsigned int eflg, cr0, i;
	
	/* i386 does not have AC bit in eflag and thus we cannot set this bit */
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT; /* Set AC-bit = 1 */
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if ((eflg & EFLAGS_AC_BIT)) { /* If we successfully set this bit, it is i486 */
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
	io_store_eflags(eflg); /* Store back */

	if (flg486) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; /* Disable Cache */
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; /* Enable Cache */
		store_cr0(cr0);
	}

	return i;
}

/* Explicit List*/
/* double word (8) alignment */
#define ALIGNMENT 8
/* Word/Footer/Header size and Double Word Size*/
#define WSIZE 4
#define DSIZE 8
/* List End Tag */
#define ENDL 0xFFFFFFFF
/* Get Max */
#define MAX(x, y) ((x) > (y) ? (x) : (y))
/* Get Min */
#define MIN(x, y) ((x) > (y) ? (y) : (x))
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
/* Read and write a word at address p */
#define GET(p) (* (unsigned int *)(p))
/* Get size/alloc/pre-alloc when p is a size tag */
#define GET_SIZE(p)	(GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_PREALLOC(p) ((GET(p) & 0x2) >> 1)
/* Given a block ptr, get its header size tag address */
#define SIZEP(bp) ((char *)(bp) - WSIZE)
/* Given an empty block ptr, get its footer size tag address */
#define FSIZEP(bp) ((char *)(bp) + GET_SIZE(SIZEP(bp)) - DSIZE)
/* Given an empty block ptr, get its prev link tag address */
#define PLP(bp) ((unsigned int *)(bp) + 1)
/* Pack together the size/alloc/prealloc in a box */
#define PACK(size, prealloc ,alloc) ((size) | (alloc) | (prealloc << 1))
/* Always points to the prologue */
static unsigned int * pro;
/* Tell the newly created block what pre-alloc it should be */
static unsigned int last_prev;
/* Always points to the last block */
static unsigned int * last_ptr;

/* The border of current memory */
static unsigned int* sbrk = 0;
static const unsigned int* end_memory = 0;

/* 
 * Memory checker 
 */
unsigned int mm_check(void){
    return (unsigned int)sbrk;
}

static unsigned int *mem_sbrk(unsigned int size){
	unsigned int *tmp;
	size >>= 2;
	// There is no memory left
	if((sbrk + size != 0 && sbrk + size < sbrk) || sbrk + size - 1 > end_memory) {
		return 0;
	}
	else{
		tmp = sbrk;
		sbrk += size;
		return tmp;
	}
}

/* 
 * mm_init - initialize the malloc package and set up pro, last_prev;
 */
void mm_init(unsigned int *start, unsigned int *end) 
{
	sbrk = start;
	end_memory = end;
	pro = (unsigned int *) mem_sbrk(WSIZE);
	*pro = ENDL;
	last_prev = 1;
}
/*
 * Assume bp is in the chain prev - bp - next.
 * This function dropes bp from the chain and reconnect prev and next
 */
inline void drop_chain(unsigned int * bp) 
{
	* ((unsigned int * ) * PLP(bp)) = * bp;
	// If the chain does not reach the end
	if(*bp != ENDL) 
	{
		* PLP((unsigned int *) *bp) = * PLP(bp);
	}
}
/*
 * Assume the chain is prev - next
 * This function puts bp into the chain: prev - bp - next
 */
inline void add_chain(unsigned int * prev, unsigned int * bp, unsigned int * next) 
{
	* prev = (unsigned int) bp;
	* ((unsigned int *)(bp)) = (unsigned int) next;
	* PLP(bp) = (unsigned int)prev;
	// If the chain does not reach the end
	if(((unsigned int)next)!=ENDL) 
	{
		* PLP(next) = (unsigned int) bp;
	}
}
/* 
 * mm_malloc - Allocate a block
 * This function iterates through the free list from the beginning. If there is big enough free
 * block in the list, we try to allocate this block. If this block is big enough, we split the 
 * block into two, drop the first half from the list as allocated one, and reconnect the rest. 
 * Otherwise, if no big enough free block left, we increment brk and create a new block.
 */
void *mm_malloc(unsigned int size) 
{
	unsigned int * bp = pro;
	// Iterate over the free list
	while(*bp != ENDL) 
	{
		bp = (unsigned int *) *bp;
		unsigned int bl_size = GET_SIZE(bp - 1);
		// When there is a free block with much more size that we can split the block into two
		if( bl_size >= size + WSIZE + 0x10) 
		{
			unsigned int new_size = ALIGN(size+WSIZE);
			* (bp-1) = PACK(new_size,1,1);
			unsigned int * free_block_size = (bp - 1 + (new_size >> 2));
			* free_block_size = PACK(bl_size - new_size,1,0);
			unsigned int * free_block_size_f = (unsigned int *) FSIZEP(free_block_size+1);
			* free_block_size_f = PACK(bl_size - new_size,1,0);
			add_chain((unsigned int*) *PLP(bp),free_block_size + 1, (unsigned int * ) *bp);
			if(last_ptr == bp) 
			{
				last_ptr = free_block_size + 1;
			}
			return bp;
		}
		// When there is a free block with about the same size 
        else if(bl_size >= size + WSIZE) 
		{
			if(last_ptr==bp) 
			{
				last_ptr = bp;
				last_prev = 1;
			}
			drop_chain(bp);
			*(bp-1) |= 0x3;
			* (bp + (GET_SIZE(bp-1)>>2) - 1) |= 2;
			return bp;
		}
	}
	// No big enough free block left
	unsigned int b_size = MAX(ALIGN(size + WSIZE),0x10); //b_size at least have to be 0x10
	unsigned int * sp = (unsigned int *) mem_sbrk(b_size);
	if(sp == 0) return 0; // No memory left
	*sp = PACK(b_size,last_prev,1);
	last_ptr = sp + 1;
	last_prev = 1;
	return sp + 1;
}
/*
 * mm_free - Free a block
 * When either or both of its adjacent block is also free, we coalesce them together to 
 * form a larger block. We drop the adjacent blocks from the list, and put the coalesced
 * big block at the beginning of the list.
 * This function first tests the coalescing case and then operates the blocks and the list.
 */
void mm_free(void *bptr) 
{
	unsigned int * bp = bptr;
	// If isP == 0, need to coalesce with the previous block
	unsigned int isP = GET_PREALLOC(SIZEP(bp));
	unsigned int * next_tag_p = bp+(GET_SIZE(SIZEP(bp))>>2)-1;
	// If isN == 0, need to coalesce  with the next block
	unsigned int isN = bp == last_ptr ? 1 : GET_ALLOC(next_tag_p);
	// If no block need to be coalesced
	if(isP && isN) 
	{
		if(bp == last_ptr) 
		{
			last_prev = 0;
			last_ptr = bp;
		}
		add_chain(pro,bp,(unsigned int *) *pro);
		(*(bp - 1)) &= 0xFFFFFFFE;
		(* (next_tag_p-1)) = (* (bp - 1));
		(* next_tag_p) &= 0xFFFFFFFD;
	}
	// If both sides need to be coalesced 
    else if(!(isP||isN)) 
	{
		unsigned int p_size_shr = (*(bp - 2)) >> 2;
		drop_chain(bp - p_size_shr);
		if(bp+(GET_SIZE(SIZEP(bp))>>2) == last_ptr) 
		{
			last_prev = 0;
			last_ptr = bp - p_size_shr;
		}
		drop_chain(bp+(GET_SIZE(SIZEP(bp))>>2));
		unsigned int sum_size = GET_SIZE(SIZEP(bp)) + GET_SIZE(bp+(GET_SIZE(SIZEP(bp))>>2)-1) + GET_SIZE(bp - p_size_shr - 1);
		unsigned int * next_size_tag = bp+(GET_SIZE(SIZEP(bp))>>2) - 1;
		next_size_tag += ((GET_SIZE(next_size_tag)>>2) - 1);
		(* (next_size_tag)) = PACK(sum_size,1,0);
		* (bp - p_size_shr - 1) = PACK(sum_size,1,0);
		add_chain(pro,bp - p_size_shr,(unsigned int *) *pro);
	}
	//If only the next block need to be coalesced
    else if(isP) 
	{
		if(bp+(GET_SIZE(SIZEP(bp))>>2) == last_ptr) 
		{
			last_prev = 0;
			last_ptr = bp;
		}
		drop_chain(bp+(GET_SIZE(SIZEP(bp))>>2));
		unsigned int sum_size = GET_SIZE(SIZEP(bp)) + GET_SIZE(bp+(GET_SIZE(SIZEP(bp))>>2)-1);
		unsigned int * next_size_tag = bp+(GET_SIZE(SIZEP(bp))>>2) - 1;
		next_size_tag += ((GET_SIZE(next_size_tag)>>2) - 1);
		(* (next_size_tag)) = PACK(sum_size,1,0);
		(*(bp - 1)) = PACK(sum_size,1,0);
		add_chain(pro,bp,(unsigned int *) *pro);
	}
	//If only the previous block need to be coalesced 
    else 
	{
		unsigned int p_size_shr = (*(bp - 2)) >> 2;
		drop_chain(bp - p_size_shr);
		if(bp == last_ptr) 
		{
			last_prev = 0;
			last_ptr = bp - p_size_shr;
		}
		unsigned int sum_size = GET_SIZE(SIZEP(bp)) + GET_SIZE(bp - p_size_shr - 1);
		* (bp - p_size_shr - 1) = PACK(sum_size,1,0);
		(* (next_tag_p-1)) = PACK(sum_size,1,0);
		(* next_tag_p) &= 0xFFFFFFFD;
		add_chain(pro,bp - p_size_shr,(unsigned int *) *pro);
	}
}
/*
 * mm_realloc - reallocate a allocated block
 * When the *ptr is null, this function is the same as malloc(size)
 * When the size is zero, this function is the same as free(ptr)
 * Otherwise, this function returns a new block with new size "size" and keeps the original data.
 * This function first test whether size is less than its original size. If so, we return the same
 * pointer. If the size is larger then its original size, we test whether the block behind is free
 * or not. If it is free and has enough space, we drop this block from the free list and return
 * still the same pointer. If none of the cases meet (the behind block is occupied), we have to
 * malloc a new space and copy and paste the original data.
 */
void *mm_realloc(void *ptr, unsigned int size) 
{
	unsigned int * bp = (unsigned int *) ptr;
	unsigned char * pt = (unsigned char *) ptr;
	// When the *ptr is null, this function is the same as malloc(size)
	if(ptr == 0)
	        return mm_malloc(size);
	// When the size is zero, this function is the same as free(ptr)
	if(size == 0) 
	{
		mm_free(ptr);
		return 0;
	}
	// When the new size is less than its original size
	unsigned int original_size = GET_SIZE(SIZEP(ptr))-WSIZE;
	if(size <= original_size) 
	{
		return ptr;
	}
	// When the new size is bigger and luckily the behind block is free and big enough that we can use
	unsigned int * next_block_tag = bp + (original_size >> 2);
	unsigned int next_block_size = GET_SIZE(next_block_tag);
	if(bp != last_ptr && GET_ALLOC(next_block_tag)==0 && next_block_size + original_size >= size) 
	{
		drop_chain(next_block_tag + 1);
		if(last_ptr == next_block_tag + 1) 
		{
			last_ptr = bp;
			last_prev = 1;
		}
		* (bp - 1) &= 03;
		* (bp - 1) |= (original_size + WSIZE + next_block_size);
		next_block_tag += (next_block_size>>2);
		(* next_block_tag) |= 0x2;
		return ptr;
	}
	// Neither of these cases meet, we have to malloc a new block
	unsigned char * p= (unsigned char *) mm_malloc(size);
	unsigned int min = MIN(original_size,size);
	// Copy and paste the data
	unsigned int i = 0;
	for (;i < min;++i) 
	{
		p[i] = pt[i];
	}
	// Free the original block
	mm_free(ptr);
	return (void*) p;
}
