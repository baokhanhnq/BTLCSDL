#ifndef ASW_FILTER_MEDIAN_H_
#define ASW_FILTER_MEDIAN_H_

#include <stdint.h>

#define MEDIAN_FILTER_SIZE 5

typedef struct {
    uint16_t buffer[MEDIAN_FILTER_SIZE];
    uint8_t index;
} Filter_Median_t;

void Filter_Median_Init(Filter_Median_t *filter, uint16_t initial_val);
uint16_t Filter_Median_Update(Filter_Median_t *filter, uint16_t new_val);

#endif /* ASW_FILTER_MEDIAN_H_ */
