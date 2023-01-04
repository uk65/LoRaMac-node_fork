
#ifndef _BH_1750_H_
#define _BH_1750_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "utilities.h"
#include "i2c.h"
    
typedef struct BH1750_s {
    I2c_t  I2c;   
} BH1750_t;


void BH1750Init(void);

LmnStatus_t BH1750Read(uint16_t *value);
    

extern BH1750_t  BH1750;


#ifdef __cplusplus
}
#endif

#endif // _BH_1750_H_