/*
 * upgrader.c
 *
 *  Created on: Dec 16, 2016
 *      Author: CNWELI4
 */
#include "upgrader.h"
#include "bsp_GPIO.h"
#include "inc/hw_types.h"
#include "inc/hw_flash.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_memmap.h"
#include "hw_nvic.h"
#include "bsp_eeprom_const.h"
#include "reset.h"
#include <stdio.h>

#include "aio_config.h"

//#define UPGRADER_APP_START_ADDR (0x20000)
#define UPGRADER_DL_BUF_START_ADDR (0xA0000)
#ifdef __USE_LAUNCH_PAD
// we define UERLED1 (PNO) as BL indicator
#define BL_INDICATOR GPIOTag_OB_LED_2
#else /* __USE_LAUNCH_PAD */
#endif /* __USE_LAUNCH_PAD */

#define LOOP_PERIOD (100)

#ifndef USE_FILE_SYSTEM_IN_TFTP
uint32_t g_ui32TransferAddress = AIO_EIPS_VTABLE_ADDR;
uint32_t g_ui32DownloadBuffAddr = UPGRADER_DL_BUF_START_ADDR;
uint32_t g_ui32WritingPosition = 0;		// indicate the next byte position to be programmed. this could jump during downloading and copying stages.
uint8_t g_ui8DownloadError = 0;
uint8_t g_ui8IsDownloadDone = 0;
uint8_t g_ui8IsWrqReceived = 0;
uint32_t g_ui32DownloadedSize = 0;
uint8_t g_ui8FlashError = 0;
#endif

FW_UPGRADER_T g_upgrader;
TFTP_ERROR_LIST g_error;

#define JUMP_WAITING_TIME_LONG      1200        //5 min
#define JUMP_WAITING_TIME_SHORT     40          //10s
#define JUMP_WAITING_TIME_NO        1           // 250ms
#define JUMP_WAITING_TIME_FOREVER   4000000000  // several monthes

uint32_t    jump_config;
uint32_t    WaitingTime = JUMP_WAITING_TIME_LONG;  //5min

FW_UPGRADER_STATUS_T upgrader_read_status_eeprom ()
{
	return UPGRADER_IDLE;
}

void upgrader_write_status_eeprom ()
{

}

void upgrader_init(FW_UPGRADER_T *upgrader)
{
    /*
     * code above deprecated by Tms on 06.12.2017
     */
    if(aio_bl_config == AIO_BL_LOAD_EIPS || aio_bl_config == AIO_BL_LOAD_PNIO)
        WaitingTime == JUMP_WAITING_TIME_LONG;
    else
    {
        //only enter this state when it is not config
        //to redirect
        //the redirection should be implemented in "main.c"
        printf("Bootloader configuration error!@2");
        while(1);
    }

	upgrader->status = UPGRADER_IDLE;
}

// blink a LED here to imply the ws is waiting for a TFTP WRQ.
// the led is simpley toggled here. the timing should be controlled by the programmer.
void upgrader_blink_indicator_led () {
#ifdef __USE_LAUNCH_PAD
	GPIO_TagToggle(BL_INDICATOR);
#else /* __USE_LAUNCH_PAD */
	GPIO_TagToggle(BL_INDICATOR);
#endif /* __USE_LAUNCH_PAD */

}

/**
 *  @brief Brief
 *  
 *  @param [in] upgrader Parameter_Description
 *  @return Return_Description
 *  
 *  @details Details
 */
void upgrader_process ()
{
	uint32_t i = 0;
	static uint32_t loop_tick_1 = 0;
    static uint32_t loop_tick_2 = 0;
    GPIO_TagWrite(GPIOTag_LED_2_RED, 1);
 //   GPIO_TagWrite(GPIOTag_LED_2_GRN, 0);
	switch (g_upgrader.status) {
	case UPGRADER_IDLE:
		loop_tick_1++;
		if (g_ui8IsWrqReceived == 1) {
			g_upgrader.status = REQUESTED;
		}
		else if (loop_tick_2 < WaitingTime) {
			// blink the LED here.
            // toggle the LED every 250ms
			if (loop_tick_1 >= (250/UPGRADER_PROCESS_INTERVAL)) {
                loop_tick_2++;
                loop_tick_1 = 0;
                upgrader_blink_indicator_led();
            }
		}
		else {
			// timeout when waiting for TFTP WRQ!
			g_upgrader.status = UPGRADER_TIMEOUT;
			//todo: redirect or something!
		}
		break;
	case UPGRADER_TIMEOUT:
		// timeout so we go directly to jump.
		g_upgrader.status = READY_TO_JUMP;
		puts("Upgrader_timeout");

		//roll back to previous setup for redirect
		aio_rollback();

		break;
	case REQUESTED:
		// requested also means we're in downloading status.
		if (g_ui8IsDownloadDone == 1) {
			g_upgrader.status = DOWNLOADED;
		}
		break;
	case DOWNLOADED:
		// download completed.
		// todo: integrity verification required here.
		// start copying from download buf to main_app_buf
		// init g_ui32WritingPosition to main_app_buf firstly and then go to next state.
	    /*
	     * below added by Tms on 06.12.2017
	     */
	    if(aio_bl_config == AIO_BL_LOAD_EIPS)
	        g_ui32TransferAddress = AIO_EIPS_VTABLE_ADDR;
	    else if(aio_bl_config == AIO_BL_LOAD_PNIO)
	        g_ui32TransferAddress = AIO_PNIO_VTABLE_ADDR;
	    else
	    {
	        //state error, no other value should show
	        //up here!
	        printf("Bootloader configuration error!@3");
	        while(1);
	    }

		g_ui32WritingPosition = g_ui32TransferAddress;
		g_upgrader.status = FLASH_COPPYING;
		break;
	case FLASH_COPPYING:
		// store the status into eeprom

		// erase the main_app_buf

	    //TODO: calculate the size!
		for (i=0; i<256; i++) {
		  if (FlashErase(g_ui32TransferAddress + i * 1024) != 0) {
			// report the error.
			g_ui8FlashError = 1;
			g_upgrader.status = UPGRADER_ERROR;
			g_error = FLASHERASE_FAILED;
		  }
		}

		// start copying here!
		if (FlashProgram((uint32_t *)g_ui32DownloadBuffAddr, g_ui32WritingPosition, g_ui32DownloadedSize) != 0) {
			// flash programming error here, report the error.
			g_ui8FlashError = 1;
			g_upgrader.status = UPGRADER_ERROR;
			g_error = FLASHCOPY_FAILED;
		}
		else {
			g_upgrader.status = FLASH_COPPIED;
		}
		break;
	case FLASH_COPPIED:
		//todo check the integrity of coppied image.

		// now it's ready to jump.
		g_upgrader.status = READY_TO_JUMP;

		// update the aio_bin_exist_flag
		if(aio_bin_exist_flag != AIO_BIN_BOTH)
		    aio_bin_exist_flag++;
		else
            aio_bl_config = AIO_BL_REDI;
		aio_writeConfig();

		//config the waiting to be "no_wait"
		jump_config = JUMP_WAITING_TIME_NO;
		EEPROMProgram ((uint32_t*)&jump_config, EEPROM_ADDR_BL_JUMP_CONFIG, EEPROM_LEN_BL_JUMP_CONFIG);
	    GPIO_TagWrite(GPIOTag_LED_2_RED, 1);
		puts("Jumping");
		break;
	case READY_TO_JUMP:
//		 reset and disable the peripherals used by the boot loader.
		SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOA);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOB);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOC);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOD);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOE);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOF);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOG);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOH);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOJ);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOK);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOL);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOM);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_GPION);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOP);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOQ);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOR);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOS);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOT);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_EEPROM0);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_EMAC0);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_EPHY0);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_ADC0);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_UART1);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_ADC1);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_PWM0);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_TIMER0);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_TIMER1);
		SysCtlPeripheralDisable(SYSCTL_PERIPH_TIMER2);
		SysTickIntDisable();
		IntMasterDisable();
		/*
		 * Branch to the specified address.  This should never return.
		 * If it does, very bad things will likely happen since it is
		 * likely that the copy of the boot loader in SRAM will have
		 * been overwritten.
		 * */
	    //
	    // Set the vector table to the beginning of the app in flash.
	    //

		//this position should not be reached in all-in-one
		resetInit();
		resetLaunch();  // should be redirect to 0x20000 after reset

//	    HWREG(NVIC_VTABLE) = g_ui32TransferAddress;
//
//	    //
//	    // Load the stack pointer from the application's vector table.
//	    //
//	    __asm("    ldr     r1, [r0]\n"
//	          "    mov     sp, r1\n");
//
//	    //
//	    // Load the initial PC from the application's vector table and branch to
//	    // the application's entry point.
//	    //
//	    __asm("    ldr     r0, [r0, #4]\n"
//	          "    bx      r0\n");

		/*
		 * In case this ever does return and the boot loader is still
		 * intact, simply reset the device.
		 * */
	    HWREG(NVIC_APINT) = (NVIC_APINT_VECTKEY |
	                         NVIC_APINT_SYSRESETREQ);

		/*
		 * The microcontroller should have reset, so this should
		 * never be .  Just in case, restart the MCU again.
		 * */
	    puts("jumping failed");
	    HWREG(NVIC_APINT) = (NVIC_APINT_VECTKEY |
	                         NVIC_APINT_SYSRESETREQ);
	    break;
	case UPGRADER_ERROR:
	default:
		// we handle here both UPGRADER_ERROR and not-defined state as error.

		// stop the upgrade and take actions accordingly.
		// simply reset the MCU here to retry.
	    puts(g_error);
	    HWREG(NVIC_APINT) = (NVIC_APINT_VECTKEY |
	                         NVIC_APINT_SYSRESETREQ);
		break;
	}
}

//void tftp_upgrade ()
//{
//	// check rrq request
//	uint32_t start_time_stamp;
//
//	start_time_stamp = Time_GetMs();
//	while ( (Time_GetMs() - start_time_stamp) < 10000 && g_ui8IsWrqReceived == 0) {
//		// blink the LED to indicate ws is waiting for a new fw put.
//		if (Time_GetMs()%500 == 0) {
//			// toggle the LED.
//		}
//	}
//
//	// turn off the LED.
//
//	// timeout?
//	if (g_ui8IsWrqReceived == 0) {
//		goto JUMP;
//	} else {
//
//	}
//
//	// waiting for the downloading complete.
//
//
//
//JUMP:
//	// reset and disable the peripherals used by the boot loader.
//
//	/*
//	 * Branch to the specified address.  This should never return.
//	 * If it does, very bad things will likely happen since it is
//	 * likely that the copy of the boot loader in SRAM will have
//	 * been overwritten.
//	 * */
//	//((void (*)(void))g_ui32TransferAddress)();
//    //
//    // Set the vector table to the beginning of the app in flash.
//    //
//    HWREG(NVIC_VTABLE) = g_ui32TransferAddress;
//
//    //
//    // Load the stack pointer from the application's vector table.
//    //
//    __asm("    ldr     r1, [r0]\n"
//          "    mov     sp, r1\n");
//
//    //
//    // Load the initial PC from the application's vector table and branch to
//    // the application's entry point.
//    //
//    __asm("    ldr     r0, [r0, #4]\n"
//          "    bx      r0\n");
//
//	/*
//	 * In case this ever does return and the boot loader is still
//	 * intact, simply reset the device.
//	 * */
//    HWREG(NVIC_APINT) = (NVIC_APINT_VECTKEY |
//                         NVIC_APINT_SYSRESETREQ);
//
//	/*
//	 * The microcontroller should have reset, so this should
//	 * never be .  Just in case, loop forever.
//	 * */
//    while(1)
//    {
//    }
//}
