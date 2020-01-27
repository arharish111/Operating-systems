
//Student Name : Harish Harish



#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct block *)(ptr) - 1)


static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct block 
{
   size_t      size;  /* Size of the allocated block of memory in bytes */
   struct block *next;  /* Pointer to the next block of allcated memory   */
   bool        free;  /* Is this block free?                     */
};


struct block *FreeList = NULL; /* Free list to track the blocks available */
struct block *startBlock = NULL;

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free blocks
 * \param size size of the block needed in bytes 
 *
 * \return a block that fits the request or NULL if no free block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct block *findFreeBlock(struct block **last, size_t size) 
{
   struct block *curr = FreeList;

#if defined FIT && FIT == 0
   /* First fit */
    //if(curr)
    //printf("\nfindfree:%d,%zu",curr->free,curr->size);
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

#if defined BEST && BEST == 0
   //printf("TODO: Implement best fit here\n");
   size_t blockSize = 0;
   int foundCheck = 0;
   struct block *curr1 = FreeList;
   while (curr1) 
   {
	  if(curr1->free)
	  {
		  if(curr1->size >= size)
		  {
			if(blockSize==0)
			{
				blockSize = curr1->size-size;
				curr = curr1;
				foundCheck = 1;
			}
			else if(blockSize >= curr1->size-size)
			{
				blockSize = curr1->size-size;
				curr = curr1;
				foundCheck = 1;
			}
			curr1 = curr1->next;
			
		  }
		  else
		  {
			if(!foundCheck)
			{
				*last = curr1;
				curr1 = curr1->next;
				curr = curr1;
			}
			else
			{
				curr1 = curr1->next;
			}
		  }
	  }
	  else
	  {
		  if(!foundCheck)
			{
				*last = curr1;
				curr1 = curr1->next;
				curr = curr1;
			}
			else
			{
				curr1 = curr1->next;
			}
	  }
   }
#endif

#if defined WORST && WORST == 0
   //printf("TODO: Implement worst fit here\n");
   size_t blockSize = 0;
   int foundCheck = 0;
   struct block *curr1 = FreeList;
   while (curr1) 
   {
	  if(curr1->free)
	  {
		  if(curr1->size >= size)
		  {
			if(blockSize==0)
			{
				blockSize = curr1->size-size;
				curr = curr1;
				foundCheck = 1;
			}
			else if(blockSize <= curr1->size-size)
			{
				blockSize = curr1->size-size;
				curr = curr1;
				foundCheck = 1;
			}
			curr1 = curr1->next;
			
		  }
		  else
		  {
			if(!foundCheck)
			{
				*last = curr1;
				curr1 = curr1->next;
				curr = curr1;
			}
			else
			{
				curr1 = curr1->next;
			}
		  }
	  }
	  else
	  {
		  if(!foundCheck)
			{
				*last = curr1;
				curr1 = curr1->next;
				curr = curr1;
			}
			else
			{
				curr1 = curr1->next;
			}
	  }
   }
#endif

#if defined NEXT && NEXT == 0
   //printf("TODO: Implement next fit here\n");
   int endFound = 0;
  // if(curr)
  // printf("\nfindfree:%d,%zu,%p",curr->free,curr->size,curr);
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
      FreeList = curr;
      if(FreeList == NULL && endFound == 0)
      {
         curr = startBlock;
         FreeList = curr;
         endFound = 1;
      }
   }
#endif

   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated block of NULL if failed
 */
struct block *growHeap(struct block *last, size_t size) 
{
	num_grows = num_grows + 1;
	num_blocks = num_blocks + 1;
	max_heap = max_heap + size;
  // if(last != NULL)
  // printf("\nlast:%p",last);
   /* Request more space from OS */
   struct block *curr = (struct block *)sbrk(0);
   
   // printf("\ncurr1:%p",curr);
  
   struct block *prev = (struct block *)sbrk(sizeof(struct block) + size);

   assert(curr == prev);


   /* OS allocation failed */
   if (curr == (struct block *)-1) 
   {
      return NULL;
   }

   /* Update FreeList if not set */
   if (FreeList == NULL) 
   {
      FreeList = curr;
   }
   
   /* Attach new block to prev block */
   if (last) 
   {
      last->next = curr;
   }
//    printf("\nlast:%p",last->next);
   /* Update block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
  // printf("\ncurr2:%zu,%p,%d",curr->size,curr->next,curr->free);
  // printf("c_s:%p",curr); 
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free block of heap memory for the calling process.
 * if there is no free block that satisfies the request then grows the 
 * heap and returns a new block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{
   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }
   num_mallocs = num_mallocs + 1;
   /* Align to multiple of 4 */
   size = ALIGN4(size);
   //printf("size:%zu",size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }
   num_requested = num_requested + size;
   /* Look for free block */
   struct block *last = FreeList;
   struct block *next = findFreeBlock(&last, size);
   
   if(next != NULL)
	num_reuses = num_reuses + 1;

   /* TODO: Split free block if possible */
   size_t req_size = (sizeof(struct block)+size);
   if(next !=NULL)
  // printf("req:%zu,ns:%zu",req_size,next->size);
   if (next != NULL && next->size > req_size)
   {
        // printf("splits:%d,blocks:%d\n",num_splits,num_blocks);
	   num_splits = num_splits + 1;
	   num_blocks =num_blocks + 1;
	   size_t curr;
         struct block *curr1 = NULL;
         unsigned int next_hex = (intptr_t)next;
	   struct block *tempBlock = next->next;
        // printf("block:%lu,size:%zu\n",sizeof(struct block),size);
	   unsigned int  blockSize = sizeof(struct block) + size;
        // printf("bs:%x\n",blockSize);
         //printf("%x,%x\n",next_hex,blockSize);
	   curr = (size_t)(next_hex + blockSize);
        // printf("splitCurr:%zu\n",curr);
         curr1 = (struct block *)curr;
         //printf("c1:%p",curr1);
	   curr1->free = true;
	   curr1->size = next->size - (size_t)blockSize;
	   curr1->next = tempBlock;
	   next->size = size;
	   next->next = curr1;
   }
   /* Could not find free block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
   }
   //printf("\nnext:%p",next);
   /* Could not find free block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }
   if(startBlock == NULL)
   {
	   startBlock = next;
   }
   /* Mark block as in use */
   next->free = false;

   /* Return data address associated with block */
   return BLOCK_DATA(next);
}

/*
 * \brief free
 *
 * frees the memory block pointed to by pointer. if the block is adjacent
 * to another block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   if (ptr == NULL) 
   {
      return;
   }
   num_frees = num_frees + 1;
   /* Make block as free */
   struct block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;
   // if(currBlock != NULL)
   // {
      // currBlock = curr;
   // }
   // /* TODO: Coalesce free blocks if needed */
// //   printf("%p,%p",(currBlock->next),curr);
   // if(&(currBlock->next) == &curr)
   // {
      // printf("%p,%p",&(currBlock->next),&curr);
   // }
   // if(curr->next != NULL)
   // {
      // if(curr->next->free == 1)
      // {
         // curr->next = curr->next->next;
         // curr->size = curr->size + curr->next->size;
      // }
   // }  
   struct block *start = startBlock;
   while(start)
   {
	  //printf("start1:%p\n",start);
	   if(start->free)
	   {
		   if(start->next != NULL)
		   {
			   if(start->next->free)
			   {
				   //printf("start2:%p\n",start);
				   num_coalesces = num_coalesces + 1;
				   num_blocks = num_blocks - 1;
				   start->size = start->size+start->next->size;
				   start->next = start->next->next;
				  // FreeList = start;
			   }
			   else
				start = start->next;
				
		   }
		   else
			break;
	   }
	   else
		start = start->next;
   }
}

/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
