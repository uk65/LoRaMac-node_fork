
#include <stdio.h>
#include <string.h>

#include "rf24_defines.h"
#include "rf24.h"

#define RF24_POWERUP_DELAY 5000

#include "nrf24l01.h"
#include "delay.h"
#include "systick.h"

#ifdef PERIPHERAL_NRF24L01

void RF24::ce(bool enable)
{
    GpioWrite(&NRF24L01.CE, enable);
}

void RF24::delay_ms(uint32_t ms)
{
    DelayMs(ms);
}

uint32_t RF24::millis(void)
{
    return systick();
}

uint32_t RF24::systick(void)
{
    return SysTick();
}

/****************************************************************************/

void RF24::toggle_features(void)
{
    GpioWrite(&NRF24L01.Spi.Nss, 0);
    DelayMs(m_csDelay);
    
    m_status = (uint8_t) SpiInOut(&NRF24L01.Spi, ACTIVATE);
    SpiInOut(&NRF24L01.Spi, 0x73);
    
    GpioWrite(&NRF24L01.Spi.Nss, 1);
}

/****************************************************************************/

void RF24::read_register(uint8_t reg, uint8_t *buf, uint8_t len)
{
    GpioWrite(&NRF24L01.Spi.Nss, 0);
    DelayMs(m_csDelay);
    
    m_status = (uint8_t) SpiInOut(&NRF24L01.Spi, R_REGISTER | reg);
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = SpiInOut(&NRF24L01.Spi, 0x00);
    }

    GpioWrite(&NRF24L01.Spi.Nss, 1);
}

/****************************************************************************/

uint8_t RF24::read_register(uint8_t reg)
{
    GpioWrite(&NRF24L01.Spi.Nss, 0);
    DelayMs(m_csDelay);
    
    m_status = (uint8_t) SpiInOut(&NRF24L01.Spi, R_REGISTER | reg);
    uint8_t result = SpiInOut(&NRF24L01.Spi, 0x00);

    GpioWrite(&NRF24L01.Spi.Nss, 1);
    
    return result;
}

/****************************************************************************/

void RF24::write_register(uint8_t reg, const uint8_t *buf, uint8_t len)
{
    GpioWrite(&NRF24L01.Spi.Nss, 0);
    DelayMs(m_csDelay);
    
    m_status = (uint8_t) SpiInOut(&NRF24L01.Spi, W_REGISTER | reg);
    for (uint8_t i = 0; i < len; i++) {
         SpiInOut(&NRF24L01.Spi, buf[i]);
    }

    GpioWrite(&NRF24L01.Spi.Nss, 1);
}

/****************************************************************************/

void RF24::write_register(uint8_t reg, uint8_t value, bool is_cmd_only)
{
    GpioWrite(&NRF24L01.Spi.Nss, 0);
    DelayMs(m_csDelay);
    
    m_status = (uint8_t) SpiInOut(&NRF24L01.Spi, W_REGISTER | reg);
    if (!is_cmd_only) {
        SpiInOut(&NRF24L01.Spi, value);
    }

    GpioWrite(&NRF24L01.Spi.Nss, 1);
}

/****************************************************************************/

void RF24::write_payload(const void *buf, uint8_t data_len, const uint8_t writeType)
{
    const uint8_t *current = reinterpret_cast<const uint8_t *>(buf);

    uint8_t blank_len = !data_len ? 1 : 0;
    
    if (!m_dynamic_payloads_enabled) {
        data_len = rf24_min(data_len, m_payload_size);
        blank_len = static_cast<uint8_t>(m_payload_size - data_len);
    }
    else {
        data_len = rf24_min(data_len, static_cast<uint8_t>(32));
    }


    GpioWrite(&NRF24L01.Spi.Nss, 0);
    DelayMs(m_csDelay);

    m_status = (uint8_t) SpiInOut(&NRF24L01.Spi, writeType);
    
    for (uint8_t i = 0; i < data_len; i++) {
         SpiInOut(&NRF24L01.Spi, current[i]);
    }
    for (uint8_t i = 0; i < blank_len; i++) {
         SpiInOut(&NRF24L01.Spi, 0x00);
    }
    
    GpioWrite(&NRF24L01.Spi.Nss, 1);
}

/****************************************************************************/

void RF24::read_payload(void *buf, uint8_t data_len)
{
    uint8_t *current = reinterpret_cast<uint8_t *>(buf);

    uint8_t blank_len = 0;

    if (!m_dynamic_payloads_enabled) {
        data_len = rf24_min(data_len, m_payload_size);
        blank_len = static_cast<uint8_t>(m_payload_size - data_len);
    } else {
        data_len = rf24_min(data_len, static_cast<uint8_t>(32));
    }

    GpioWrite(&NRF24L01.Spi.Nss, 0);
    DelayMs(m_csDelay);

    m_status = (uint8_t) SpiInOut(&NRF24L01.Spi, R_RX_PAYLOAD);

    for (uint8_t i = 0; i < data_len; i++) {
         current[i] = SpiInOut(&NRF24L01.Spi, 0x00);
    }
    for (uint8_t i = 0; i < blank_len; i++) {
         SpiInOut(&NRF24L01.Spi, 0xff);
    }

    GpioWrite(&NRF24L01.Spi.Nss, 1);
}

/****************************************************************************/

uint8_t RF24::flush_rx(void)
{
    write_register(FLUSH_RX, RF24_NOP, true);
    return m_status;
}

/****************************************************************************/

uint8_t RF24::flush_tx(void)
{
    write_register(FLUSH_TX, RF24_NOP, true);
    return m_status;
}

/****************************************************************************/

uint8_t RF24::get_status(void)
{
    write_register(RF24_NOP, RF24_NOP, true);
    return m_status;
}

/****************************************************************************/

RF24::RF24()
    : m_payload_size(32)
    , m_is_p_variant(false)
    , m_is_p0_rx(false)
    , m_addr_width(5)
    , m_dynamic_payloads_enabled(true)
    , m_csDelay(5)
{
    m_pipe0_reading_address[0] = 0;
    init_radio();
}

/****************************************************************************/

void RF24::setChannel(uint8_t channel)
{
    const uint8_t max_channel = 125;
    write_register(RF_CH, rf24_min(channel, max_channel));
}

uint8_t RF24::getChannel()
{
    return read_register(RF_CH);
}

/****************************************************************************/

void RF24::setPayloadSize(uint8_t size)
{
    // payload size must be in range [1, 32]
    m_payload_size = static_cast<uint8_t>(rf24_max(1, rf24_min(32, size)));

    // write static payload size setting for all pipes
    for (uint8_t i = 0; i < 6; ++i) {
        write_register(static_cast<uint8_t>(RX_PW_P0 + i), m_payload_size);
    }
}

/****************************************************************************/

uint8_t RF24::getPayloadSize(void)
{
    return m_payload_size;
}

/****************************************************************************/

bool RF24::init_radio()
{
    // Must allow the radio time to settle else configuration bits will not necessarily stick.
    // This is actually only required following power up but some settling time also appears to
    // be required after resets too. For full coverage, we'll always assume the worst.
    // Enabling 16b CRC is by far the most obvious case if the wrong timing is used - or skipped.
    // Technically we require 4.5ms + 14us as a worst case. We'll just call it 5ms for good measure.
    // WARNING: Delay is based on P-variant whereby non-P *may* require different timing.
//    delay(5);

    // Set 1500uS (minimum for 32B payload in ESB@250KBPS) timeouts, to make testing a little easier
    // WARNING: If this is ever lowered, either 250KBS mode with AA is broken or maximum packet
    // sizes must never be used. See datasheet for a more complete explanation.
    setRetries(5, 15);

    // Then set the data rate to the slowest (and most reliable) speed supported by all
    // hardware. Since this value occupies the same register as the PA level value, set
    // the PA level to MAX
    setRadiation(RF24_PA_MAX, RF24_1MBPS); // LNA enabled by default

    // detect if is a plus variant & use old toggle features command accordingly
    uint8_t before_toggle = read_register(FEATURE);
    toggle_features();
    uint8_t after_toggle = read_register(FEATURE);
    m_is_p_variant = before_toggle == after_toggle;
    if (after_toggle) {
        if (m_is_p_variant) {
            // module did not experience power-on-reset (#401)
            toggle_features();
        }
        // allow use of multicast parameter and dynamic payloads by default
        write_register(FEATURE, 0);
    }
    m_ack_payloads_enabled = false; // ack payloads disabled by default
    write_register(DYNPD, 0);     // disable dynamic payloads by default (for all pipes)
    m_dynamic_payloads_enabled = false;
    write_register(EN_AA, 0x3F);  // enable auto-ack on all pipes
    write_register(EN_RXADDR, 3); // only open RX pipes 0 & 1
    setPayloadSize(32);           // set static payload size to 32 (max) bytes by default
    setAddressWidth(5);           // set default address length to (max) 5 bytes

    // Set up default configuration.  Callers can always change it later.
    // This channel should be universally safe and not bleed over into adjacent
    // spectrum.
    setChannel(76);

    // Reset current status
    // Notice reset and flush is the last thing we do
    write_register(NRF_STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));

    // Flush buffers
    flush_rx();
    flush_tx();

    // Clear CONFIG register:
    //      Reflect all IRQ events on IRQ pin
    //      Enable PTX
    //      Power Up
    //      16-bit CRC (CRC required by auto-ack)
    // Do not write CE high so radio will remain in standby I mode
    // PTX should use only 22uA of power
    write_register(NRF_CONFIG, ((1 << EN_CRC) | (1 << CRCO)));
    m_config_reg = read_register(NRF_CONFIG);

    powerUp();

    // if config is not set correctly then there was a bad response from module
    return m_config_reg == ((1 << EN_CRC) | (1 << CRCO) | (1 << PWR_UP)) ? true : false;
}

/****************************************************************************/

bool RF24::isChipConnected()
{
    return read_register(SETUP_AW) == (m_addr_width - static_cast<uint8_t>(2));
}

/****************************************************************************/

void RF24::startListening(void)
{
    powerUp();

    m_config_reg |= (1 << PRIM_RX);
    write_register(NRF_CONFIG, m_config_reg);
    write_register(NRF_STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));

    ce(true);

    // Restore the pipe0 address, if exists
    if (m_is_p0_rx) {
        write_register(RX_ADDR_P0, m_pipe0_reading_address, m_addr_width);
    }
    else {
        closeReadingPipe(0);
    }
}

/****************************************************************************/

static const uint8_t child_pipe_enable[] = {
    ERX_P0,
    ERX_P1,
    ERX_P2,
    ERX_P3,
    ERX_P4,
    ERX_P5
};

void RF24::stopListening(void)
{
    ce(false);

    delay_ms(1);
//###    delayMicroseconds(static_cast<int>(m_txDelay));
    if (m_ack_payloads_enabled) {
        flush_tx();
    }

    m_config_reg = static_cast<uint8_t>(m_config_reg & ~(1 << PRIM_RX));
    write_register(NRF_CONFIG, m_config_reg);

    #define pgm_read_byte(addr)     (*(const unsigned char*)(addr))
    write_register(EN_RXADDR, static_cast<uint8_t>(read_register(EN_RXADDR) | (1 << child_pipe_enable[0]))); // Enable RX on pipe0
}

/****************************************************************************/

void RF24::powerDown(void)
{
    ce(false);
    
    m_config_reg = static_cast<uint8_t>(m_config_reg & ~(1 << PWR_UP));
    write_register(NRF_CONFIG, m_config_reg);
}

/****************************************************************************/

//Power up now. Radio will not power down unless instructed by MCU for config changes etc.
void RF24::powerUp(void)
{
    // if not powered up then power up and wait for the radio to initialize
    if (!(m_config_reg & (1 << PWR_UP))) {
        m_config_reg |= (1 << PWR_UP);
        write_register(NRF_CONFIG, m_config_reg);

        // For nRF24L01+ to go from power down mode to TX or RX mode it must first pass through stand-by mode.
        // There must be a delay of Tpd2stby (see Table 16.) after the nRF24L01+ leaves power down mode before
        // the CEis set high. - Tpd2stby can be up to 5ms per the 1.0 datasheet
//        delayMicroseconds(RF24_POWERUP_DELAY);
    }
}

/******************************************************************/

//Similar to the previous write, clears the interrupt flags
bool RF24::write(const void* buf, uint8_t len, const bool multicast)
{
    //Start Writing
    startFastWrite(buf, len, multicast);

    while (!(get_status() & ((1 << TX_DS) | (1 << MAX_RT)))) {
    }

    ce(false);

    write_register(NRF_STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));

    //Max retries exceeded
    if (m_status & (1 << MAX_RT)) {
        flush_tx(); // Only going to be 1 packet in the FIFO at a time using this method, so just flush
        return 0;
    }
    //TX OK 1 or 0
    return 1;
}

bool RF24::write(const void* buf, uint8_t len)
{
    return write(buf, len, 0);
}

/****************************************************************************/

//For general use, the interrupt flags are not important to clear
bool RF24::writeBlocking(const void* buf, uint8_t len, uint32_t timeout)
{
    //Block until the FIFO is NOT full.
    //Keep track of the MAX retries and set auto-retry if seeing failures
    //This way the FIFO will fill up and allow blocking until packets go through
    //The radio will auto-clear everything in the FIFO as long as CE remains high

    uint32_t timer =  millis(); // Get the time that the payload transmission started

    while ((get_status() & ((1 << TX_FULL)))) { // Blocking only if FIFO is full. This will loop and block until TX is successful or timeout

        if (m_status & (1 << MAX_RT)) { // If MAX Retries have been reached
            reUseTX();              // Set re-transmit and clear the MAX_RT interrupt flag
            if (millis() - timer > timeout) {
                return 0; // If this payload has exceeded the user-defined timeout, exit and return 0
            }
        }
    }

    //Start Writing
    startFastWrite(buf, len, 0); // Write the payload if a buffer is clear

    return 1; // Return 1 to indicate successful transmission
}

/****************************************************************************/

void RF24::reUseTX()
{
    write_register(NRF_STATUS, (1 << MAX_RT)); //Clear max retry flag
    write_register(REUSE_TX_PL, RF24_NOP, true);
    
    ce(false);
    ce(true);
}

/****************************************************************************/

bool RF24::writeFast(const void* buf, uint8_t len, const bool multicast)
{
    //Block until the FIFO is NOT full.
    //Keep track of the MAX retries and set auto-retry if seeing failures
    //Return 0 so the user can control the retries and set a timer or failure counter if required
    //The radio will auto-clear everything in the FIFO as long as CE remains high

    //Blocking only if FIFO is full. This will loop and block until TX is successful or fail
    while ((get_status() & ((1 << TX_FULL)))) {
        if (m_status & (1 << MAX_RT)) {
            return 0; //Return 0. The previous payload has not been retransmitted
            // From the user perspective, if you get a 0, just keep trying to send the same payload
        }
    }
    startFastWrite(buf, len, multicast); // Start Writing

    return 1;
}

bool RF24::writeFast(const void* buf, uint8_t len)
{
    return writeFast(buf, len, 0);
}

/****************************************************************************/

//Per the documentation, we want to set PTX Mode when not listening. Then all we do is write data and set CE high
//In this mode, if we can keep the FIFO buffers loaded, packets will transmit immediately (no 130us delay)
//Otherwise we enter Standby-II mode, which is still faster than standby mode
//Also, we remove the need to keep writing the config register over and over and delaying for 150 us each time if sending a stream of data

void RF24::startFastWrite(const void* buf, uint8_t len, const bool multicast, bool startTx)
{ //TMRh20

    write_payload(buf, len, multicast ? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD);
    if (startTx) {
        ce(true);
    }
}

/****************************************************************************/

//Added the original startWrite back in so users can still use interrupts, ack payloads, etc
//Allows the library to pass all tests
bool RF24::startWrite(const void* buf, uint8_t len, const bool multicast)
{
    // Send the payload
    write_payload(buf, len, multicast ? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD);
    ce(true);
    ce(false);
    return !(m_status & (1 << TX_FULL));
}

/****************************************************************************/

bool RF24::rxFifoFull()
{
    return read_register(FIFO_STATUS) & (1 << RX_FULL);
}

/****************************************************************************/

uint8_t RF24::isFifo(bool about_tx)
{
    return static_cast<uint8_t>((read_register(FIFO_STATUS) >> (4 * about_tx)) & 3);
}

/****************************************************************************/

bool RF24::isFifo(bool about_tx, bool check_empty)
{
    return static_cast<bool>(isFifo(about_tx) & (1 << !check_empty));
}

/****************************************************************************/

bool RF24::txStandBy()
{
    while (!(read_register(FIFO_STATUS) & (1 << TX_EMPTY))) {
        if (m_status & (1 << MAX_RT)) {
            write_register(NRF_STATUS, (1 << MAX_RT));
            ce(false);
            flush_tx(); //Non blocking, flush the data
            return 0;
        }
    }

    ce(false);
    return 1;
}

/****************************************************************************/

bool RF24::txStandBy(uint32_t timeout, bool startTx)
{

    if (startTx) {
        stopListening();
        ce(true);
    }

    uint32_t start = systick();

    while (!(read_register(FIFO_STATUS) & (1 << TX_EMPTY))) {
        if (m_status & (1 << MAX_RT)) {
            write_register(NRF_STATUS, (1 << MAX_RT));
            ce(false);
            ce(true);

            if (systick() - start >= timeout) {
                ce(false);
                flush_tx();
                return 0;
            }
        }
    }

    ce(false);
    
    return 1;
}

/****************************************************************************/

void RF24::maskIRQ(bool tx, bool fail, bool rx)
{
    /* clear the interrupt flags */
    m_config_reg = static_cast<uint8_t>(m_config_reg & ~(1 << MASK_MAX_RT | 1 << MASK_TX_DS | 1 << MASK_RX_DR));
    /* set the specified interrupt flags */
    m_config_reg = static_cast<uint8_t>(m_config_reg | fail << MASK_MAX_RT | tx << MASK_TX_DS | rx << MASK_RX_DR);
    write_register(NRF_CONFIG, m_config_reg);
}

/****************************************************************************/

uint8_t RF24::getDynamicPayloadSize(void)
{
    uint8_t result = read_register(R_RX_PL_WID);

    if (result > 32) {
        flush_rx();
        delay_ms(2);
        return 0;
    }
    return result;
}

/****************************************************************************/

bool RF24::available(void)
{
    return available(NULL);
}

/****************************************************************************/

bool RF24::available(uint8_t* pipe_num)
{
    // get implied RX FIFO empty flag from status byte
    uint8_t pipe = (get_status() >> RX_P_NO) & 0x07;
    if (pipe > 5)
        return 0;

    // If the caller wants the pipe number, include that
    if (pipe_num)
        *pipe_num = pipe;

    return 1;
}

/****************************************************************************/

void RF24::read(void* buf, uint8_t len)
{
    // Fetch the payload
    read_payload(buf, len);

    //Clear the only applicable interrupt flags
    write_register(NRF_STATUS, (1 << RX_DR));
}

/****************************************************************************/

void RF24::whatHappened(bool& tx_ok, bool& tx_fail, bool& rx_ready)
{
    // Read the status & reset the status in one easy call
    // Or is that such a good idea?
    write_register(NRF_STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));

    // Report to the user what happened
    tx_ok = m_status & (1 << TX_DS);
    tx_fail = m_status & (1 << MAX_RT);
    rx_ready = m_status & (1 << RX_DR);
}

/****************************************************************************/

void RF24::openWritingPipe(uint64_t value)
{
    // Note that AVR 8-bit uC's store this LSB first, and the NRF24L01(+)
    // expects it LSB first too, so we're good.

    write_register(RX_ADDR_P0, reinterpret_cast<uint8_t*>(&value), m_addr_width);
    write_register(TX_ADDR, reinterpret_cast<uint8_t*>(&value), m_addr_width);
}

/****************************************************************************/

void RF24::openWritingPipe(const uint8_t* address)
{
    // Note that AVR 8-bit uC's store this LSB first, and the NRF24L01(+)
    // expects it LSB first too, so we're good.
    write_register(RX_ADDR_P0, address, m_addr_width);
    write_register(TX_ADDR, address, m_addr_width);
}

/****************************************************************************/

static const uint8_t child_pipe[] = {
    RX_ADDR_P0,
    RX_ADDR_P1,
    RX_ADDR_P2,
    RX_ADDR_P3,
    RX_ADDR_P4,
    RX_ADDR_P5
};

void RF24::openReadingPipe(uint8_t child, uint64_t address)
{
    // If this is pipe 0, cache the address.  This is needed because
    // openWritingPipe() will overwrite the pipe 0 address, so
    // startListening() will have to restore it.
    if (child == 0) {
        memcpy(m_pipe0_reading_address, &address, m_addr_width);
        m_is_p0_rx = true;
    }

    if (child <= 5) {
        // For pipes 2-5, only write the LSB
        if (child < 2) {
            write_register(pgm_read_byte(&child_pipe[child]), reinterpret_cast<const uint8_t*>(&address), m_addr_width);
        }
        else {
            write_register(pgm_read_byte(&child_pipe[child]), reinterpret_cast<const uint8_t*>(&address), 1);
        }

        // Note it would be more efficient to set all of the bits for all open
        // pipes at once.  However, I thought it would make the calling code
        // more simple to do it this way.
        write_register(EN_RXADDR, static_cast<uint8_t>(read_register(EN_RXADDR) | (1 <<  pgm_read_byte(&child_pipe_enable[child]))));
    }
}

/****************************************************************************/

void RF24::setAddressWidth(uint8_t a_width)
{
    a_width = static_cast<uint8_t>(a_width - 2);
    if (a_width) {
        write_register(SETUP_AW, static_cast<uint8_t>(a_width % 4));
        m_addr_width = static_cast<uint8_t>((a_width % 4) + 2);
    }
    else {
        write_register(SETUP_AW, static_cast<uint8_t>(0));
        m_addr_width = static_cast<uint8_t>(2);
    }
}

/****************************************************************************/

void RF24::openReadingPipe(uint8_t child, const uint8_t* address)
{
    // If this is pipe 0, cache the address.  This is needed because
    // openWritingPipe() will overwrite the pipe 0 address, so
    // startListening() will have to restore it.
    if (child == 0) {
        memcpy(m_pipe0_reading_address, address, m_addr_width);
        m_is_p0_rx = true;
    }
    if (child <= 5) {
        // For pipes 2-5, only write the LSB
        if (child < 2) {
            write_register(pgm_read_byte(&child_pipe[child]), address, m_addr_width);
        }
        else {
            write_register(pgm_read_byte(&child_pipe[child]), address, 1);
        }

        // Note it would be more efficient to set all of the bits for all open
        // pipes at once.  However, I thought it would make the calling code
        // more simple to do it this way.
        write_register(EN_RXADDR, static_cast<uint8_t>(read_register(EN_RXADDR) | (1 << pgm_read_byte(&child_pipe_enable[child]))));
    }
}

/****************************************************************************/

void RF24::closeReadingPipe(uint8_t pipe)
{
    write_register(EN_RXADDR, static_cast<uint8_t>(read_register(EN_RXADDR) & ~(1 << pgm_read_byte(&child_pipe_enable[pipe]))));
    if (!pipe) {
        // keep track of pipe 0's RX state to avoid null vs 0 in addr cache
        m_is_p0_rx = false;
    }
}

/****************************************************************************/

void RF24::enableDynamicPayloads(void)
{
    // Enable dynamic payload throughout the system

    //toggle_features();
    write_register(FEATURE, read_register(FEATURE) | (1 << EN_DPL));

    // Enable dynamic payload on all pipes
    //
    // Not sure the use case of only having dynamic payload on certain
    // pipes, so the library does not support it.
    write_register(DYNPD, read_register(DYNPD) | (1 << DPL_P5) | (1 << DPL_P4) | (1 << DPL_P3) | (1 << DPL_P2) | (1 << DPL_P1) | (1 << DPL_P0));

    m_dynamic_payloads_enabled = true;
}

/****************************************************************************/

void RF24::disableDynamicPayloads(void)
{
    // Disables dynamic payload throughout the system.  Also disables Ack Payloads

    //toggle_features();
    write_register(FEATURE, 0);

    // Disable dynamic payload on all pipes
    //
    // Not sure the use case of only having dynamic payload on certain
    // pipes, so the library does not support it.
    write_register(DYNPD, 0);

    m_dynamic_payloads_enabled = false;
    m_ack_payloads_enabled = false;
}

/****************************************************************************/

void RF24::enableAckPayload(void)
{
    // enable ack payloads and dynamic payload features

    if (!m_ack_payloads_enabled) {
        write_register(FEATURE, read_register(FEATURE) | (1 << EN_ACK_PAY) | (1 << EN_DPL));

        // Enable dynamic payload on pipes 0 & 1
        write_register(DYNPD, read_register(DYNPD) | (1 << DPL_P1) | (1 << DPL_P0));
        m_dynamic_payloads_enabled = true;
        m_ack_payloads_enabled = true;
    }
}

/****************************************************************************/

void RF24::disableAckPayload(void)
{
    // disable ack payloads (leave dynamic payload features as is)
    if (m_ack_payloads_enabled) {
        write_register(FEATURE, static_cast<uint8_t>(read_register(FEATURE) & ~(1 << EN_ACK_PAY)));

        m_ack_payloads_enabled = false;
    }
}

/****************************************************************************/

void RF24::enableDynamicAck(void)
{
    //
    // enable dynamic ack features
    //
    //toggle_features();
    write_register(FEATURE, read_register(FEATURE) | (1 << EN_DYN_ACK));
}

/****************************************************************************/

bool RF24::writeAckPayload(uint8_t pipe, const void* buf, uint8_t len)
{
    if (m_ack_payloads_enabled) {
        const uint8_t* current = reinterpret_cast<const uint8_t*>(buf);

        write_payload(current, len, W_ACK_PAYLOAD | (pipe & 0x07));
        return !(m_status & (1 << TX_FULL));
    }
    return 0;
}

/****************************************************************************/

bool RF24::isAckPayloadAvailable(void)
{
    return available(NULL);
}

/****************************************************************************/

bool RF24::isPVariant(void)
{
    return m_is_p_variant;
}

/****************************************************************************/

void RF24::setAutoAck(bool enable)
{
    if (enable) {
        write_register(EN_AA, 0x3F);
    }
    else {
        write_register(EN_AA, 0);
        // accommodate ACK payloads feature
        if (m_ack_payloads_enabled) {
            disableAckPayload();
        }
    }
}

/****************************************************************************/

void RF24::setAutoAck(uint8_t pipe, bool enable)
{
    if (pipe < 6) {
        uint8_t en_aa = read_register(EN_AA);
        if (enable) {
            en_aa |= static_cast<uint8_t>((1 << pipe));
        }
        else {
            en_aa = static_cast<uint8_t>(en_aa & ~(1 << pipe));
            if (m_ack_payloads_enabled && !pipe) {
                disableAckPayload();
            }
        }
        write_register(EN_AA, en_aa);
    }
}

/****************************************************************************/

bool RF24::testCarrier(void)
{
    return (read_register(CD) & 1);
}

/****************************************************************************/

bool RF24::testRPD(void)
{
    return (read_register(RPD) & 1);
}

/****************************************************************************/

void RF24::setPALevel(uint8_t level, bool lnaEnable)
{
    uint8_t setup = read_register(RF_SETUP) & static_cast<uint8_t>(0xF8);
    setup |= pa_level_reg_value(level, lnaEnable);
    write_register(RF_SETUP, setup);
}

/****************************************************************************/

uint8_t RF24::getPALevel(void)
{
    return (read_register(RF_SETUP) & ((1 << RF_PWR_LOW) | (1 << RF_PWR_HIGH))) >> 1;
}

/****************************************************************************/

uint8_t RF24::getARC(void)
{
    return read_register(OBSERVE_TX) & 0x0F;
}

/****************************************************************************/

bool RF24::setDataRate(rf24_datarate_e speed)
{
    bool result = false;
    uint8_t setup = read_register(RF_SETUP);

    // HIGH and LOW '00' is 1Mbs - our default
    setup = static_cast<uint8_t>(setup & ~((1 << RF_DR_LOW) | (1 << RF_DR_HIGH)));
    setup |= data_rate_reg_value(speed);

    write_register(RF_SETUP, setup);

    // Verify our result
    if (read_register(RF_SETUP) == setup) {
        result = true;
    }
    return result;
}

/****************************************************************************/

rf24_datarate_e RF24::getDataRate(void)
{
    rf24_datarate_e result;
    uint8_t dr = read_register(RF_SETUP) & ((1 << RF_DR_LOW) | (1 << RF_DR_HIGH));

    // switch uses RAM (evil!)
    // Order matters in our case below
    if (dr == (1 << RF_DR_LOW)) {
        // '10' = 250KBPS
        result = RF24_250KBPS;
    }
    else if (dr == (1 << RF_DR_HIGH)) {
        // '01' = 2MBPS
        result = RF24_2MBPS;
    }
    else {
        // '00' = 1MBPS
        result = RF24_1MBPS;
    }
    return result;
}

/****************************************************************************/

void RF24::setCRCLength(rf24_crclength_e length)
{
    m_config_reg = static_cast<uint8_t>(m_config_reg & ~((1 << CRCO) | (1 << EN_CRC)));

    // switch uses RAM (evil!)
    if (length == RF24_CRC_DISABLED) {
        // Do nothing, we turned it off above.
    }
    else if (length == RF24_CRC_8) {
        m_config_reg |= (1 << EN_CRC);
    }
    else {
        m_config_reg |= (1 << EN_CRC);
        m_config_reg |= (1 << CRCO);
    }
    write_register(NRF_CONFIG, m_config_reg);
}

/****************************************************************************/

rf24_crclength_e RF24::getCRCLength(void)
{
    rf24_crclength_e result = RF24_CRC_DISABLED;
    uint8_t AA = read_register(EN_AA);
    m_config_reg = read_register(NRF_CONFIG);

    if (m_config_reg & (1 << EN_CRC) || AA) {
        if (m_config_reg & (1 << CRCO)) {
            result = RF24_CRC_16;
        }
        else {
            result = RF24_CRC_8;
        }
    }

    return result;
}

/****************************************************************************/

void RF24::disableCRC(void)
{
    m_config_reg = static_cast<uint8_t>(m_config_reg & ~(1 << EN_CRC));
    write_register(NRF_CONFIG, m_config_reg);
}

/****************************************************************************/
void RF24::setRetries(uint8_t delay, uint8_t count)
{
    write_register(SETUP_RETR, static_cast<uint8_t>(rf24_min(15, delay) << ARD | rf24_min(15, count)));
}

/****************************************************************************/
void RF24::startConstCarrier(rf24_pa_dbm_e level, uint8_t channel)
{
    stopListening();
    write_register(RF_SETUP, read_register(RF_SETUP) | (1 << CONT_WAVE) | (1 << PLL_LOCK));
    if (isPVariant()) {
        setAutoAck(0);
        setRetries(0, 0);
        uint8_t dummy_buf[32];
        for (uint8_t i = 0; i < 32; ++i)
            dummy_buf[i] = 0xFF;

        // use write_register() instead of openWritingPipe() to bypass
        // truncation of the address with the current RF24::addr_width value
        write_register(TX_ADDR, reinterpret_cast<uint8_t*>(&dummy_buf), 5);
        flush_tx(); // so we can write to top level

        // use write_register() instead of write_payload() to bypass
        // truncation of the payload with the current RF24::payload_size value
        write_register(W_TX_PAYLOAD, reinterpret_cast<const uint8_t*>(&dummy_buf), 32);

        disableCRC();
    }
    setPALevel(level);
    setChannel(channel);

    ce(true);
    if (isPVariant()) {
        delay_ms(1); // datasheet says 1 ms is ok in this instance
        ce(false);
        reUseTX();
    }
}

/****************************************************************************/

void RF24::stopConstCarrier()
{
    /*
     * A note from the datasheet:
     * Do not use REUSE_TX_PL together with CONT_WAVE=1. When both these
     * registers are set the chip does not react when setting CE low. If
     * however, both registers are set PWR_UP = 0 will turn TX mode off.
     */
    powerDown(); // per datasheet recommendation (just to be safe)
    write_register(RF_SETUP, static_cast<uint8_t>(read_register(RF_SETUP) & ~(1 << CONT_WAVE) & ~(1 << PLL_LOCK)));
    ce(false);
}

/****************************************************************************/

void RF24::toggleAllPipes(bool isEnabled)
{
    write_register(EN_RXADDR, static_cast<uint8_t>(isEnabled ? 0x3F : 0));
}

/****************************************************************************/

uint8_t RF24::data_rate_reg_value(rf24_datarate_e speed)
{
    m_txDelay = 280;
    if (speed == RF24_250KBPS) {
        m_txDelay = 505;
        // Must set the RF_DR_LOW to 1; RF_DR_HIGH (used to be RF_DR) is already 0
        // Making it '10'.
        return static_cast<uint8_t>((1 << RF_DR_LOW));
    }
    else if (speed == RF24_2MBPS) {
        m_txDelay = 240;
        // Set 2Mbs, RF_DR (RF_DR_HIGH) is set 1
        // Making it '01'
        return static_cast<uint8_t>((1 << RF_DR_HIGH));
    }
    // HIGH and LOW '00' is 1Mbs - our default
    return static_cast<uint8_t>(0);
}

/****************************************************************************/

uint8_t RF24::pa_level_reg_value(uint8_t level, bool lnaEnable)
{
    // If invalid level, go to max PA
    // Else set level as requested
    // + lnaEnable (1 or 0) to support the SI24R1 chip extra bit
    return static_cast<uint8_t>(((level > RF24_PA_MAX ? static_cast<uint8_t>(RF24_PA_MAX) : level) << 1) + lnaEnable);
}

/****************************************************************************/

void RF24::setRadiation(uint8_t level, rf24_datarate_e speed, bool lnaEnable)
{
    uint8_t setup = data_rate_reg_value(speed);
    setup |= pa_level_reg_value(level, lnaEnable);
    write_register(RF_SETUP, setup);
}

#endif // PERIPHERAL_NRF24L01