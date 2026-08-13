#include "power_app.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "board.h"
#include "power_protocol.h"
#include "power_radio.h"
#include "sdm120.h"
#include "timer.h"

#define ACTIVE_INTERVAL_MS                          5000
#define IDLE_INTERVAL_MS                            60000
#define NIGHT_INTERVAL_MS                           900000
#define IDLE_AFTER_ZERO_MS                          300000
#define NIGHT_AFTER_ZERO_MS                         1800000
#define ZERO_THRESHOLD_W                            1

static TimerEvent_t tx_timer;
static volatile bool tx_pending;
static uint32_t sequence;
static uint32_t boot_id;
static uint32_t next_interval_ms = ACTIVE_INTERVAL_MS;
static uint32_t zero_duration_ms;
static bool zero_period_active;

static void on_tx_timer( void* context )
{
    ( void ) context;
    TimerStop( &tx_timer );
    tx_pending = true;
}

static void update_interval( int32_t power_w )
{
    if( power_w > ZERO_THRESHOLD_W )
    {
        zero_period_active = false;
        zero_duration_ms   = 0;
        next_interval_ms   = ACTIVE_INTERVAL_MS;
        return;
    }

    if( zero_period_active == false )
    {
        zero_period_active = true;
        zero_duration_ms   = 0;
    }
    else if( UINT32_MAX - zero_duration_ms >= next_interval_ms )
    {
        zero_duration_ms += next_interval_ms;
    }
    else
    {
        zero_duration_ms = UINT32_MAX;
    }

    if( zero_duration_ms >= NIGHT_AFTER_ZERO_MS )
    {
        next_interval_ms = NIGHT_INTERVAL_MS;
    }
    else if( zero_duration_ms >= IDLE_AFTER_ZERO_MS )
    {
        next_interval_ms = IDLE_INTERVAL_MS;
    }
    else
    {
        next_interval_ms = ACTIVE_INTERVAL_MS;
    }
}

static void send_measurement( void )
{
    power_measurement_t measurement;
    sdm120_measurement_t meter;
    uint8_t packet[POWER_PACKET_SIZE];

    memset( &measurement, 0, sizeof( measurement ) );
    measurement.boot_id  = boot_id;
    measurement.sequence = sequence++;
    measurement.is_valid = sdm120_read_measurement( &meter );
    if( measurement.is_valid )
    {
        measurement.power_w    = meter.power_w;
        measurement.voltage_dv = meter.voltage_dv;
        measurement.current_ma = meter.current_ma;
        update_interval( measurement.power_w );
    }

    power_protocol_encode( &measurement, packet );
    power_radio_transmit( packet, sizeof( packet ) );
}

void power_app_init( void )
{
    sdm120_init( );
    boot_id = BoardGetRandomSeed( ) ^ TimerGetCurrentTime( );
    TimerInit( &tx_timer, on_tx_timer );
    TimerSetValue( &tx_timer, ACTIVE_INTERVAL_MS );
    tx_pending = true;
}

void power_app_process( void )
{
    power_radio_event_t event;

    if( tx_pending )
    {
        tx_pending = false;
        send_measurement( );
    }

    if( power_radio_take_event( &event ) &&
        ( event.type == POWER_RADIO_EVENT_TX_DONE || event.type == POWER_RADIO_EVENT_TX_TIMEOUT ) )
    {
        TimerSetValue( &tx_timer, next_interval_ms );
        TimerStart( &tx_timer );
    }
}
