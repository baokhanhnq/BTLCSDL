/**
  ******************************************************************************
  * @file           : delay.h
  * @brief          : DWT Microsecond Delay Common Utility Header.
  ******************************************************************************
  */

#ifndef INC_DELAY_H_
#define INC_DELAY_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>

/* Exported Functions --------------------------------------------------------*/

/**
 * @brief Initializes the DWT (Data Watchpoint and Trace) hardware block.
 */
void Delay_DWT_Init(void);

/**
 * @brief Block execution for a cycle-accurate microsecond duration.
 * @param us Duration in microseconds
 */
void delay_us(uint16_t us);

#ifdef __cplusplus
}
#endif

#endif /* INC_DELAY_H_ */
