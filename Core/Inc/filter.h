/**
  ******************************************************************************
  * @file           : filter.h
  * @brief          : Median Filter algorithm module header.
  ******************************************************************************
  */

#ifndef INC_FILTER_H_
#define INC_FILTER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported Defines ----------------------------------------------------------*/
#define MEDIAN_FILTER_SIZE 5

/* Exported Types ------------------------------------------------------------*/

/**
 * @brief Circular buffer context for the rolling Median Filter.
 */
typedef struct {
    uint16_t buffer[MEDIAN_FILTER_SIZE]; /*!< Circular buffer for raw samples */
    uint8_t index;                       /*!< Active write index in the circular buffer */
} MedianFilter_t;

/* Exported Functions --------------------------------------------------------*/

/**
 * @brief Initializes the median filter context with a safe initial default value.
 * @param filter Pointer to filter context
 * @param initial_val Default initial distance or value (e.g. 100 cm)
 */
void Filter_Init(MedianFilter_t *filter, uint16_t initial_val);

/**
 * @brief Inserts a new raw value into the rolling filter, sorts the buffer, and returns the median value.
 * @param filter Pointer to filter context
 * @param new_val Latest raw sensor measurement
 * @return Smoothed/filtered median value
 */
uint16_t Filter_Update(MedianFilter_t *filter, uint16_t new_val);

#ifdef __cplusplus
}
#endif

#endif /* INC_FILTER_H_ */
