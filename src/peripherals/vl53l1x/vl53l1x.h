
#ifndef _VL53L1X_H_
#define _VL53L1X_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "i2c.h"
#include "utilities.h"

typedef enum {
    DistanceShort = 1,      // up to 1,3 meter
    DistanceLong  = 2,      // up to 4 meter
} DistanceMode_t;

typedef enum {
    ActiveLow  = 0,
    ActiveHigh = 1,
} InterruptPolarity_t;

typedef struct VL53L1X_s {
    I2c_t                   I2c;
    DistanceMode_t          DistanceMode;
    uint32_t                InterMeasurementMs;
    InterruptPolarity_t     InterruptPolarity;
} VL53L1X_t;

LmnStatus_t VL53L1XIStartRanging(void);

LmnStatus_t VL53L1XIStopRanging(void);

LmnStatus_t VL53L1XInit(void);

LmnStatus_t VL53L1XGetDistance(uint16_t *distance);


extern VL53L1X_t  VL53L1X;

#ifdef __cplusplus
}
#endif

#endif // _VL53L1X_H_