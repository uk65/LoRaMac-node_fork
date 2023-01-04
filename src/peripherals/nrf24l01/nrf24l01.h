
#ifndef _NRF24L01_H_
#define _NRF24L01_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "gpio.h"
#include "spi.h"


typedef struct NRF24L01_s
{
    Gpio_t      CE;
    Gpio_t      INT;
    Spi_t       Spi;
} NRF24L01_t;

typedef void (NRF24L01IrqHandler)(void *context);

extern NRF24L01_t NRF24L01;

#ifdef __cplusplus
}
#endif

#endif