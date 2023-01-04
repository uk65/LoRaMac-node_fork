
#include "vl53l1x.h"
#include "delay.h"
#include "vl53l1x_api.h"

#ifdef PERIPHERAL_VL53L1X

VL53L1X_t  VL53L1X;

static const uint16_t dev = 0x52;


LmnStatus_t VL53L1XInit(void)
{
    int status = 0;

    // chip boot
    uint8_t sensorState = 0;
    while (sensorState == 0) {
        status = VL53L1X_BootState(dev, &sensorState);
	    DelayMs(2);
    }

    // This function must to be called to initialize the sensor with the default setting
    status = VL53L1X_SensorInit(dev);
    
    // Optional functions to be used to change the main ranging parameters according the 
    // application requirements to get the best ranging performances
    status = VL53L1X_SetDistanceMode(dev, VL53L1X.DistanceMode);  // 1 = short, 2 = long
    status = VL53L1X_SetTimingBudgetInMs(dev, 100);               // in ms possible values [20, 50, 100, 200, 500]
    status = VL53L1X_SetInterMeasurementInMs(dev, 100);           // in ms, IM must be > = TB
    // status = VL53L1X_SetOffset(dev,20);                     // offset compensation in mm
    // status = VL53L1X_SetROI(dev, 16, 16);                   // minimum ROI 4,4
    // status = VL53L1X_CalibrateOffset(dev, 140, &offset);    // may take few second to perform the offset cal
    // status = VL53L1X_CalibrateXtalk(dev, 1000, &xtalk);     // may take few second to perform the xtalk cal

    // This function has to be called to enable the ranging
    // status = VL53L1X_StartRanging(dev);
    
    return (status == 1) ? LMN_STATUS_OK : LMN_STATUS_ERROR;
}

LmnStatus_t VL53L1XIStartRanging(void)
{
    int status = VL53L1X_StartRanging(dev);
    return (status == 1) ? LMN_STATUS_OK : LMN_STATUS_ERROR;
}

LmnStatus_t VL53L1XIStopRanging(void)
{
    int status = VL53L1X_StopRanging(dev);
    return (status == 1) ? LMN_STATUS_OK : LMN_STATUS_ERROR;    
}

LmnStatus_t VL53L1XGetDistance(uint16_t *distance)
{
    int status = 0;

    // uint16_t SignalRate, AmbientRate, SpadNum; 
    // uint8_t RangeStatus;
    
    uint8_t dataReady = 0;
    while (dataReady == 0) {
        status = VL53L1X_CheckForDataReady(dev, &dataReady);
        DelayMs(2);
    }
    dataReady = 0;

    // status = VL53L1X_GetRangeStatus(dev, &RangeStatus);
    status = VL53L1X_GetDistance(dev, distance);
    // status = VL53L1X_GetSignalRate(dev, &SignalRate);
    // status = VL53L1X_GetAmbientRate(dev, &AmbientRate);
    // status = VL53L1X_GetSpadNb(dev, &SpadNum);
    // status = VL53L1X_ClearInterrupt(dev); /* clear interrupt has to be called to enable next interrupt*/
    
    return (status == 1) ? LMN_STATUS_OK : LMN_STATUS_ERROR;
}

#endif // PERIPHERAL_VL53L1X