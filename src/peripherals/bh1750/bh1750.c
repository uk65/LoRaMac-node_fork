
#include <stdbool.h>
#include "bh1750.h"

#ifdef PERIPHERAL_BH1750

#define BH1750_POWER_DOWN     0x00          // No active state
#define BH1750_POWER_ON       0x01          // Waiting for measurement command
#define BH1750_RESET          0x07          // Reset data register value - not accepted in POWER_DOWN mode
#define BH1750_DEFAULT_MTREG    69          // Default MTreg value
#define BH1750_ADDR_PIN_LOW   (0x23 << 1)   // ADDR-Pin low
#define BH1750_ADDR_PIN_HIGH  (0x5c << 1)   // ADDR-Pin high

typedef enum {      
    UNCONFIGURED               = 0x00,  // same as Power Down 
    CONTINUOUS_HIGH_RES_MODE   = 0x10,  // 1 lux / 120ms
    CONTINUOUS_HIGH_RES_MODE_2 = 0x11,  // 0.5 lux / 120ms
    CONTINUOUS_LOW_RES_MODE    = 0x13,  // 4 lux / 16ms
    ONE_TIME_HIGH_RES_MODE     = 0x20,  // 1 lux / 120ms
    ONE_TIME_HIGH_RES_MODE_2   = 0x21,  // 0.5 lux / 120ms
    ONE_TIME_LOW_RES_MODE      = 0x23   // 4 lux / 16ms
} BH1750_Mode;


BH1750_t  BH1750;


static bool write_config_value(uint8_t value)
{
    return (I2cWrite(&BH1750.I2c, BH1750_ADDR_PIN_LOW, value) == LMN_STATUS_OK);
}


void BH1750Init(void)
{
    // Send MTreg and the current mode to the sensor
    // High bit: 01000_MT[7,6,5]
    // Low bit:  011_MT[4,3,2,1,0]    
#if 0
    if (write_config_value(ONE_TIME_HIGH_RES_MODE)) {
        if (write_config_value((0b01000 << 3) | (BH1750_DEFAULT_MTREG >> 5))) {
            write_config_value((0b011 << 5) | (BH1750_DEFAULT_MTREG & 0b11111));
        }
    }
#endif
    
    if (write_config_value(ONE_TIME_HIGH_RES_MODE)) {
        if (write_config_value((0x08 << 3) | (BH1750_DEFAULT_MTREG >> 5))) {
            write_config_value((0x03 << 5) | (BH1750_DEFAULT_MTREG & 0x1f));
        }
    }
}

LmnStatus_t BH1750Read(uint16_t *value)
{
    uint8_t msb, lsb;
    if (I2cReadBuffer(&BH1750.I2c, BH1750_ADDR_PIN_LOW, &msb, 1) == LMN_STATUS_OK) {
        if (I2cReadBuffer(&BH1750.I2c, BH1750_ADDR_PIN_LOW, &lsb, 1) == LMN_STATUS_OK) {
            *value = (msb << 8 | lsb) / 1.2;
            return LMN_STATUS_OK;
        }
    }
    
    *value = 0;
    return LMN_STATUS_ERROR;
}

#endif // PERIPHERAL_BH1750
