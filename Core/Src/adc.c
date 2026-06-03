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
    if (hadc == NULL || hadc->Instance == NULL) {
        return 0;
    }
    
    ADC_TypeDef *ADCx = hadc->Instance;
    
    // 1. Bắt đầu chuyển đổi bằng cách ghi cờ SWSTART vào thanh ghi CR2 (Register-level)
    ADCx->CR2 |= ADC_CR2_SWSTART;
    
    // 2. Chờ chuyển đổi hoàn tất bằng cách kiểm tra cờ EOC (End of Conversion) trong thanh ghi SR
    // Có thêm timeout bảo vệ chống treo cứng CPU
    uint32_t timeout = 10000;
    while (!(ADCx->SR & ADC_SR_EOC)) {
        timeout--;
        if (timeout == 0) {
            return 0; // Trả về 0 nếu bị timeout
        }
    }
    
    // 3. Đọc dữ liệu chuyển đổi từ thanh ghi dữ liệu DR (Data Register)
    return ADCx->DR;
}
