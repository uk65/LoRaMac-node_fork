
#include <string.h>
#include "vl53l1_platform.h"
#include "i2c.h"
#include "delay.h"

#ifdef PERIPHERAL_VL53L1X

uint8_t buffer[256];

#ifdef PERIPHERAL_VL53L1X_LORAMAC_NODE_IMPL

#include "vl53l1x.h"
#include "utilities.h"


int8_t VL53L1_WriteMulti (uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count)
{
    buffer[0] = index >> 8;
    buffer[1] = index & 0xff;
    memcpy(&buffer[2], pdata, count);
    
    if (I2cWriteBuffer(&VL53L1X.I2c, dev, buffer, count + 2) == LMN_STATUS_OK)
        return 1;

    return 0;
}

int8_t VL53L1_ReadMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count)
{
    if (I2cReadMemBuffer(&VL53L1X.I2c, dev, index, pdata, (uint16_t) count) == LMN_STATUS_OK)
        return 1;

    return 0;
}

int8_t VL53L1_WrByte(uint16_t dev, uint16_t index, uint8_t data)
{
    buffer[0] = index >> 8;
    buffer[1] = index & 0xff;
    buffer[2] = data;

    if (I2cWriteBuffer(&VL53L1X.I2c, dev, buffer, 3) == LMN_STATUS_OK)
        return 1;

    return 0;
}

int8_t VL53L1_WrWord(uint16_t dev, uint16_t index, uint16_t data)
{
    buffer[0] = index >> 8;
    buffer[1] = index & 0xff;
    buffer[2] = data >> 8;
    buffer[3] = data & 0x00ff;

    if (I2cWriteBuffer(&VL53L1X.I2c, dev, buffer, 4) == LMN_STATUS_OK)
        return 1;

    return 0;
}

int8_t VL53L1_WrDWord(uint16_t dev, uint16_t index, uint32_t data)
{
    buffer[0] = index >> 8;
    buffer[1] = index & 0xff;
    buffer[2] = (data >> 24) & 0xff;
    buffer[3] = (data >> 16) & 0xff;
    buffer[4] = (data >> 8)  & 0xff;
    buffer[5] = (data >> 0 ) & 0xff;

    if (I2cWriteBuffer(&VL53L1X.I2c, dev, buffer, 6) == LMN_STATUS_OK)
        return 1;

    return 0;
}

int8_t VL53L1_RdByte(uint16_t dev, uint16_t index, uint8_t *data)
{
    if (I2cReadMemBuffer(&VL53L1X.I2c, dev, index, data, 1) == LMN_STATUS_OK)
        return 1;
    
    return 0;
}

int8_t VL53L1_RdWord(uint16_t dev, uint16_t index, uint16_t *data)
{
    if (I2cReadMemBuffer(&VL53L1X.I2c, dev, index, buffer, 2) == LMN_STATUS_OK) {
        *data = 
            ((uint16_t) buffer[0] << 8) +
             (uint16_t) buffer[1];
        return 1;
    }

    return 0;
}

int8_t VL53L1_RdDWord(uint16_t dev, uint16_t index, uint32_t *data)
{
    if (I2cReadMemBuffer(&VL53L1X.I2c, dev, index, buffer, 4) == LMN_STATUS_OK) {
        *data = 
            ((uint32_t) buffer[0] << 24) + 
            ((uint32_t) buffer[1] << 16) + 
            ((uint32_t) buffer[2] <<  8) +
             (uint32_t) buffer[3];
        return 1;
    }

    return 0;
}

int8_t VL53L1_WaitMs(uint16_t dev, int32_t wait_ms)
{
    DelayMs(wait_ms);
    return 0;
}

#else // !PERIPHERAL_VL53L1X_LORAMAC_NODE_IMPL

int8_t VL53L1_WriteMulti (uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count)
{
    buffer[0] = index >> 8;
    buffer[1] = index & 0xff;
    memcpy(&buffer[2], pdata, count);
    if (i2c_write(dev, buffer, count + 2, 1) == Error)
        return 1;
    return 0;
}

int8_t VL53L1_ReadMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count)
{
    buffer[0] = index >> 8;
    buffer[1] = index & 0xff;
    if (i2c_transact(dev, buffer, 2, pdata, count) == Error)
        return 1;
    return 0;
}

int8_t VL53L1_WrByte(uint16_t dev, uint16_t index, uint8_t data)
{
    buffer[0] = index >> 8;
    buffer[1] = index & 0xff;
    buffer[2] = data;
    if (i2c_write(dev, buffer, 3, 1) == Error)
        return 1;
    return 0;
}

int8_t VL53L1_WrWord(uint16_t dev, uint16_t index, uint16_t data)
{
    buffer[0] = index >> 8;
    buffer[1] = index & 0xff;
    buffer[2] = data >> 8;
    buffer[3] = data & 0x00ff;
    if (i2c_write(dev, buffer, 4, 1) == Error)
        return 1;
    return 0;
}

int8_t VL53L1_WrDWord(uint16_t dev, uint16_t index, uint32_t data)
{
    buffer[0] = index >> 8;
    buffer[1] = index & 0xff;
    buffer[2] = (data >> 24) & 0xff;
    buffer[3] = (data >> 16) & 0xff;
    buffer[4] = (data >> 8)  & 0xff;
    buffer[5] = (data >> 0 ) & 0xff;
    if (i2c_write(dev, buffer, 6, 1) == Error)
        return 1;
    return 0;
}

int8_t VL53L1_RdByte(uint16_t dev, uint16_t index, uint8_t *data)
{
    buffer[0] = index >> 8;
    buffer[1] = index & 0xff;
    if (i2c_transact(dev, buffer, 2, data, 1) == Error)
        return 1;
    return 0;
}

int8_t VL53L1_RdWord(uint16_t dev, uint16_t index, uint16_t *data)
{
    buffer[0] = index >> 8;
    buffer[1] = index & 0xff;
    uint8_t rec_buffer[2];

    if (i2c_transact(dev, buffer, 2, rec_buffer, 2) == Error)
        return 1;

    *data = ((uint16_t) rec_buffer[0] << 8) + (uint16_t) rec_buffer[1];
    return 0;
}

int8_t VL53L1_RdDWord(uint16_t dev, uint16_t index, uint32_t *data)
{
    buffer[0] = index >> 8;
    buffer[1] = index & 0xff;
    uint8_t rec_buffer[4];

    if (i2c_transact(dev, buffer, 2, rec_buffer, 4) == Error)
        return 1;

    *data = ((uint32_t) rec_buffer[0] << 24) + ((uint32_t) rec_buffer[1] << 16) + ((uint32_t) rec_buffer[2]<<8) + (uint32_t) rec_buffer[3];
    return 0;
}

int8_t VL53L1_WaitMs(uint16_t dev, int32_t wait_ms)
{
    delay_ms(wait_ms);
    return 0;
}

#endif // PERIPHERAL_VL53L1X_LORAMAC_NODE_IMPL

#endif // PERIPHERAL_VL53L1X