#ifndef LDR_H
#define LDR_H

#include <stdint.h>

/**
 * @brief Initialize the LDR sensor (ADC channel).
 */
void ldr_init();

/**
 * @brief Read the LDR sensor value in millivolts.
 *
 * @return LDR value in millivolts.
 */
uint16_t ldr_read();

#endif