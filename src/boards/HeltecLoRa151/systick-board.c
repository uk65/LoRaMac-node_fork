
#include "stm32l1xx.h"
#include "systick-board.h"

uint32_t SysTickMcu(void)
{
    return HAL_GetTick();
}