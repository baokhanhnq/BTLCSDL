/**
  ******************************************************************************
  * @file           : filter.c
  * @brief          : Median Filter algorithm implementation.
  ******************************************************************************
  */

#include "filter.h"
#include <string.h>

/**
 * @brief Initializes the median filter context with a safe initial default value.
 * @param filter Pointer to filter context
 * @param initial_val Default initial distance or value (e.g. 100 cm)
 */
void Filter_Init(MedianFilter_t *filter, uint16_t initial_val) {
    if (filter == NULL) return;
    
    for (int i = 0; i < MEDIAN_FILTER_SIZE; i++) {
        filter->buffer[i] = initial_val;
    }
    filter->index = 0;
}

/**
 * @brief Inserts a new raw value into the rolling filter, sorts the buffer, and returns the median value.
 * @param filter Pointer to filter context
 * @param new_val Latest raw sensor measurement
 * @return Smoothed/filtered median value
 */
uint16_t Filter_Update(MedianFilter_t *filter, uint16_t new_val) {
    if (filter == NULL) return 0;
    
    // Insert into circular buffer
    filter->buffer[filter->index] = new_val;
    filter->index = (filter->index + 1) % MEDIAN_FILTER_SIZE;
    
    // Copy the buffer for sorting to avoid corrupting the original circular buffer history
    uint16_t sorted[MEDIAN_FILTER_SIZE];
    memcpy(sorted, filter->buffer, sizeof(sorted));
    
    // Sort array in ascending order (simple Bubble Sort for 5 elements)
    for (int i = 0; i < (MEDIAN_FILTER_SIZE - 1); i++) {
        for (int j = i + 1; j < MEDIAN_FILTER_SIZE; j++) {
            if (sorted[i] > sorted[j]) {
                uint16_t temp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = temp;
            }
        }
    }
    
    // Extract the median element (index 2 for size of 5)
    return sorted[MEDIAN_FILTER_SIZE / 2];
}
