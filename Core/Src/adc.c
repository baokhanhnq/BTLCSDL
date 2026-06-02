/**
  ******************************************************************************
  * @file           : adc.c
  * @brief          : Generic ADC Helper Utility Implementation.
  ******************************************************************************
  */

#include "adc.h"

/**
 * @brief Reads the raw digital value of a single ADC channel by polling for conversion.
 * @param hadc Handle to configured ADC peripheral
 * @return Raw ADC digitized reading (0 to 4095)
 */
uint32_t ADC_Read(ADC_HandleTypeDef *hadc) {
    uint32_t val = 0;
    if (hadc == NULL) {
        return 0;
    }
    
    HAL_ADC_Start(hadc);
    if (HAL_ADC_PollForConversion(hadc, 10) == HAL_OK) {
        val = HAL_ADC_GetValue(hadc);
    }
    HAL_ADC_Stop(hadc);
    
    return val;
}
