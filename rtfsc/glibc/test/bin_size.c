#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#ifndef INTERNAL_SIZE_T
# define INTERNAL_SIZE_T size_t
#endif

/* The corresponding word size.  */
#define SIZE_SZ (sizeof (INTERNAL_SIZE_T))

struct malloc_chunk {

  INTERNAL_SIZE_T      mchunk_prev_size;  /* Size of previous chunk (if free).  */
  INTERNAL_SIZE_T      mchunk_size;       /* Size in bytes, including overhead. */

  struct malloc_chunk* fd;         /* double links -- used only if free. */
  struct malloc_chunk* bk;

  /* Only used for large blocks: pointer to next larger size.  */
  struct malloc_chunk* fd_nextsize; /* double links -- used only if free. */
  struct malloc_chunk* bk_nextsize;
};


#define MALLOC_ALIGNMENT (2*SIZE_SZ)
#define MALLOC_ALIGN_MASK (MALLOC_ALIGNMENT - 1)


#define MIN_CHUNK_SIZE        (offsetof(struct malloc_chunk, fd_nextsize))

#define MINSIZE  \
  (unsigned long)(((MIN_CHUNK_SIZE+MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK))

#define request2size(req)                                         \
  (((req) + SIZE_SZ + MALLOC_ALIGN_MASK < MINSIZE)  ?             \
   MINSIZE :                                                      \
   ((req) + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK)

#define fastbin_index(sz) \
  ((((unsigned int)(sz)) >> (SIZE_SZ == 8 ? 4 : 3)) - 2)

#define MAX_FAST_SIZE (80 * SIZE_SZ / 4)

#define NFASTBINS (fastbin_index(request2size(MAX_FAST_SIZE)) + 1)



int main(void)
{
  printf("SIZE_SZ: %d\n", SIZE_SZ);
  printf("NFASTBINS: %d\n", NFASTBINS);
  printf("request2size(MAX_FAST_SIZE): %d\n", request2size(MAX_FAST_SIZE));

  int prev_index = 10000;
  int index;
  for(int i=0; i<=MAX_FAST_SIZE; i++)
  {
    index = request2size(i);
    if(index != prev_index)
    {
      printf("size: %d  index:%d\n", i, index);
      prev_index = index;
    }
  }
}