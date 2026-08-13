#include "power_radio.h"

#include <string.h>

#include "loramac_radio.h"
#include "power_protocol.h"

#if defined( REGION_AS923 )
#define RF_FREQ_IN_HZ 923000000
#elif defined( REGION_AU915 ) || defined( REGION_US915 )
#define RF_FREQ_IN_HZ 915000000
#elif defined( REGION_CN470 )
#define RF_FREQ_IN_HZ 470000000
#elif defined( REGION_CN779 )
#define RF_FREQ_IN_HZ 779000000
#elif defined( REGION_EU433 )
#define RF_FREQ_IN_HZ 433000000
#elif defined( REGION_EU868 )
#define RF_FREQ_IN_HZ 868000000
#elif defined( REGION_KR920 )
#define RF_FREQ_IN_HZ 920000000
#elif defined( REGION_IN865 )
#define RF_FREQ_IN_HZ 865000000
#elif defined( REGION_RU864 )
#define RF_FREQ_IN_HZ 864000000
#else
#error "Please select a region under compiler options."
#endif

static volatile power_radio_event_type_t pending_event = POWER_RADIO_EVENT_NONE;
static power_radio_event_t event_data;
static loramac_radio_irq_t radio_irq_callbacks;

static void irq_tx_done( void )
{
    loramac_radio_set_sleep( );
    pending_event = POWER_RADIO_EVENT_TX_DONE;
}

static void irq_rx_done( loramac_radio_irq_rx_done_params_t* params )
{
    loramac_radio_set_sleep( );
    if( params->size_in_bytes <= sizeof( event_data.buffer ) )
    {
        memcpy( event_data.buffer, params->buffer, params->size_in_bytes );
        event_data.size        = params->size_in_bytes;
        event_data.rssi_in_dbm = params->rssi_in_dbm;
        event_data.snr_in_db   = params->snr_in_db;
        pending_event          = POWER_RADIO_EVENT_RX_DONE;
    }
    else
    {
        pending_event = POWER_RADIO_EVENT_RX_ERROR;
    }
}

static void irq_rx_error( void )
{
    loramac_radio_set_sleep( );
    pending_event = POWER_RADIO_EVENT_RX_ERROR;
}

static void irq_tx_timeout( void )
{
    loramac_radio_set_sleep( );
    pending_event = POWER_RADIO_EVENT_TX_TIMEOUT;
}

static void irq_rx_timeout( void )
{
    loramac_radio_set_sleep( );
    pending_event = POWER_RADIO_EVENT_RX_ERROR;
}

void power_radio_init( void )
{
    radio_irq_callbacks.loramac_radio_irq_tx_done    = irq_tx_done;
    radio_irq_callbacks.loramac_radio_irq_rx_done    = irq_rx_done;
    radio_irq_callbacks.loramac_radio_irq_rx_error   = irq_rx_error;
    radio_irq_callbacks.loramac_radio_irq_tx_timeout = irq_tx_timeout;
    radio_irq_callbacks.loramac_radio_irq_rx_timeout = irq_rx_timeout;
    loramac_radio_init( &radio_irq_callbacks );

    loramac_radio_lora_cfg_params_t config = {
        .rf_freq_in_hz           = RF_FREQ_IN_HZ,
        .tx_rf_pwr_in_dbm        = 14,
        .sf                      = RAL_LORA_SF7,
        .bw                      = RAL_LORA_BW_125_KHZ,
        .cr                      = RAL_LORA_CR_4_5,
        .preamble_len_in_symb    = 8,
        .is_pkt_len_fixed        = false,
        .pld_len_in_bytes        = POWER_PACKET_SIZE,
        .is_crc_on               = true,
        .invert_iq_is_on         = false,
        .rx_sync_timeout_in_symb = 6,
        .is_rx_continuous        = true,
        .tx_timeout_in_ms        = 4000,
    };
    loramac_radio_lora_set_cfg( &config );
}

void power_radio_process( void )
{
    loramac_radio_irq_process( );
}

void power_radio_transmit( uint8_t* buffer, uint16_t size )
{
    loramac_radio_transmit( buffer, size );
}

void power_radio_receive( void )
{
    loramac_radio_set_rx( RAL_RX_TIMEOUT_CONTINUOUS_MODE );
}

bool power_radio_take_event( power_radio_event_t* event )
{
    if( event == NULL || pending_event == POWER_RADIO_EVENT_NONE )
    {
        return false;
    }

    *event        = event_data;
    event->type   = pending_event;
    pending_event = POWER_RADIO_EVENT_NONE;
    return true;
}
