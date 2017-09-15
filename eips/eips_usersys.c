/*
 *            Copyright (c) 2002-2013 by Real Time Automation, Inc.
 *
 *  This software is copyrighted by and is the sole property of
 *  Real Time Automation, Inc. (RTA).  All rights, title, ownership, 
 *  or other interests in the software remain the property of RTA.  
 *  This software may only be used in accordance with the corresponding
 *  license agreement.  Any unauthorized use, duplication, transmission,
 *  distribution, or disclosure of this software is expressly forbidden.
 *
 *  This Copyright notice MAY NOT be removed or modified without prior
 *  written consent of RTA.
 *
 *  RTA reserves the right to modify this software without notice.
 *
 *  Real Time Automation
 *  150 S. Sunny Slope Road            USA 262.439.4999
 *  Suite 130                          www.rtaautomation.com
 *  Brookfield, WI 53005               software@rtaautomation.com
 *
 *************************************************************************
 *
 *    Version Date: 05NOV2013
 *         Version: 2.37
 *    Conformed To: EtherNet/IP Protocol Conformance Test A-10 (28-AUG-2012)
 *     Module Name: eips_usersys.c
 *          Author: Jamin D. Wendorf (jwendorf@rtaautomation.com)
 *        Language: Ansi C
 * Compile Options: N/A
 * Compile defines: N/A
 *       Libraries: N/A
 *    Link Options: N/A
 *
 * Description.
 * =======================================================================
 * This file contains system functions that need to be written by the user.
 * This includes timer, NVRAM, and task calls.
 *
 */

/* ---------------------------- */
/* INCLUDE FILES                */
/* ---------------------------- */
#include "led.h"                // LED management header file
#include "eips_system.h"		// eips system
#include "eips_usersys.h"       // header file

/* used for debug */
#undef  __RTA_FILE__
#define __RTA_FILE__ "eips_usersys.c"

/* ---------------------------- */
/* EXTERN FUNCTIONS             */
/* ---------------------------- */

/* ---------------------------- */
/* LOCAL STRUCTURE DEFINITIONS  */
/* ---------------------------- */

/* ---------------------------- */
/* STATIC VARIABLES             */
/* ---------------------------- */

/* ---------------------------- */
/* EXTERN VARIABLES             */
/* ---------------------------- */

/* ---------------------------- */
/* LOCAL FUNCTIONS              */
/* ---------------------------- */

/* ---------------------------- */
/* MISCELLANEOUS                */
/* ---------------------------- */

/**/
/* ******************************************************************** */
/*                  GLOBAL FUNCTIONS CALLED BY RTA                      */
/* ******************************************************************** */
/* ====================================================================
Function:   eips_usersys_init
Parameters: init type
Returns:    N/A

This function initialize all user system variables.
======================================================================= */
void eips_usersys_init (uint8 init_type)
{
    /* different initialization based on passed parameters (if needed) */
    switch (init_type)
    {
        /* Out of Box Initialization */
        case EIPSINIT_OUTOFBOX:
            break;

        /* Normal Initialization */
        case EIPSINIT_NORMAL:
        default:
            break;
    };
}

/* ====================================================================
Function:   eips_usersys_process
Parameters: N/A
Returns:    N/A

This function handles any user system processing.
======================================================================= */
void eips_usersys_process (void)
{
    /* do nothing */
}

/* ====================================================================
Function:   eips_usersys_reset
Parameters: N/A
Returns:    N/A

This function handles fatal errors.
======================================================================= */
int16 eips_usersys_reset (uint8 reset_type)
{
    switch(reset_type)
    {
        case EIPS_STATE_RESETNORMAL:
            return(0);

#ifdef EIPS_STATE_RESETOUTOFBOX
        case EIPS_STATE_RESETOUTOFBOX:
            return(0);
#endif

#ifdef EIPS_STATE_RESETOUTOFBOX_NOCOMM
        case EIPS_STATE_RESETOUTOFBOX_NOCOMM:
            return(0);
#endif
    };

    return(-2);
}

/* ====================================================================
Function:   eips_usersys_noreturn_reboot
Parameters: N/A
Returns:    N/A

This function triggers a hardware reset (like a watchdog or software
initiated). This function MUST NOT RETURN.
======================================================================= */
#ifdef EIPS_USERSYS_HWRESET_SUPPORTED
void eips_usersys_noreturn_reboot (void)
{
    eips_user_dbprint0("eips_usersys_noreturn_reboot\n");

    /* since we can't really reset, close all sockets to allow init to run the next time */
    eips_usersock_close_all_sockets();
    //Sleep(2000); /* sleep a little */
}
#endif

/* ====================================================================
Function:   eips_usersys_fatalError
Parameters: N/A
Returns:    N/A

This function handles fatal errors.
======================================================================= */
void eips_usersys_fatalError (char *function_name, int16 error_num)
{
    RTA_UNUSED_PARAM(function_name);
    RTA_UNUSED_PARAM(error_num);

    eips_user_dbprint2("FATAL ERROR: \"%s\" Error: %d\n",function_name, error_num);

    /* we shouldn't get here, but make sure we don't return */
    exit(error_num);
}

/* ====================================================================
Function:   eips_usersys_getIncarnationID
Parameters: N/A
Returns:    16-bit number

This function generates the Incarnation ID.
======================================================================= */
uint16 eips_usersys_getIncarnationID (void)
{
    // Init the seed used to generate random number.
    // Since the seed is IP addr specific, it will gurantee that the device in the lan will have a unique IncarID.
    srand(eips_usersock_getOurIPAddr());
    return((uint16)rand());
}

/* ====================================================================
Function:   eips_usersys_ledTest
Parameters: N/A
Returns:    N/A

   Turn first indicator Green, all other indicators off
   Leave first indicator on Green for approximately 0.25 second
   Turn first indicator on Red for approximately 0.25 second
   Turn first indicator on Green
   Turn second indicator (if present) on Green for approx. 0.25 second
   Turn second indicator (if present) on Red for approx. 0.25 second
   Turn second indicator (if present) Off

If other indicators are present, test each indicator in sequence as
prescribed by the second indicator above. If a Module Status indicator
is present, it shall be the first indicator in the sequence, followed
by any Network Status indicators present.

After completion of this power up test, the indicator(s) shall turn
to a normal operational state (all OFF).
======================================================================= */
#if EIPS_NTWK_LED_USED || EIPS_IO_LED_USED
void eips_usersys_ledTest (void)
{
	// Just initialize led, no led test here
    //LED_Struct_Init(&LED_ETHIP, LED_TYPE_GREENRED, 1, GPIOTag_LED_ETHIP_OK, GPIOTag_LED_ETHIP_FT);
    //eips_user_dbprint0("eips_usersys_ledTest\r\n");
}
#endif

/* ====================================================================
Function:   eips_usersys_nsLedUpdate
Parameters: led state (see eips_cnxn.h)
Returns:    N/A

This function controls the Network LED.
======================================================================= */
#ifdef EIPS_NTWK_LED_USED
#include "system.h"
void eips_usersys_nsLedUpdate (uint8 led_state)
{
    static uint8 first_time = 1;
    static uint8 last_state;

    /* we only want to sense changes to the LED state */
    if(first_time)
    {
        first_time = 0;

        /* force this to look like a state change */
        last_state = (uint8)(led_state+1);
    }

    /* don't do anything if the state didn't change */
    if(last_state == led_state)
        return;

    /* store the last state */
    last_state = led_state;

    /* switch on the state */
    switch(led_state)
    {
        /* switch on the valid LED states */
        case EIPS_LEDSTATE_OFF:
        	led_bicolor_setstate(&eip_led_ns, LED_BI_STATE_OFF);
        	//eip_ns_led.state = BCLEDSTATE_OFF;
            eips_user_dbprint0("NS LED: Off\r\n");
            break;
        case EIPS_LEDSTATE_FLASH_GREEN:
        	led_bicolor_setstate(&eip_led_ns, LED_BI_STATE_FLASH_GREEN);
        	//eip_ns_led.state = BCLEDSTATE_FLASH_GREEN;
            eips_user_dbprint0("NS LED: Flashing Green\r\n");
            break;
        case EIPS_LEDSTATE_FLASH_RED:
        	led_bicolor_setstate(&eip_led_ns, LED_BI_STATE_FLASH_RED);
        	//eip_ns_led.state = BCLEDSTATE_FLASH_RED;
            eips_user_dbprint0("NS LED: Flashing Red\r\n");
            break;
        case EIPS_LEDSTATE_STEADY_GREEN:
        	led_bicolor_setstate(&eip_led_ns, LED_BI_STATE_STEADY_GREEN);
        	//eip_ns_led.state = BCLEDSTATE_STEADY_GREEN;
            eips_user_dbprint0("NS LED: Steady Green\r\n");
            break;
        case EIPS_LEDSTATE_STEADY_RED:
        	led_bicolor_setstate(&eip_led_ns, LED_BI_STATE_STEADY_RED);
        	//eip_ns_led.state = BCLEDSTATE_STEADY_RED;
            eips_user_dbprint0("NS LED: Steady Red\r\n");
            break;
        case EIPS_LEDSTATE_ALT_REDGREEN:
        	led_bicolor_setstate(&eip_led_ns, LED_BI_STATE_ALT_REDGREEN);
        	//eip_ns_led.state = BCLEDSTATE_ALT_REDGREEN;
            eips_user_dbprint0("NS LED: Red/Green Toggle\r\n");
            break;

        /* error */
        default:
            eips_user_dbprint0("NS LED: ERR\r\n");
            break;
    };
}
#endif

/* ====================================================================
Function:   eips_usersys_ioLedUpdate
Parameters: led state (see eips_cnxn.h)
Returns:    N/A

This function controls the I/O LED.
======================================================================= */
#ifdef EIPS_IO_LED_USED
extern BiColorLED_TypeDef eip_io_led;
void eips_usersys_ioLedUpdate (uint8 led_state)
{
    static uint8 first_time = 1;
    static uint8 last_state;

    /* we only want to sense changes to the LED state */
    if(first_time)
    {
        first_time = 0;

        /* force this to look like a state change */
        last_state = (uint8)(led_state+1);
    }

    /* don't do anything if the state didn't change */
    if(last_state == led_state)
        return;

    /* store the last state */
    last_state = led_state;

    /* switch on the state */
    switch(led_state)
    {
        /* switch on the valid LED states */
        case EIPS_LEDSTATE_OFF:
        	eip_io_led.state = BCLEDSTATE_OFF;
            eips_user_dbprint0("IO LED: Off\r\n");
            break;
        case EIPS_LEDSTATE_FLASH_GREEN:
        	eip_io_led.state = BCLEDSTATE_FLASH_GREEN;
            eips_user_dbprint0("IO LED: Flashing Green\r\n");
            break;
        case EIPS_LEDSTATE_FLASH_RED:
        	eip_io_led.state = BCLEDSTATE_FLASH_RED;
            eips_user_dbprint0("IO LED: Flashing Red\r\n");
            break;
        case EIPS_LEDSTATE_STEADY_GREEN:
        	eip_io_led.state = BCLEDSTATE_STEADY_GREEN;
            eips_user_dbprint0("IO LED: Steady Green\r\n");
            break;
        case EIPS_LEDSTATE_STEADY_RED:
        	eip_io_led.state = BCLEDSTATE_STEADY_RED;
            eips_user_dbprint0("IO LED: Steady Red\r\n");
            break;
        case EIPS_LEDSTATE_ALT_REDGREEN:
        	eip_io_led.state = BCLEDSTATE_ALT_REDGREEN;
            eips_user_dbprint0("IO LED: Red/Green Toggle\r\n");
            break;

        /* error */
        default:
            eips_user_dbprint0("IO LED: ERR\r\n");
            break;
    };
}
#endif

/**/
/* ******************************************************************** */
/*                      LOCAL FUNCTIONS                                 */
/* ******************************************************************** */
/* ====================================================================
Function:   N/A
Parameters: N/A
Returns:    N/A

This function 
======================================================================= */

/* *********** */
/* END OF FILE */
/* *********** */
