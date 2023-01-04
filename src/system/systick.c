
#include "systick.h"
#include "systick-board.h"


uint32_t SysTick(void)
{
    return SysTickMcu();
}