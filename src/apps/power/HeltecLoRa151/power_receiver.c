#include "power_app.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "power_protocol.h"
#include "power_radio.h"
#include "uart.h"

extern Uart_t Uart2;

static bool replay_state_valid;
static uint32_t last_boot_id;
static uint32_t last_sequence;

void power_app_init( void )
{
    UartConfig( &Uart2, RX_TX, 115200, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
    power_radio_receive( );
    printf( "{\"event\":\"start\",\"role\":\"receive\"}\n" );
}

void power_app_process( void )
{
    power_radio_event_t event;
    power_measurement_t measurement;

    if( power_radio_take_event( &event ) == false )
    {
        return;
    }

    if( event.type == POWER_RADIO_EVENT_RX_DONE )
    {
        if( power_protocol_decode( event.buffer, event.size, &measurement ) == false )
        {
            printf( "{\"event\":\"drop\",\"reason\":\"invalid_auth_or_format\"}\n" );
        }
        else if( replay_state_valid && measurement.boot_id == last_boot_id && measurement.sequence <= last_sequence )
        {
            printf( "{\"event\":\"drop\",\"reason\":\"replay\",\"seq\":%lu}\n",
                    ( unsigned long ) measurement.sequence );
        }
        else
        {
            replay_state_valid = true;
            last_boot_id       = measurement.boot_id;
            last_sequence      = measurement.sequence;
            printf( "{\"device\":%u,\"boot_id\":%lu,\"seq\":%lu,\"modbus_ok\":%s,\"power_w\":%ld,"
                    "\"voltage_v\":%u.%u,\"current_ma\":%u,\"rssi\":%d,\"snr\":%d}\n",
                    POWER_DEVICE_ID, ( unsigned long ) measurement.boot_id, ( unsigned long ) measurement.sequence,
                    measurement.is_valid ? "true" : "false", ( long ) measurement.power_w,
                    measurement.voltage_dv / 10, measurement.voltage_dv % 10, measurement.current_ma,
                    event.rssi_in_dbm, event.snr_in_db );
        }
    }

    if( event.type == POWER_RADIO_EVENT_RX_DONE || event.type == POWER_RADIO_EVENT_RX_ERROR )
    {
        power_radio_receive( );
    }
}
