#ifndef POWER_SDM120_H
#define POWER_SDM120_H

#include <stdbool.h>
#include <stdint.h>

typedef struct sdm120_measurement_s
{
    int32_t  power_w;
    uint16_t voltage_dv;
    uint16_t current_ma;
} sdm120_measurement_t;

void sdm120_init( void );
bool sdm120_read_measurement( sdm120_measurement_t* measurement );

#endif
