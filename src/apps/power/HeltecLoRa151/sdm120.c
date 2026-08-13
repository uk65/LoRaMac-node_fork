#include "sdm120.h"

#if defined( POWER_ROLE_SEND )

#include <math.h>
#include <string.h>

#include "board.h"
#include "delay.h"
#include "gpio.h"
#include "uart.h"

#define SDM120_UART_BAUDRATE                        2400
#define SDM120_SLAVE_ADDRESS                        1
#define SDM120_FUNCTION_READ_INPUT_REGISTERS        4
#define SDM120_FIRST_REGISTER                       0x0000
#define SDM120_REGISTER_COUNT                       14
#define SDM120_RESPONSE_SIZE                        61
#define SDM120_RESPONSE_TIMEOUT_IN_MS               500
#define SDM120_TX_DRAIN_TIME_IN_MS                  40
#define SDM120_RS485_DE_PIN                         PA_0

extern Uart_t Uart2;

static Gpio_t rs485_direction;

static uint16_t modbus_crc16( const uint8_t* data, uint16_t size )
{
    uint16_t crc = 0xFFFF;

    for( uint16_t i = 0; i < size; i++ )
    {
        crc ^= data[i];
        for( uint8_t bit = 0; bit < 8; bit++ )
        {
            crc = ( crc & 1 ) != 0 ? ( crc >> 1 ) ^ 0xA001 : crc >> 1;
        }
    }
    return crc;
}

static float decode_float_be( const uint8_t* data )
{
    uint32_t bits = ( ( uint32_t ) data[0] << 24 ) | ( ( uint32_t ) data[1] << 16 ) |
                    ( ( uint32_t ) data[2] << 8 ) | data[3];
    float value;

    memcpy( &value, &bits, sizeof( value ) );
    return value;
}

static void discard_uart_input( void )
{
    uint8_t byte;
    while( UartGetChar( &Uart2, &byte ) == 0 )
    {
    }
}

void sdm120_init( void )
{
    UartConfig( &Uart2, RX_TX, SDM120_UART_BAUDRATE, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
    GpioInit( &rs485_direction, SDM120_RS485_DE_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    discard_uart_input( );
}

bool sdm120_read_measurement( sdm120_measurement_t* measurement )
{
    uint8_t request[8] = {
        SDM120_SLAVE_ADDRESS,
        SDM120_FUNCTION_READ_INPUT_REGISTERS,
        ( uint8_t ) ( SDM120_FIRST_REGISTER >> 8 ),
        ( uint8_t ) SDM120_FIRST_REGISTER,
        ( uint8_t ) ( SDM120_REGISTER_COUNT >> 8 ),
        ( uint8_t ) SDM120_REGISTER_COUNT,
        0,
        0,
    };
    uint8_t response[SDM120_RESPONSE_SIZE];
    uint16_t received = 0;
    uint16_t crc      = modbus_crc16( request, 6 );

    if( measurement == NULL )
    {
        return false;
    }

    request[6] = ( uint8_t ) crc;
    request[7] = ( uint8_t ) ( crc >> 8 );
    discard_uart_input( );

    GpioWrite( &rs485_direction, 1 );
    if( UartPutBuffer( &Uart2, request, sizeof( request ) ) != 0 )
    {
        GpioWrite( &rs485_direction, 0 );
        return false;
    }
    // At 2400 baud an 8-byte 8N1 request takes about 33.4 ms.
    DelayMs( SDM120_TX_DRAIN_TIME_IN_MS );
    GpioWrite( &rs485_direction, 0 );

    for( uint16_t elapsed = 0; elapsed < SDM120_RESPONSE_TIMEOUT_IN_MS && received < sizeof( response ); elapsed++ )
    {
        while( received < sizeof( response ) && UartGetChar( &Uart2, &response[received] ) == 0 )
        {
            received++;
        }
        if( received < sizeof( response ) )
        {
            DelayMs( 1 );
        }
    }

    if( received != sizeof( response ) || response[0] != SDM120_SLAVE_ADDRESS ||
        response[1] != SDM120_FUNCTION_READ_INPUT_REGISTERS || response[2] != SDM120_REGISTER_COUNT * 2 )
    {
        return false;
    }

    crc = modbus_crc16( response, sizeof( response ) - 2 );
    if( response[sizeof( response ) - 2] != ( uint8_t ) crc ||
        response[sizeof( response ) - 1] != ( uint8_t ) ( crc >> 8 ) )
    {
        return false;
    }

    float voltage_v = decode_float_be( &response[3] );
    float current_a = decode_float_be( &response[3 + 6 * 2] );
    float power_w   = decode_float_be( &response[3 + 12 * 2] );

    if( isfinite( voltage_v ) == 0 || isfinite( current_a ) == 0 || isfinite( power_w ) == 0 || voltage_v < 0.0f ||
        voltage_v > 300.0f || current_a < 0.0f || current_a > 100.0f || power_w < -10000.0f || power_w > 10000.0f )
    {
        return false;
    }

    measurement->voltage_dv = ( uint16_t ) lroundf( voltage_v * 10.0f );
    measurement->current_ma = ( uint16_t ) lroundf( current_a * 1000.0f );
    measurement->power_w    = ( int32_t ) lroundf( power_w );
    return true;
}

#else

void sdm120_init( void )
{
}

bool sdm120_read_measurement( sdm120_measurement_t* measurement )
{
    ( void ) measurement;
    return false;
}

#endif
