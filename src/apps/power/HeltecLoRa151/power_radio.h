#ifndef POWER_RADIO_H
#define POWER_RADIO_H

#include <stdbool.h>
#include <stdint.h>

typedef enum power_radio_event_type_e
{
    POWER_RADIO_EVENT_NONE,
    POWER_RADIO_EVENT_TX_DONE,
    POWER_RADIO_EVENT_TX_TIMEOUT,
    POWER_RADIO_EVENT_RX_DONE,
    POWER_RADIO_EVENT_RX_ERROR,
} power_radio_event_type_t;

typedef struct power_radio_event_s
{
    power_radio_event_type_t type;
    uint8_t                  buffer[255];
    uint16_t                 size;
    int16_t                  rssi_in_dbm;
    int8_t                   snr_in_db;
} power_radio_event_t;

void power_radio_init( void );
void power_radio_process( void );
void power_radio_transmit( uint8_t* buffer, uint16_t size );
void power_radio_receive( void );
bool power_radio_take_event( power_radio_event_t* event );

#endif
