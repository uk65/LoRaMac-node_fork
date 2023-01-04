
#include "bmp280.h"
#include "bmp280_driver.h"

#ifdef PERIPHERAL_BMP280

static BMP280_Handle  s_bmp280;


void BMP280Init(void)
{
    bmp280_init_default_params(&s_bmp280.params);
    s_bmp280.addr = BMP280_I2C_ADDRESS_0;
    bmp280_init(&s_bmp280, &s_bmp280.params);
}

LmnStatus_t ReadBMP280Fixed(BMP280Data_t *data)
{
    bool res = bmp280_read_fixed(&s_bmp280, &data->temp.i32, &data->press.ui32, &data->hum.ui32);
    return res ? LMN_STATUS_OK : LMN_STATUS_ERROR; 
}

LmnStatus_t ReadBMP280Float(BMP280Data_t *data)
{
    bool res = bmp280_read_float(&s_bmp280, &data->temp.f, &data->press.f, &data->hum.f);
    return res ? LMN_STATUS_OK : LMN_STATUS_ERROR; 
}

BMP280_t  BMP280;

#endif // PERIPHERAL_BMP280