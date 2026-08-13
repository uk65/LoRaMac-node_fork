#ifndef POWER_PROTOCOL_H
#define POWER_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>

#define POWER_DEVICE_ID                             1
#define POWER_PACKET_SIZE                           28

typedef struct power_measurement_s
{
    bool     is_valid;
    uint32_t boot_id;
    uint32_t sequence;
    int32_t  power_w;
    uint16_t voltage_dv;
    uint16_t current_ma;
} power_measurement_t;

void power_protocol_encode( const power_measurement_t* measurement, uint8_t packet[POWER_PACKET_SIZE] );
bool power_protocol_decode( const uint8_t* packet, uint16_t size, power_measurement_t* measurement );

#endif
