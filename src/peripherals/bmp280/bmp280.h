
#ifndef _BMP_280_H_
#define _BMP_280_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "utilities.h"
#include "i2c.h"
    
typedef struct BMP280_s {
    I2c_t  I2c;   
} BMP280_t;

typedef union valif_u {
    int32_t i32;
    float f;
} valif_t;

typedef union valuif_u {
    uint32_t ui32;
    float f;
} valuif_t;

typedef struct BMP280Data_s {
    valif_t   temp;
    valuif_t  press;
    valuif_t  hum;
} BMP280Data_t;


void BMP280Init(void);

LmnStatus_t ReadBMP280Fixed(BMP280Data_t *data);

LmnStatus_t ReadBMP280Float(BMP280Data_t *data);
    

extern BMP280_t  BMP280;


#ifdef __cplusplus
}
#endif

#endif // _BMP_280_H_