#include <stdio.h>
#include <string.h>
#include "../firmwareVersion.h"
#include "../../common/githubVersion.h"
#include "utilities.h"
#include "board.h"
#include "gpio.h"
#include "uart.h"
#include "RegionCommon.h"
#include "delay.h"
#include "timer.h"
//#include "radio.h"
#include "Commissioning.h"
#include "LmHandler.h"
#include "LmhpCompliance.h"
#include "CayenneLpp.h"
#include "NvmDataMgmt.h"


#define ACTIVE_REGION                               LORAMAC_REGION_EU868

// LoRaWAN default end-device class
#define LORAWAN_DEFAULT_CLASS                       CLASS_A

// Defines the application data transmission duty cycle. 5s, value in [ms].
#define APP_TX_DUTYCYCLE                            5000

// Defines a random delay for application data transmission duty cycle. 1s, value in [ms].
#define APP_TX_DUTYCYCLE_RND                        1000

// LoRaWAN Adaptive Data Rate. Please note that when ADR is enabled the end-device should be static
#define LORAWAN_ADR_STATE                           LORAMAC_HANDLER_ADR_ON

// Default datarate. Please note that LORAWAN_DEFAULT_DATARATE is used only when ADR is disabled 
#define LORAWAN_DEFAULT_DATARATE                    DR_0

// LoRaWAN confirmed messages
#define LORAWAN_DEFAULT_CONFIRMED_MSG_STATE         LORAMAC_HANDLER_UNCONFIRMED_MSG

// User application data buffer size
#define LORAWAN_APP_DATA_BUFFER_MAX_SIZE            242

// LoRaWAN ETSI duty cycle control enable/disable. Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
#define LORAWAN_DUTYCYCLE_ON                        true

// LoRaWAN application port. The allowed port range is from 1 up to 223. Other values are reserved.
#define LORAWAN_APP_PORT                            2


typedef enum {
    LORAMAC_HANDLER_TX_ON_TIMER,
    LORAMAC_HANDLER_TX_ON_EVENT,
} LmHandlerTxEvents_t;

// User application data
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

// User application data structure
static LmHandlerAppData_t AppData = {
    .Port       = 0,
    .BufferSize = 0,
    .Buffer     = AppDataBuffer,
};

// Specifies the state of the application LED
static bool AppLedStateOn = false;

// Timer to handle the application data transmission duty cycle
static TimerEvent_t TxTimer;

// Timer to handle the state of LED1
static TimerEvent_t Led1Timer;

// GPIO pin objects
extern Gpio_t Led1;
extern Gpio_t Nvm_Reset;

// Timer to handle the state of LED beacon indicator
static TimerEvent_t LedBeaconTimer;

// Indicates if LoRaMacProcess call is pending.
// If variable is equal to 0 then the MCU can be set in low power mode
static volatile uint8_t IsMacProcessPending = 0;
static volatile uint8_t IsTxFramePending = 0;
static volatile uint32_t TxPeriodicity = 0;

static void OnTxTimerEvent (void *context);
static void OnMacProcessNotify (void);
static void OnNvmDataChange (LmHandlerNvmContextStates_t state, uint16_t size);
static void OnNetworkParametersChange (CommissioningParams_t *params);
static void OnMacMcpsRequest (LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxIn);
static void OnMacMlmeRequest (LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxIn);
static void OnJoinRequest (LmHandlerJoinParams_t *params);
static void OnTxData (LmHandlerTxParams_t *params);
static void OnRxData (LmHandlerAppData_t *appData, LmHandlerRxParams_t *params);
static void OnClassChange (DeviceClass_t deviceClass);
static void OnBeaconStatusChange (LoRaMacHandlerBeaconParams_t *params);
static void OnSysTimeUpdate (bool isSynchronized, int32_t timeCorrection);
static void PrepareTxFrame (void);
static void StartTxProcess (LmHandlerTxEvents_t txEvent);
static void UplinkProcess (void);

// LmhpCompliance, LmHandlerPackageRegister()
static void OnTxPeriodicityChanged (uint32_t periodicity);
static void OnTxFrameCtrlChanged (LmHandlerMsgTypes_t isTxConfirmed);
static void OnPingSlotPeriodicityChanged (uint8_t pingSlotPeriodicity);

static void OnTxTimerEvent (void *context);
static void OnLed1TimerEvent (void *context);
//static void OnLedBeaconTimerEvent (void *context);

static LmHandlerCallbacks_t LmHandlerCallbacks = {
    .GetBatteryLevel                = BoardGetBatteryLevel,
    .GetTemperature                 = NULL,
    .GetRandomSeed                  = BoardGetRandomSeed,
    .OnMacProcess                   = OnMacProcessNotify,
    .OnNvmDataChange                = OnNvmDataChange,
    .OnNetworkParametersChange      = OnNetworkParametersChange,
    .OnMacMcpsRequest               = OnMacMcpsRequest,
    .OnMacMlmeRequest               = OnMacMlmeRequest,
    .OnJoinRequest                  = OnJoinRequest,
    .OnTxData                       = OnTxData,
    .OnRxData                       = OnRxData,
    .OnClassChange                  = OnClassChange,
    .OnBeaconStatusChange           = OnBeaconStatusChange,
    .OnSysTimeUpdate                = OnSysTimeUpdate,
};

static LmHandlerParams_t LmHandlerParams = {
    .Region                         = ACTIVE_REGION,
    .AdrEnable                      = LORAWAN_ADR_STATE,
    .IsTxConfirmed                  = LORAWAN_DEFAULT_CONFIRMED_MSG_STATE,
    .TxDatarate                     = LORAWAN_DEFAULT_DATARATE,
    .PublicNetworkEnable            = LORAWAN_PUBLIC_NETWORK,
    .DutyCycleEnabled               = LORAWAN_DUTYCYCLE_ON,
    .DataBufferMaxSize              = LORAWAN_APP_DATA_BUFFER_MAX_SIZE,
    .DataBuffer                     = AppDataBuffer,
    .PingSlotPeriodicity            = REGION_COMMON_DEFAULT_PING_SLOT_PERIODICITY,
};

#ifdef __cplusplus
    // Initialisierung der union FwVersion geht so nicht in c++
    static LmhpComplianceParams_t LmhpComplianceParams;
#else
    static LmhpComplianceParams_t LmhpComplianceParams = {
        .FwVersion.Value                = FIRMWARE_VERSION,
        .OnTxPeriodicityChanged         = OnTxPeriodicityChanged,
        .OnTxFrameCtrlChanged           = OnTxFrameCtrlChanged,
        .OnPingSlotPeriodicityChanged   = OnPingSlotPeriodicityChanged,
    };
#endif // __cplusplus

static void blink(uint32_t ms_on, uint32_t ms_off);

static void OnMacProcessNotify (void)
{
    IsMacProcessPending = 1;
}

static void OnNvmDataChange(LmHandlerNvmContextStates_t state, uint16_t size)
{
    blink(500, 1000);
}

static void OnNetworkParametersChange(CommissioningParams_t *params)
{
    blink(500, 1000);
}

static void OnMacMcpsRequest(LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxIn)
{
    blink(500, 1000);
}

static void OnMacMlmeRequest(LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxIn)
{
    blink(500, 1000);
}

static void OnJoinRequest(LmHandlerJoinParams_t *params)
{
    if (params->Status == LORAMAC_HANDLER_ERROR) {
        LmHandlerJoin();
    } else {
        LmHandlerRequestClass(LORAWAN_DEFAULT_CLASS);
    }
}

static void OnTxData(LmHandlerTxParams_t *params)
{
    blink(500, 1000);
}

static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
    switch (appData->Port) {
        case 1: // The application LED can be controlled on port 1 or 2
        case LORAWAN_APP_PORT: {
                AppLedStateOn = appData->Buffer[0] & 0x01;
            }
            break;
        default:
            break;
    }

    // Switch LED 2 ON for each received downlink
    //GpioWrite(&Led2, 1);
    //TimerStart(&Led2Timer);
}

static void OnClassChange(DeviceClass_t deviceClass)
{
    // Inform the server as soon as possible that the end-device has switched to ClassB
    LmHandlerAppData_t appData =
    {
        .Port       = 0,
        .BufferSize = 0,
        .Buffer     = NULL,
    };
    LmHandlerSend(&appData, LORAMAC_HANDLER_UNCONFIRMED_MSG);
}

static void OnBeaconStatusChange(LoRaMacHandlerBeaconParams_t *params)
{
    switch (params->State) {
        case LORAMAC_HANDLER_BEACON_RX:
        {
            TimerStart(&LedBeaconTimer);
            break;
        }
        case LORAMAC_HANDLER_BEACON_LOST:
        case LORAMAC_HANDLER_BEACON_NRX:
        {
            TimerStop(&LedBeaconTimer);
            break;
        }
        default:
        {
            break;
        }
    }
}

static void OnSysTimeUpdate(bool isSynchronized, int32_t timeCorrection)
{
}

static void PrepareTxFrame(void)
{
    if (LmHandlerIsBusy() == true) {
        return;
    }

    uint8_t channel = 0;

    AppData.Port = LORAWAN_APP_PORT;

    CayenneLppReset();
    //CayenneLppAddDigitalInput(channel++, AppLedStateOn);
    CayenneLppAddAnalogInput(channel++, BoardGetBatteryLevel() * 100 / 254);
    CayenneLppAddTemperature(channel, 21.79);

    CayenneLppCopy(AppData.Buffer);
    AppData.BufferSize = CayenneLppGetSize();

    if (LmHandlerSend(&AppData, LmHandlerParams.IsTxConfirmed) == LORAMAC_HANDLER_SUCCESS) {
        // Switch LED 1 ON
        GpioWrite(&Led1, 1);
        TimerStart(&Led1Timer);
    }
}

static void StartTxProcess(LmHandlerTxEvents_t txEvent)
{
    switch (txEvent) {
    default:
        // Intentional fall through
    case LORAMAC_HANDLER_TX_ON_TIMER:
        {
            // Schedule 1st packet transmission
            TimerInit(&TxTimer, OnTxTimerEvent);
            TimerSetValue(&TxTimer, TxPeriodicity);
            OnTxTimerEvent(NULL);
        }
        break;
    case LORAMAC_HANDLER_TX_ON_EVENT:
        {
        }
        break;
    }
}

static void UplinkProcess(void)
{
    uint8_t isPending = 0;
    CRITICAL_SECTION_BEGIN();
    isPending = IsTxFramePending;
    IsTxFramePending = 0;
    CRITICAL_SECTION_END();
    if (isPending == 1) {
        PrepareTxFrame();
    }
}

static void OnTxPeriodicityChanged(uint32_t periodicity)
{
    TxPeriodicity = periodicity;

    if (TxPeriodicity == 0) {
        // Revert to application default periodicity
        TxPeriodicity = APP_TX_DUTYCYCLE + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);
    }

    // Update timer periodicity
    TimerStop(&TxTimer);
    TimerSetValue(&TxTimer, TxPeriodicity);
    TimerStart(&TxTimer);
}

static void OnTxFrameCtrlChanged(LmHandlerMsgTypes_t isTxConfirmed)
{
    LmHandlerParams.IsTxConfirmed = isTxConfirmed;
}

static void OnPingSlotPeriodicityChanged(uint8_t pingSlotPeriodicity)
{
    LmHandlerParams.PingSlotPeriodicity = pingSlotPeriodicity;
}

static void OnTxTimerEvent(void *context)
{
    TimerStop(&TxTimer);

    IsTxFramePending = 1;

    // Schedule next transmission
    TimerSetValue(&TxTimer, TxPeriodicity);
    TimerStart(&TxTimer);
}

static void OnLed1TimerEvent(void *context)
{
    TimerStop(&Led1Timer);
    // Switch LED 1 OFF
    GpioWrite(&Led1, 0);
}

#if 0
static void OnLedBeaconTimerEvent (void *context)
{
    //GpioWrite(&Led2, 1);
    //TimerStart(&Led2Timer);
    TimerStart(&LedBeaconTimer);
}
#endif

static void blink(uint32_t ms_on, uint32_t ms_off)
{
    GpioWrite(&Led1, 1);
    DelayMs(ms_on);
    GpioWrite(&Led1, 0);
    DelayMs(ms_off);
}

int main(void)
{
    BoardInitMcu();
    BoardInitPeriph();

    // Wenn Pin A8 == 0 ==> VvmData zurücksetzen
    if (GpioRead(&Nvm_Reset) == 0) {
        NvmDataMgmtFactoryReset();
        while (GpioRead(&Nvm_Reset) == 0) {
            blink(500, 1000);
        }
    }

    TimerInit(&Led1Timer, OnLed1TimerEvent);
    TimerSetValue(&Led1Timer, 25);

    TxPeriodicity = APP_TX_DUTYCYCLE + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);

    if (LmHandlerInit(&LmHandlerCallbacks, &LmHandlerParams) != LORAMAC_HANDLER_SUCCESS) {
        while (1) {
            blink(200, 500);
        }
    }

    // Set system maximum tolerated rx error in milliseconds
    LmHandlerSetSystemMaxRxError(20);

    {
#ifdef __cplusplus
        LmhpComplianceParams.FwVersion.Value                = FIRMWARE_VERSION,
        LmhpComplianceParams.OnTxPeriodicityChanged         = OnTxPeriodicityChanged,
        LmhpComplianceParams.OnTxFrameCtrlChanged           = OnTxFrameCtrlChanged,
        LmhpComplianceParams.OnPingSlotPeriodicityChanged   = OnPingSlotPeriodicityChanged,
#endif
        // The LoRa-Alliance Compliance protocol package should always be initialized and activated.
        LmHandlerPackageRegister(PACKAGE_ID_COMPLIANCE, &LmhpComplianceParams);
    }

    LmHandlerJoin();

    StartTxProcess(LORAMAC_HANDLER_TX_ON_TIMER);

    while (1) {
        
        // Processes the LoRaMac events
        LmHandlerProcess();

        // Process application uplinks management
        UplinkProcess();

        CRITICAL_SECTION_BEGIN();
        if (IsMacProcessPending == 1) {
            // Clear flag and prevent MCU to go into low power modes.
            IsMacProcessPending = 0;
        } else {
            // The MCU wakes up through events
            BoardLowPowerHandler();
        }
        CRITICAL_SECTION_END();
    }
}
