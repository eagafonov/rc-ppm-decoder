#include <stdio.h>
#include <stdlib.h>

#include "gpio-event-drv.h"

/******************************************************************************************************
 *
  RC Transiver PPM signal

  K channels is encoded as K+1 pulses of fixed width (approx 0.4 msec)
  Channel  value is a time between two raising edges
  Ususaly ~1.0 msec is a minimal value, ~2.0 msec - max value

  PPM frames are separared by a gap (more than 4-10 msec)


            Nth PPM frame                                                         (N+1)th PPM frame

HIGH   |---|     |---|          |---|    |---|      |---|                         |----| 
       |   |     |   |          |   |    |   |      |   |                         |    |
       |   |     |   |          |   |    |   |      |   |                         |    |
       |   |     |   |          |   |    |   |      |   |                         |    |
       |   |     |   |          |   |    |   |      |   |                         |    |
LOW ___|   |_____|   |__________|   |____|   |______|   |_________________________|    |

 -------------------------------------------------------------------------------------------------->
                                                                                            time
       |<---t1-->|<------t2---->|<--t3-->|<---t4--->|<------------ gap ---------->|
 
 ************************************************************************************************************/


#define  GAP 4000
#define GPIO_MONITOR_PIN 3

int main( int argc, char **argv )
{
    FILE               *fs;

    // check device driver file
    if (( fs = fopen( "/dev/gpio-event", "r" )) == NULL )
    {
        perror( "Check to make sure gpio_event_drv has been loaded. Unable to open /dev/gpio-event" );
        exit( 1 );
    }

    // set binary read mode
    ioctl( fileno( fs ), GPIO_EVENT_IOCTL_SET_READ_MODE, 1 );

    // setup GPIO Monitoring pins

    GPIO_EventMonitor_t gpio_minotor = {0};

    // Monitr pin for raising edges
    gpio_minotor.gpio = GPIO_MONITOR_PIN; // TODO read from command line options
    gpio_minotor.edgeType = GPIO_EventRisingEdge; // It is important to monitor rising edges only to 
    gpio_minotor.debounceMilliSec = 0;
    gpio_minotor.onOff = 1;

    ioctl( fileno( fs ), GPIO_EVENT_IOCTL_MONITOR_GPIO, &gpio_minotor);


    printf("reading PPM:\n");

    printf("If you don't see fast running output log, make sure GPIO%d is set-up for input.\n", GPIO_MONITOR_PIN);
    printf("The following command will do it for you:\necho \"set gpio 3 input\" > /dev/v2r_gpio\n\n");

    long last_usec = 0;

    int channel_index = 0;

    while ( 1 )
    {
        ssize_t numBytes;

        GPIO_Event_t    gpioEvent;

        if (( numBytes = fread( &gpioEvent, 1, sizeof( gpioEvent ), fs )) == sizeof( gpioEvent ))
        {
            // Filter out alien pin events
            if (gpioEvent.gpio != GPIO_MONITOR_PIN) {
                continue;
            }

            long usec = gpioEvent.time.tv_sec * 1000000 + gpioEvent.time.tv_usec;

            if (!last_usec) {
                last_usec = usec;
                continue;
            }

            long delay = usec - last_usec;
            last_usec = usec;

            if (delay > GAP) { // gap is 2 x max channel duration 
                // End of channels
                printf("\n");
                channel_index = 0;

                //TODO drop (possibly) incomplete first frame
            } else {
                // Got next channel
                channel_index++;
                printf("  %d:%5ld", channel_index, delay);
            };
        }
    }

    fclose( fs );

    return 0;
}
