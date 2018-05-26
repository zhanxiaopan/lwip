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
 *     Module Name: eips_timers.c
 *          Author: Jamin D. Wendorf (jwendorf@rtaautomation.com)
 *        Language: Ansi C
 * Compile Options: N/A
 * Compile defines: N/A
 *       Libraries: N/A
 *    Link Options: N/A
 *
 * Description.
 * =======================================================================
 * This file contains the definitions needed for timer processing.
 *
 * The resolution of timers is based on "EIPS_USER_TICK_RES_IN_USECS".  
 * This file doesn't care what the resolution is.  To simplify the processing,
 * a static "link list" is used..
 *
 */

/* ---------------------------- */
/* INCLUDE FILES                */
/* ---------------------------- */
#include "eips_system.h"

/* used for debug */
#undef  __RTA_FILE__
#define __RTA_FILE__ "eips_timer.c"

/* ---------------------------- */
/* FUNCTION PROTOTYPES          */
/* ---------------------------- */
void EIPS_UpdateTimerIxTable (void);
int16 timertest = 0;
/* ---------------------------- */
/* STATIC VARIABLES             */
/* ---------------------------- */

EIPS_TIMER_STRUCT EIPS_Timers[RTA_MAX_NUM_TIMERS];
uint16 EIPS_UsedTimerIx[RTA_MAX_NUM_TIMERS];
uint16 EIPS_NumTimerIxUsed;

/* ---------------------------- */
/* MISCELLANEOUS                */
/* ---------------------------- */
#define EIPS_TIM_STATE_STOPPED 0
#define EIPS_TIM_STATE_RUNNING 1
#define EIPS_TIM_STATE_TIMEOUT 2

#ifdef EIPS_TIMER_NAME_USED
    #define TIMER_DB_PRINT(f,tn,ts,o) {eips_user_dbprint4("%s - %d (%s) (%d)\r\n",f,tn,ts,o);}
#else
    #define TIMER_DB_PRINT(f,tn,ts,o)
#endif

/**/
/* ******************************************************************** */
/*                      GLOBAL FUNCTIONS                                */
/* ******************************************************************** */
/* ====================================================================
Function:   eips_timer_init
Parameters: init type
Returns:    N/A

This function initializes static variables used by this file.
======================================================================= */
void eips_timer_init (uint8 init_type)
{
    uint16 i;

    RTA_UNUSED_PARAM(init_type);

    /* initialize all the timers */
    for(i=0; i<RTA_MAX_NUM_TIMERS; i++)
    {
        eips_timer_free(i);
    }

    /* keep track of how many timers are used */
    EIPS_NumTimerIxUsed = 0;
}

/* ====================================================================
Function:   eips_timer_new
Parameters: N/A
Returns:    N/A

This function searches for the next unused timer.  If all the timers
are used, EIPS_TIMER_NULL is returned.
======================================================================= */
uint16 eips_timer_new (void)
{
    uint16 i;

    /* Make sure we have free timers */
    if(EIPS_NumTimerIxUsed == RTA_MAX_NUM_TIMERS)
        return(EIPS_TIMER_NULL);

    /* search through all timers for a free one */
    for(i=1; i<RTA_MAX_NUM_TIMERS; i++)
    {
        /* we found a free timer, use it */
        if(EIPS_Timers[i].owned == FALSE)
        {
            /* reset the timer values to 0 to start */
            eips_timer_free(i);

            /* mark the timer as used */
            EIPS_Timers[i].owned = TRUE;
         
            /* update the table of used indexes */
            EIPS_UpdateTimerIxTable();

            /* return the timer number */
            return(i);
        }
    }

    /* no timers are free */
    return(EIPS_TIMER_NULL);
}

/* ====================================================================
Function:   eips_timer_free
Parameters: timer number
Returns:    N/A

This function validates the timer number, then frees the timer to
unused.
======================================================================= */
void eips_timer_free (uint16 timer_num)
{
    /* make sure the timer number isn't EIPS_TIMER_NULL */
    if(!timer_num || timer_num == EIPS_TIMER_NULL)
        return;
    
    /* validate timer_num is in range */
    if(timer_num >= RTA_MAX_NUM_TIMERS)
        return;

    /* free the timer */
    EIPS_Timers[timer_num].owned = FALSE;
    EIPS_Timers[timer_num].state = EIPS_TIM_STATE_STOPPED;
    EIPS_Timers[timer_num].timer_data.current_value = 0;
    EIPS_Timers[timer_num].timer_data.reload_value  = 0;
    EIPS_Timers[timer_num].timer_data.timeout_flag_ptr = NULL;
    EIPS_Timers[timer_num].timer_data.timeout_func_ptr = NULL;
    EIPS_Timers[timer_num].timer_data.custom_func_arg = NULL;
    EIPS_Timers[timer_num].timer_data.tmo_custom_func_ptr = NULL;    
#ifdef EIPS_TIMER_NAME_USED
    EIPS_Timers[timer_num].timer_data.timer_name[0] = 0;
#endif
}

/* ====================================================================
Function:   eips_timer_reset
Parameters: timer number
Returns:    N/A

This function validates the timer number, then sets the current value
to the reload value.
======================================================================= */
void eips_timer_reset (uint16 timer_num)
{
    /* make sure the timer number isn't EIPS_TIMER_NULL */
    if(!timer_num || timer_num == EIPS_TIMER_NULL)
        return;
    
    /* validate timer_num is in range */
    if(timer_num >= RTA_MAX_NUM_TIMERS)
        return;

TIMER_DB_PRINT("eips_timer_reset", timer_num, EIPS_Timers[timer_num].timer_data.timer_name, 0);

    /* only reset the timer if it is owned */
    if(EIPS_Timers[timer_num].owned == TRUE)
    {
        /* set the current value to the reload value */
        EIPS_Timers[timer_num].timer_data.current_value = EIPS_Timers[timer_num].timer_data.reload_value;
		if (timer_num == 31) timertest = EIPS_Timers[timer_num].timer_data.current_value;
        /* set the state back to running */
        EIPS_Timers[timer_num].state = EIPS_TIM_STATE_RUNNING;
    }

    /* if the current time is 0, stop the timer */
    if(!EIPS_Timers[timer_num].timer_data.current_value)
        eips_timer_stop(timer_num);
}

/* ====================================================================
Function:    eips_timer_newval
Parameters:  timer number
             new timeout value
Returns:     N/A

This function sets the timeout value to the passed value.
======================================================================= */
void eips_timer_newval (uint16 timer_num, uint32 newval)
{
   /* validate timer_num is in range */
   if(!timer_num || timer_num >= RTA_MAX_NUM_TIMERS)
      return;

TIMER_DB_PRINT("eips_timer_newval", timer_num, EIPS_Timers[timer_num].timer_data.timer_name, newval);

   /* only start the timer if it is owned */
   if(EIPS_Timers[timer_num].owned == TRUE)
   {
      /* set the current value to the reload value */
      EIPS_Timers[timer_num].timer_data.current_value = newval;
      EIPS_Timers[timer_num].timer_data.reload_value  = newval;
   }

    /* if the current time is 0, stop the timer */
    if(!EIPS_Timers[timer_num].timer_data.current_value)
        eips_timer_stop(timer_num);
}

/* ====================================================================
Function:   eips_timer_start
Parameters: timer number
Returns:    N/A

This function validates the timer number, then starts the timer.
======================================================================= */
void eips_timer_start (uint16 timer_num)
{
    /* make sure the timer number isn't EIPS_TIMER_NULL */
    if(!timer_num || timer_num == EIPS_TIMER_NULL)
        return;
    
    /* validate timer_num is in range */
    if(timer_num >= RTA_MAX_NUM_TIMERS)
        return;

TIMER_DB_PRINT("eips_timer_start", timer_num, EIPS_Timers[timer_num].timer_data.timer_name, 0);

    /* only start the timer if it is owned */
    if(EIPS_Timers[timer_num].owned == TRUE)
    {
        /* set the state back to running */
        EIPS_Timers[timer_num].state = EIPS_TIM_STATE_RUNNING;
    }

    /* if the current time is 0, stop the timer */
    if(!EIPS_Timers[timer_num].timer_data.current_value)
        eips_timer_stop(timer_num);
}

/* ====================================================================
Function:   eips_timer_stop
Parameters: timer number
Returns:    N/A

This function validates the timer number, then stops the timer.
======================================================================= */
void eips_timer_stop (uint16 timer_num)
{
    /* make sure the timer number isn't EIPS_TIMER_NULL */
    if(!timer_num || timer_num == EIPS_TIMER_NULL)
        return;
    
    /* validate timer_num is in range */
    if(timer_num >= RTA_MAX_NUM_TIMERS)
        return;

TIMER_DB_PRINT("eips_timer_stop", timer_num, EIPS_Timers[timer_num].timer_data.timer_name, 0);

    /* only start the timer if it is owned */
    if(EIPS_Timers[timer_num].owned == TRUE)
    {
        /* set the state to stopped */
        EIPS_Timers[timer_num].state = EIPS_TIM_STATE_STOPPED;
    }
}

/* ====================================================================
Function:   eips_timer_running
Parameters: timer number
Returns:    TRUE  - timer is running
            FALSE - timer isn't running

This function validates the timer number and returns TRUE if the timer
is running.
======================================================================= */
uint8 eips_timer_running (uint16 timer_num)
{
    /* make sure the timer number isn't EIPS_TIMER_NULL */
    if(!timer_num || timer_num == EIPS_TIMER_NULL)
        return(FALSE);
    
    /* validate timer_num is in range */
    if(timer_num >= RTA_MAX_NUM_TIMERS)
        return(FALSE);

    /* only start the timer if it is owned */
    if( (EIPS_Timers[timer_num].owned == TRUE) &&
        (EIPS_Timers[timer_num].state != EIPS_TIM_STATE_STOPPED))
    {
        /* we are running */
        return(TRUE);
    }

TIMER_DB_PRINT("eips_timer_running", timer_num, EIPS_Timers[timer_num].timer_data.timer_name, 0);

    return(FALSE);
}

/* ====================================================================
Function:   eips_timer_changeValues
Parameters: timer number
            new reload value
Returns:    N/A

This function validates the timer number, then changes the reload value.
======================================================================= */
void eips_timer_changeValues (uint16 timer_num, uint32 current, uint32 reload)
{
    /* make sure the timer number isn't EIPS_TIMER_NULL */
    if(!timer_num || timer_num == EIPS_TIMER_NULL)
        return;
    
    /* validate timer_num is in range */
    if(timer_num >= RTA_MAX_NUM_TIMERS)
        return;

TIMER_DB_PRINT("eips_timer_changeValues", timer_num, EIPS_Timers[timer_num].timer_data.timer_name, current);

    /* only change the values if the timer if it is owned */
    if(EIPS_Timers[timer_num].owned == TRUE)
    {
        EIPS_Timers[timer_num].timer_data.current_value = current;
        EIPS_Timers[timer_num].timer_data.reload_value  = reload;
    }

    /* if the current time is 0, stop the timer */
    if(!current)
        eips_timer_stop(timer_num);
}

/* ====================================================================
Function:   eips_timer_get_struct
Parameters: timer number
Returns:    pointer to the timer data element, NULL on error

This function validates the timer number is valid and owned, then
returns the timer structure.
======================================================================= */
EIPS_TIMER_DATA_STRUCT * eips_timer_get_struct (uint16 timer_num)
{
    /* make sure the timer number isn't EIPS_TIMER_NULL */
    if(!timer_num || timer_num == EIPS_TIMER_NULL)
        return(NULL);
   
    /* validate timer_num is in range */
    if(timer_num >= RTA_MAX_NUM_TIMERS)
        return(NULL);

    /* only start the timer if it is owned */
    if(EIPS_Timers[timer_num].owned == TRUE)
    {
        /* set the state back to running */
        return(&EIPS_Timers[timer_num].timer_data);
    }

    /* if the timer isn't owned, return NULL */
    return(NULL);
}

/* ====================================================================
Function:   eips_timer_process
Parameters: N/A
Returns:    N/A

This function processes all timers.  Flags are incremented on timeout.
Function pointers are called on timeouts.
======================================================================= */
void eips_timer_process (uint32 ticks_passed)
{
    uint16 i, timer_ix;

    /* if ticks_passed is 0, return */
    if(ticks_passed == 0)
        return;
        
    /* only process timers in our lookup table */
    for(i=0; i<EIPS_NumTimerIxUsed; i++)
    {
        /* get the next timer index to access */
        timer_ix = EIPS_UsedTimerIx[i];

        /* only check the timer if it is running */
        if(EIPS_Timers[timer_ix].state == EIPS_TIM_STATE_RUNNING)
        {
            /* see if ticks_passed causes us to transition to timed out */
            if(ticks_passed >= EIPS_Timers[timer_ix].timer_data.current_value)
            {
                /* transition to timed out */
                EIPS_Timers[timer_ix].state = EIPS_TIM_STATE_TIMEOUT;

                /* set the current value to 0 */
                EIPS_Timers[timer_ix].timer_data.current_value = 0;

                /* if available, increment the timeout counter */
                if(EIPS_Timers[timer_ix].timer_data.timeout_flag_ptr != NULL)
                {
                    (*EIPS_Timers[timer_ix].timer_data.timeout_flag_ptr)++;
                }
 
                /* if available call the timeout function pointer */
                if(EIPS_Timers[timer_ix].timer_data.timeout_func_ptr != NULL)
                {
                    (*EIPS_Timers[timer_ix].timer_data.timeout_func_ptr)(timer_ix);
                }

                /* if custom function is available */
                if(EIPS_Timers[timer_ix].timer_data.tmo_custom_func_ptr != NULL)
                {
                    (*EIPS_Timers[timer_ix].timer_data.tmo_custom_func_ptr)(timer_ix, EIPS_Timers[timer_ix].timer_data.custom_func_arg);
                }
            }                                   
    
            /* we didn't timeout, decrement our current count */
            else
            {
                EIPS_Timers[EIPS_UsedTimerIx[i]].timer_data.current_value -= ticks_passed;
				if (i==31) timertest++;
            }
        }
    }
}

/**/
/* ******************************************************************** */
/*                      LOCAL FUNCTIONS                                 */
/* ******************************************************************** */
/* ====================================================================
Function:   EIPS_UpdateTimerIxTable
Parameters: N/A
Returns:    N/A

A table of used indexes is implemented to shorten the process time for
timers.  Only used indexes are updated in the timer process.
======================================================================= */
void EIPS_UpdateTimerIxTable (void)
{
    uint16 i;

    /* if a timer is used, add it to the table of used indexes */
    for(i=1, EIPS_NumTimerIxUsed = 0; i<RTA_MAX_NUM_TIMERS; i++)
    {
        /* if a timer is owned, add the index to the table */
        if(EIPS_Timers[i].owned == TRUE)
        {
            /* add the index to the lookup table */
            EIPS_UsedTimerIx[EIPS_NumTimerIxUsed] = i;

            /* increment the number of used indexes */
            EIPS_NumTimerIxUsed++;
        }
    }
}

/* *********** */
/* END OF FILE */
/* *********** */
