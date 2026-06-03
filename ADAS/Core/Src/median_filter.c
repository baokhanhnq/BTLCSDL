#include "median_filter.h"

static inline void SWAP(uint32_t *a, uint32_t *b)
{
    if(*a > *b)
    {
        uint32_t t = *a;
        *a = *b;
        *b = t;
    }
}

uint32_t MedianFilter5(uint32_t *arr)
{
    uint32_t a = arr[0];
    uint32_t b = arr[1];
    uint32_t c = arr[2];
    uint32_t d = arr[3];
    uint32_t e = arr[4];

    SWAP(&a,&b);
    SWAP(&d,&e);

    SWAP(&a,&c);
    SWAP(&b,&c);

    SWAP(&a,&d);
    SWAP(&c,&d);

    SWAP(&b,&e);
    SWAP(&b,&c);

    SWAP(&d,&e);
    SWAP(&c,&d);

    return c;
}
