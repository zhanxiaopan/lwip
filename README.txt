The project can make implement the function that detect the realtime flowrate leakage ,
and it can also debug with eips instead of transfer the bin(make by eips project)
through the tool tftp.exe to bootloader.so if you wanna debug or excute it directly,
you can follow the readme and get the information and modify the macro as follows 
1: (sys_config.h)
 #define FIELDBUS_TYPE_NONE (0)
 #define FIELDBUS_TYPE_EIPS(1) 
 #define FIELDBUS_TYPE_PNIO (2)
 #define FIELDBUS_TYPE_BL (3) 
 #define WS_FIELDBUS_TYPE FIELDBUS_TYPE_EIPS 
 modify WS_FIELDBUS_TYPE as FIELDBUS_TYPE_BL，it excute mode ，
 FIELDBUS_TYPE_EIPS is debug mode instead

2: (tm4c1294ncpdt.cmd) 
num 12: APP_BASE:0x00000000 debug mode 
APP_BASE:0x00020000 excute mode


notice:this version is for eips debug
