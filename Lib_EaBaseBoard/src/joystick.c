/*****************************************************************************
 *   joystick.c:  Driver for the Joystick switch
 *
 *   Copyright(C) 2009, Embedded Artists AB
 *   All rights reserved.
 *
 ******************************************************************************/

/*
 * NOTE: GPIOInit must have been called before using any functions in this
 * file.
 *
 */


/******************************************************************************
 * Includes
 *****************************************************************************/

#include "mcu_regs.h"
#include "type.h"
#include "gpio.h"
#include "joystick.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

/******************************************************************************
 * External global variables
 *****************************************************************************/

/******************************************************************************
 * Local variables
 *****************************************************************************/


/******************************************************************************
 * Local Functions
 *****************************************************************************/


/******************************************************************************
 * Public Functions
 *****************************************************************************/

/******************************************************************************
 *
 * Description:
 *    Initialize the Joystick Driver
 *
 *****************************************************************************/
void joystick_init (void)
{
    /* set the GPIOs as inputs */
    GPIOSetDir( PORT2, 0, 0 );
    GPIOSetDir( PORT2, 1, 0 );
    GPIOSetDir( PORT2, 2, 0 );
    GPIOSetDir( PORT2, 3, 0 );
#if defined (MCU_LPC1343)
    GPIOSetDir( PORT2, 4, 0 );
#elif defined (MCU_LPC1114)
    GPIOSetDir( PORT3, 4, 0 );
#else
#error __USE_CMSIS undefined
#endif
}

/******************************************************************************
 *
 * Description:
 *    Read the joystick status
 *
 * Returns:
 *   The joystick status. The returned value is a bit mask. More than one
 *   direction may be active at any given time (e.g. UP and RIGHT)
 *
 *****************************************************************************/
uint8_t joystick_read(void)
{
    uint8_t status = 0;

    if (!GPIOGetValue( PORT2, 0)) {
        status |= JOYSTICK_CENTER;
    }

    if (!GPIOGetValue( PORT2, 1)) {
        status |= JOYSTICK_DOWN;
    }

    if (!GPIOGetValue( PORT2, 2)) {
        status |= JOYSTICK_RIGHT;
    }

    if (!GPIOGetValue( PORT2, 3)) {
        status |= JOYSTICK_UP;
    }

#if defined (MCU_LPC1343)
    if (!GPIOGetValue( PORT2, 4)) {
        status |= JOYSTICK_LEFT;
    }
#elif defined(MCU_LPC1114)
    if (!GPIOGetValue( PORT3, 4)) {
        status |= JOYSTICK_LEFT;
    }
#else
#error __USE_CMSIS undefined
#endif

    return status;
}





