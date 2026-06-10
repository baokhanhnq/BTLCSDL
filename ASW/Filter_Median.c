#include "Filter_Median.h"
#include <stddef.h>

static inline void SWAP(uint16_t *a, uint16_t *b) {
    if (*a > *b) {
        uint16_t t = *a;
        *a = *b;
        *b = t;
    }
}

void Filter_Median_Init(Filter_Median_t *filter, uint16_t initial_val) {
    if (filter == NULL) return;
    for (int i = 0; i < MEDIAN_FILTER_SIZE; i++) {
        filter->buffer[i] = initial_val;
    }
    filter->index = 0;
}

uint16_t Filter_Median_Update(Filter_Median_t *filter, uint16_t new_val) {
    if (filter == NULL) return 0;
    
    filter->buffer[filter->index] = new_val;
    filter->index = (filter->index + 1) % MEDIAN_FILTER_SIZE;
    
    uint16_t a = filter->buffer[0];
    uint16_t b = filter->buffer[1];
    uint16_t c = filter->buffer[2];
    uint16_t d = filter->buffer[3];
    uint16_t e = filter->buffer[4];

    /* 5-element sorting network for median extraction */
    SWAP(&a, &b);
    SWAP(&d, &e);

    SWAP(&a, &c);
    SWAP(&b, &c);

    SWAP(&a, &d);
    SWAP(&c, &d);

    SWAP(&b, &e);
    SWAP(&b, &c);

    SWAP(&d, &e);
    SWAP(&c, &d);

    return c;
}
