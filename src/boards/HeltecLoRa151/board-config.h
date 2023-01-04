// ===================================================================================
//
//                      ------------------------------------
//                     |           Heltec LoRa 151          |
//                      ------------------------------------
//                     |== RST                       USER ==|
//                     |  *VUSB*          1          *VDD*  |
//                     |  *GND*           2          *GND*  |
//          LED        |   PB8            3           PA14  |     SWCLK
//                     |   PB9            4           PA13  |     SWDIO
//          SDA1       |   PB7            5           PA10  |     RX1
//          SCL1       |   PB6            6           PA9   |     TX1
//                     |   PB5            7           PA8   |     $ NVM_RESET
//                     |  *NRST*          8           PB15  |     # RF24_MOSI
//                     |   PA0            9           PB14  |     # RF24_MISO
//                     |   PA1           10           PB13  |     # RF24_SCK
//                     |   PA2           11           PA15  |
//                     |  *VEXT*         12           PB3   |     # RF24_INT
//         LoRa SCK =  |   PA5           13          *VEXT* |
//        LoRa MISO =  |   PA6           14           PB4   |     # RF24_CE
//        LoRa MOSI =  |   PA7   PB0     15     PB11  PB12  |     # RF24_NSS
//         LoRa NSS =  |   PA4   PB1     16     PB10  PA3   |     = LoRa RST
//                     |                                    |
//                     |               Antenne              |
//                      ------------------------------------
//
//                            LoRa DIO3       = LoRa DIO0
//                            LoRa DIO2       = LoRa DIO1
//
// ===================================================================================
//
//                  SPI1        SPI2        LoRa        RF24
//      -----------------------------------------------------------------------
//      PA0
//      PA1
//      PA2
//      PA3                                             RF24_INT
//      PA4         LoRa_NSS
//      PA5         LoRa_SCK                
//      PA6         LoRa_MISO
//      PA7         LoRa_MOSI
//      PA8
//      PA9
//      PA10
//      PA13                                                            SWDIO
//      PA14                                                            SWCLK
//
//      PB0                                 LoRa_DIO3
//      PB1                                 LoRa_DIO2
//      PB4                                             RF24_CE
//      PB5
//      PB6
//      PB7
//      PB8                                                             LED
//      PB9
//      PB10                                LoRa_DIO1
//      PB11                                LoRa_DIO0
//      PB12                    RF24_NSS
//      PB13                    RF24_CLK
//      PB14                    RF24_MISO
//      PB15                    RF24_MOSI
//
// ===================================================================================


#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

// Defines the time required for the TCXO to wakeup [ms].
#define BOARD_TCXO_WAKEUP_TIME                      0

// Board MCU pins definitions
#define RADIO_RESET                                 PA_3

#define RADIO_MOSI                                  PA_7
#define RADIO_MISO                                  PA_6
#define RADIO_SCLK                                  PA_5
#define RADIO_NSS                                   PA_4

#define RADIO_DIO_0                                 PB_11
#define RADIO_DIO_1                                 PB_10
#define RADIO_DIO_2                                 PB_1
#define RADIO_DIO_3                                 PB_0
#define RADIO_DIO_4                                 NC
#define RADIO_DIO_5                                 NC

#define RADIO_ANT_SWITCH                            NC

#define LED_1                                       PB_8
#define LED_2                                       NC

#define NVM_RESET                                   PA_8

#define RF24_MOSI                                   PB_15
#define RF24_MISO                                   PB_14
#define RF24_SCLK                                   PB_13
#define RF24_NSS                                    PB_12

#define RF24_INT                                    PB_3
#define RF24_CE                                     PB_4

// Debug pins definition.
#define RADIO_DBG_PIN_TX                            NC  // PA_2
#define RADIO_DBG_PIN_RX                            NC  // PA_3

#define OSC_LSE_IN                                  PC_14
#define OSC_LSE_OUT                                 PC_15

#define OSC_HSE_IN                                  PH_0
#define OSC_HSE_OUT                                 PH_1

#define SWCLK                                       NC  // PA_14
#define SWDAT                                       NC  // PA_13

#define I2C_SCL                                     PB_6
#define I2C_SDA                                     PB_7

#define UART_TX                                     NC  // PA_9
#define UART_RX                                     NC  // PA_10

#ifdef __cplusplus
}
#endif

#endif // __BOARD_CONFIG_H__
