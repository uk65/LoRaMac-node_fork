#include "power_protocol.h"

#include <string.h>

#include "cmac.h"

#define POWER_PROTOCOL_VERSION                      1
#define POWER_MESSAGE_MEASUREMENT                   1
#define POWER_FLAG_AUTHENTICATED                    0x01
#define POWER_FLAG_ENCRYPTED                        0x02
#define POWER_FLAG_MEASUREMENT_VALID                0x10
#define POWER_PACKET_AUTHENTICATED_SIZE             20
#define POWER_TAG_SIZE                              8

/* Replace this development key on both devices before deployment. */
static const uint8_t power_auth_key[AES_CMAC_KEY_LENGTH] = {
    0x4D, 0xA1, 0x73, 0x92, 0x26, 0xC8, 0x0F, 0x55,
    0xB4, 0x31, 0xE7, 0x68, 0x9A, 0xDC, 0x02, 0xBE,
};

static void write_u16_be( uint8_t* output, uint16_t value )
{
    output[0] = ( uint8_t ) ( value >> 8 );
    output[1] = ( uint8_t ) value;
}

static void write_u32_be( uint8_t* output, uint32_t value )
{
    output[0] = ( uint8_t ) ( value >> 24 );
    output[1] = ( uint8_t ) ( value >> 16 );
    output[2] = ( uint8_t ) ( value >> 8 );
    output[3] = ( uint8_t ) value;
}

static uint16_t read_u16_be( const uint8_t* input )
{
    return ( ( uint16_t ) input[0] << 8 ) | input[1];
}

static uint32_t read_u32_be( const uint8_t* input )
{
    return ( ( uint32_t ) input[0] << 24 ) | ( ( uint32_t ) input[1] << 16 ) |
           ( ( uint32_t ) input[2] << 8 ) | input[3];
}

static void compute_auth_tag( const uint8_t* data, uint16_t size, uint8_t tag[POWER_TAG_SIZE] )
{
    AES_CMAC_CTX context;
    uint8_t      digest[AES_CMAC_DIGEST_LENGTH];

    AES_CMAC_Init( &context );
    AES_CMAC_SetKey( &context, power_auth_key );
    AES_CMAC_Update( &context, data, size );
    AES_CMAC_Final( digest, &context );
    memcpy( tag, digest, POWER_TAG_SIZE );
}

static bool tags_equal( const uint8_t* left, const uint8_t* right )
{
    uint8_t difference = 0;
    for( uint8_t i = 0; i < POWER_TAG_SIZE; i++ )
    {
        difference |= left[i] ^ right[i];
    }
    return difference == 0;
}

void power_protocol_encode( const power_measurement_t* measurement, uint8_t packet[POWER_PACKET_SIZE] )
{
    packet[0] = POWER_PROTOCOL_VERSION;
    packet[1] = POWER_MESSAGE_MEASUREMENT;
    packet[2] = POWER_FLAG_AUTHENTICATED | ( measurement->is_valid ? POWER_FLAG_MEASUREMENT_VALID : 0 );
    packet[3] = POWER_DEVICE_ID;
    write_u32_be( &packet[4], measurement->boot_id );
    write_u32_be( &packet[8], measurement->sequence );
    write_u32_be( &packet[12], ( uint32_t ) measurement->power_w );
    write_u16_be( &packet[16], measurement->voltage_dv );
    write_u16_be( &packet[18], measurement->current_ma );
    compute_auth_tag( packet, POWER_PACKET_AUTHENTICATED_SIZE, &packet[20] );
}

bool power_protocol_decode( const uint8_t* packet, uint16_t size, power_measurement_t* measurement )
{
    uint8_t expected_tag[POWER_TAG_SIZE];

    if( packet == NULL || measurement == NULL || size != POWER_PACKET_SIZE || packet[0] != POWER_PROTOCOL_VERSION ||
        packet[1] != POWER_MESSAGE_MEASUREMENT || packet[3] != POWER_DEVICE_ID ||
        ( packet[2] & POWER_FLAG_AUTHENTICATED ) == 0 || ( packet[2] & POWER_FLAG_ENCRYPTED ) != 0 )
    {
        return false;
    }

    compute_auth_tag( packet, POWER_PACKET_AUTHENTICATED_SIZE, expected_tag );
    if( tags_equal( expected_tag, &packet[20] ) == false )
    {
        return false;
    }

    measurement->boot_id    = read_u32_be( &packet[4] );
    measurement->is_valid   = ( packet[2] & POWER_FLAG_MEASUREMENT_VALID ) != 0;
    measurement->sequence   = read_u32_be( &packet[8] );
    measurement->power_w    = ( int32_t ) read_u32_be( &packet[12] );
    measurement->voltage_dv = read_u16_be( &packet[16] );
    measurement->current_ma = read_u16_be( &packet[18] );
    return true;
}
