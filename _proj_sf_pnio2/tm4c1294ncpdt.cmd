/******************************************************************************
 *
 * Default Linker Command file for the Texas Instruments TM4C1294NCPDT
 *
 * This is derived from revision 15071 of the TivaWare Library.
 *
 *****************************************************************************/

--retain=g_pfnVectors
#define APP_BASE 0x00060000
//#define APP_BASE 0x00000000

#define RAM_BASE 0x20000000
MEMORY
{
    //FLASH (RX) : origin = 0x00000000, length = 0x000D0000
    //below is the config for aio
    FLASH (RX) : origin = APP_BASE, length = 0x00040000
    SRAM (RWX) : origin = RAM_BASE, length = 0x00038000
    SRAM_STACK (RWX) : origin = 0x20038000, length = 0x00008000
}

/* The following command line options are set as part of the CCS project.    */
/* If you are building using the command line, or for some reason want to    */
/* define them here, you can uncomment and modify these lines as needed.     */
/* If you are using CCS for building, it is probably better to make any such */
/* modifications in your CCS project and leave this file alone.              */
/*                                                                           */
/* --heap_size=0                                                             */
/* --stack_size=256                                                          */
/* --library=rtsv7M4_T_le_eabi.lib                                           */

/* Section allocation in memory */
/*
SECTIONS
{
    .text :
    {
        _text = .;
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)
        _etext = .;
    } > FLASH

    .data : AT(ADDR(.text) + SIZEOF(.text))
    {
        _data = .;
        *(vtable)
        *(.data*)
        _edata = .;
    } > SRAM

    .bss :
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > SRAM

   .stack :
   {
       KEEP(*(.stack))
   } > SRAM_STACK
}
*/


SECTIONS
{
    .intvecs:   > APP_BASE//0x00000000
    .text   :   > FLASH
    .const  :   > FLASH
    .cinit  :   > FLASH
    .pinit  :   > FLASH
    .init_array : > FLASH

    .vtable :   > RAM_BASE
    .data   :   > SRAM
    .bss    :   > SRAM
    .sysmem :   > SRAM
    .stack  :   > SRAM_STACK
}

__STACK_TOP = __stack + 32768;

