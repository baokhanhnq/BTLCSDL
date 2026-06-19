#include "Filter_Median.h"
#include <stddef.h>

/*
 * Doi cho hai gia tri neu chua dung thu tu tang dan.
 */
static inline void Filter_Median_Swap(uint16_t *a, uint16_t *b)
{
    if (*a > *b)
    {
        uint16_t temp;

        temp = *a;
        *a = *b;
        *b = temp;
    }
}

/*
 * Khoi tao bo dem loc trung vi bang gia tri an toan ban dau.
 */
void Filter_Median_Init(Filter_Median_t *filter, uint16_t initial_val)
{
    uint8_t i;

    if (filter == NULL)
    {
        return;
    }

    for (i = 0U; i < MEDIAN_FILTER_SIZE; i++)
    {
        filter->buffer[i] = initial_val;
    }

    filter->index = 0U;
}

/*
 * Cap nhat bo loc trung vi 5 mau va tra ve gia tri trung vi.
 */
uint16_t Filter_Median_Update(Filter_Median_t *filter, uint16_t new_val)
{
    uint16_t a;
    uint16_t b;
    uint16_t c;
    uint16_t d;
    uint16_t e;

    if (filter == NULL)
    {
        return 0U;
    }

    filter->buffer[filter->index] = new_val;
    filter->index = (uint8_t)((filter->index + 1U) % MEDIAN_FILTER_SIZE);

    a = filter->buffer[0];
    b = filter->buffer[1];
    c = filter->buffer[2];
    d = filter->buffer[3];
    e = filter->buffer[4];

    /* Mang so sanh 5 gia tri; c la gia tri trung vi sau khi sap xep. */
    Filter_Median_Swap(&a, &b);
    Filter_Median_Swap(&d, &e);
    Filter_Median_Swap(&a, &c);
    Filter_Median_Swap(&b, &c);
    Filter_Median_Swap(&a, &d);
    Filter_Median_Swap(&c, &d);
    Filter_Median_Swap(&b, &e);
    Filter_Median_Swap(&b, &c);
    Filter_Median_Swap(&d, &e);
    Filter_Median_Swap(&c, &d);

    return c;
}
