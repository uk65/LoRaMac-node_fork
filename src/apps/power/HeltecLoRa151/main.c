#include "board.h"
#include "power_app.h"
#include "power_radio.h"
#include "timer.h"

int main( void )
{
    BoardInitMcu( );
    BoardInitPeriph( );
    power_radio_init( );
    power_app_init( );

    while( 1 )
    {
        power_app_process( );
        BoardLowPowerHandler( );
        power_radio_process( );
        TimerProcess( );
    }
}
