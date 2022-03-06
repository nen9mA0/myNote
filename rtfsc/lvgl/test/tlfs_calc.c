#include <stdio.h>

#ifdef TLSF_64BIT
    #define TLSF_FLS(n) ((n) & 0xffffffff00000000ull ? 32 + TLSF_FLS32((size_t)(n) >> 32) : TLSF_FLS32(n))
#else
    #define TLSF_FLS(n) TLSF_FLS32(n)
#endif

#define TLSF_FLS32(n) ((n) & 0xffff0000 ? 16 + TLSF_FLS16((n) >> 16) : TLSF_FLS16(n))
#define TLSF_FLS16(n) ((n) & 0xff00     ?  8 + TLSF_FLS8 ((n) >>  8) : TLSF_FLS8 (n))
#define TLSF_FLS8(n)  ((n) & 0xf0       ?  4 + TLSF_FLS4 ((n) >>  4) : TLSF_FLS4 (n))
#define TLSF_FLS4(n)  ((n) & 0xc        ?  2 + TLSF_FLS2 ((n) >>  2) : TLSF_FLS2 (n))
#define TLSF_FLS2(n)  ((n) & 0x2        ?  1 + TLSF_FLS1 ((n) >>  1) : TLSF_FLS1 (n))
#define TLSF_FLS1(n)  ((n) & 0x1        ?  1 : 0)


#define TLSF_LOG2_CEIL(n) ((n) & (n - 1) ? TLSF_FLS(n) : TLSF_FLS(n) - 1)


#define TLSF_MAX_POOL_SIZE 32 * 1024

#define SL_INDEX_COUNT_LOG2 5
#define ALIGN_SIZE_LOG2 2
#define ALIGN_SIZE (1 << ALIGN_SIZE_LOG2)
#define FL_INDEX_MAX TLSF_LOG2_CEIL(TLSF_MAX_POOL_SIZE)

#define SL_INDEX_COUNT (1 << SL_INDEX_COUNT_LOG2)
#define FL_INDEX_SHIFT (SL_INDEX_COUNT_LOG2 + ALIGN_SIZE_LOG2)
#define FL_INDEX_COUNT (FL_INDEX_MAX - FL_INDEX_SHIFT + 1)
#define SMALL_BLOCK_SIZE (1 << FL_INDEX_SHIFT)


int tlsf_fls(unsigned int word)
{
    return TLSF_FLS32(word) - 1;
}

int tlsf_ffs(unsigned int word)
{
    const unsigned int reverse = word & (~word + 1);
    return TLSF_FLS32(reverse) - 1;
}

size_t mapping_search_roundup(size_t size)
{
    if(size >= SMALL_BLOCK_SIZE)
    {
        const size_t round = (1 << (tlsf_fls(size) - SL_INDEX_COUNT_LOG2)) - 1;
        size += round;
    }
    return size;
}

int main()
{
    unsigned int a = 0x10;
    unsigned int b = 0x12;
    unsigned int zero = 0;

    printf("===FLS/FFS Test====\n");
    printf("a:\n\tfls:%d\n\tffs:%d\n", tlsf_fls(a), tlsf_ffs(a));
    printf("b:\n\tfls:%d\n\tffs:%d\n", tlsf_fls(b), tlsf_ffs(b));
    printf("zero:\n\tfls:%d\n\tffs:%d\n", tlsf_fls(zero), tlsf_ffs(zero));
    printf("a:\n\tlog2 ceil:%d\n", TLSF_LOG2_CEIL(a));
    printf("b:\n\tlog2 ceil:%d\n", TLSF_LOG2_CEIL(b));
    printf("zero:\n\tlog2 ceil:%d\n", TLSF_LOG2_CEIL(zero));

    printf("====TLFS Test====\n");

    int prev_fl = -1;
    int prev_sl = -1;
    int l_size = -1;
    int r_size = -1;
    int fl, sl;
    for(int i=0; i<TLSF_MAX_POOL_SIZE; i++)
    {
        int size = i;
        if(size < SMALL_BLOCK_SIZE)
        {
            /* Store small blocks in first list. */
            fl = 0;
            sl = size / (SMALL_BLOCK_SIZE / SL_INDEX_COUNT);
        }
        else
        {
            fl = tlsf_fls(size);
            sl = (size >> (fl - SL_INDEX_COUNT_LOG2)) ^ (1 << SL_INDEX_COUNT_LOG2);
            if( !(size >> (fl - SL_INDEX_COUNT_LOG2)) & (1 << SL_INDEX_COUNT_LOG2) )
                printf("WHY??????????????????\n");
            fl -= (FL_INDEX_SHIFT - 1);
        }
        if(fl != prev_fl || sl != prev_sl)
        {
            if(r_size != -1)
            {
                printf("Range: %d\t%d  \tfl:%d  sl:%d\n", l_size, r_size, prev_fl, prev_sl);
                printf("       %d\t%d\n", mapping_search_roundup(l_size), mapping_search_roundup(r_size));
            }
            l_size = size;
        }
        else
        {
            r_size = size;
        }
        prev_fl = fl;
        prev_sl = sl;
    }

}
