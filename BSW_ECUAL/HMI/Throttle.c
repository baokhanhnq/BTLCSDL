#include "Throttle.h"
#include "adc.h"

void Throttle_Init(void) {
    ADC1_Init();
}

uint32_t Throttle_ReadAdc(void) {
    return (uint32_t)ADC1_Read();
}

uint16_t Throttle_GetPercent(uint32_t adc_val) {
    /* Map 12-bit ADC (0 - 4095) to percentage (0 - 100%) */
    uint32_t duty = (adc_val * 100) / 4095;
    if (duty > 100) {
        duty = 100;
    }
    return (uint16_t)duty;
}
