################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
lwip-1.4.1/port/tiva-tm4c129/ethernetif.obj: D:/ABB_TI_TM4C1294Ncptd3/ABB_proj/lwip-1.4.1/port/tiva-tm4c129/ethernetif.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/include" --include_path="D:/ABB_TI_TM4C1294Ncptd3/ABB_proj/lwip-1.4.1/src/include" --include_path="D:/ABB_TI_TM4C1294Ncptd3/ABB_proj/_proj_sf_bl" --include_path="D:/ABB_TI_TM4C1294Ncptd3/ABB_proj/TivaWare" --include_path="D:/ABB_TI_TM4C1294Ncptd3/ABB_proj/TivaWare/driverlib" --include_path="D:/ABB_TI_TM4C1294Ncptd3/ABB_proj/TivaWare/inc" --include_path="D:/ABB_TI_TM4C1294Ncptd3/ABB_proj/application" --include_path="D:/ABB_TI_TM4C1294Ncptd3/ABB_proj/bsp_userconfig" --include_path="D:/ABB_TI_TM4C1294Ncptd3/ABB_proj/lwip-1.4.1" --include_path="D:/ABB_TI_TM4C1294Ncptd3/ABB_proj/lwip-1.4.1/port/tiva-tm4c129" --include_path="D:/ABB_TI_TM4C1294Ncptd3/ABB_proj/lwip-1.4.1/src/include/ipv4" --include_path="D:/ABB_TI_TM4C1294Ncptd3/ABB_proj/utilities" --include_path="D:/ABB_TI_TM4C1294Ncptd3/ABB_proj/bl_config" --include_path="D:/ABB_TI_TM4C1294Ncptd3/ABB_proj/tftp" --define=ccs="ccs" --define=PART_TM4C1294NCPDT -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="lwip-1.4.1/port/tiva-tm4c129/ethernetif.d_raw" --obj_directory="lwip-1.4.1/port/tiva-tm4c129" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


