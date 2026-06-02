/**
  ******************************************************************************
  * @file           : adc.h
  * @brief          : Generic ADC Helper Utility Header.
  ******************************************************************************
  */

#ifndef INC_ADC_H_
#define INC_ADC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>

/* Exported Functions --------------------------------------------------------*/

/**
 * @brief Reads the raw digital value of a single ADC channel by polling for conversion.
 * @param hadc Handle to configured ADC peripheral
 * @return Raw ADC digitized reading (0 to 4095)
 */
uint32_t ADC_Read(ADC_HandleTypeDef *hadc);

#ifdef __cplusplus
}
#endif

#endif /* INC_ADC_H_ */
