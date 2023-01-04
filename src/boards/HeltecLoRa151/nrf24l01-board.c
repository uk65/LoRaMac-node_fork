
#include "nrf24l01-board.h"
#include "board-config.h"


void NRF24L01IoInit(void)
{
}

void NRF24L01IoIrqInit(NRF24L01IrqHandler *irqHandler)
{
    // active low ==> falling edge
    GpioSetInterrupt(&NRF24L01.INT, IRQ_FALLING_EDGE, IRQ_HIGH_PRIORITY, irqHandler);
}

void NRF24L01IoDeInit(void)
{
    GpioInit(&NRF24L01.Spi.Nss, RF24_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1);

    GpioInit(&NRF24L01.CE, RF24_CE, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0);
    GpioInit(&NRF24L01.INT, RF24_INT, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0);
}
