################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/adc_ad7766.c \
../Core/Src/adc_utility.c \
../Core/Src/calculate_ph.c \
../Core/Src/crc.c \
../Core/Src/crc_util.c \
../Core/Src/dac8562.c \
../Core/Src/data_smoothing.c \
../Core/Src/eeprom.c \
../Core/Src/eeprom_M95080.c \
../Core/Src/init.c \
../Core/Src/ioport.c \
../Core/Src/main.c \
../Core/Src/qaqc.c \
../Core/Src/salinity_conductivity.c \
../Core/Src/scan.c \
../Core/Src/stm32g0xx_hal_msp.c \
../Core/Src/stm32g0xx_it.c \
../Core/Src/subsystem.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32g0xx.c \
../Core/Src/tasktable_init.c \
../Core/Src/tasktable_master.c \
../Core/Src/temperature.c \
../Core/Src/timer.c \
../Core/Src/usart_commsboard.c 

OBJS += \
./Core/Src/adc_ad7766.o \
./Core/Src/adc_utility.o \
./Core/Src/calculate_ph.o \
./Core/Src/crc.o \
./Core/Src/crc_util.o \
./Core/Src/dac8562.o \
./Core/Src/data_smoothing.o \
./Core/Src/eeprom.o \
./Core/Src/eeprom_M95080.o \
./Core/Src/init.o \
./Core/Src/ioport.o \
./Core/Src/main.o \
./Core/Src/qaqc.o \
./Core/Src/salinity_conductivity.o \
./Core/Src/scan.o \
./Core/Src/stm32g0xx_hal_msp.o \
./Core/Src/stm32g0xx_it.o \
./Core/Src/subsystem.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32g0xx.o \
./Core/Src/tasktable_init.o \
./Core/Src/tasktable_master.o \
./Core/Src/temperature.o \
./Core/Src/timer.o \
./Core/Src/usart_commsboard.o 

C_DEPS += \
./Core/Src/adc_ad7766.d \
./Core/Src/adc_utility.d \
./Core/Src/calculate_ph.d \
./Core/Src/crc.d \
./Core/Src/crc_util.d \
./Core/Src/dac8562.d \
./Core/Src/data_smoothing.d \
./Core/Src/eeprom.d \
./Core/Src/eeprom_M95080.d \
./Core/Src/init.d \
./Core/Src/ioport.d \
./Core/Src/main.d \
./Core/Src/qaqc.d \
./Core/Src/salinity_conductivity.d \
./Core/Src/scan.d \
./Core/Src/stm32g0xx_hal_msp.d \
./Core/Src/stm32g0xx_it.d \
./Core/Src/subsystem.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32g0xx.d \
./Core/Src/tasktable_init.d \
./Core/Src/tasktable_master.d \
./Core/Src/temperature.d \
./Core/Src/timer.d \
./Core/Src/usart_commsboard.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G071xx -c -I../Core/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/adc_ad7766.cyclo ./Core/Src/adc_ad7766.d ./Core/Src/adc_ad7766.o ./Core/Src/adc_ad7766.su ./Core/Src/adc_utility.cyclo ./Core/Src/adc_utility.d ./Core/Src/adc_utility.o ./Core/Src/adc_utility.su ./Core/Src/calculate_ph.cyclo ./Core/Src/calculate_ph.d ./Core/Src/calculate_ph.o ./Core/Src/calculate_ph.su ./Core/Src/crc.cyclo ./Core/Src/crc.d ./Core/Src/crc.o ./Core/Src/crc.su ./Core/Src/crc_util.cyclo ./Core/Src/crc_util.d ./Core/Src/crc_util.o ./Core/Src/crc_util.su ./Core/Src/dac8562.cyclo ./Core/Src/dac8562.d ./Core/Src/dac8562.o ./Core/Src/dac8562.su ./Core/Src/data_smoothing.cyclo ./Core/Src/data_smoothing.d ./Core/Src/data_smoothing.o ./Core/Src/data_smoothing.su ./Core/Src/eeprom.cyclo ./Core/Src/eeprom.d ./Core/Src/eeprom.o ./Core/Src/eeprom.su ./Core/Src/eeprom_M95080.cyclo ./Core/Src/eeprom_M95080.d ./Core/Src/eeprom_M95080.o ./Core/Src/eeprom_M95080.su ./Core/Src/init.cyclo ./Core/Src/init.d ./Core/Src/init.o ./Core/Src/init.su ./Core/Src/ioport.cyclo ./Core/Src/ioport.d ./Core/Src/ioport.o ./Core/Src/ioport.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/qaqc.cyclo ./Core/Src/qaqc.d ./Core/Src/qaqc.o ./Core/Src/qaqc.su ./Core/Src/salinity_conductivity.cyclo ./Core/Src/salinity_conductivity.d ./Core/Src/salinity_conductivity.o ./Core/Src/salinity_conductivity.su ./Core/Src/scan.cyclo ./Core/Src/scan.d ./Core/Src/scan.o ./Core/Src/scan.su ./Core/Src/stm32g0xx_hal_msp.cyclo ./Core/Src/stm32g0xx_hal_msp.d ./Core/Src/stm32g0xx_hal_msp.o ./Core/Src/stm32g0xx_hal_msp.su ./Core/Src/stm32g0xx_it.cyclo ./Core/Src/stm32g0xx_it.d ./Core/Src/stm32g0xx_it.o ./Core/Src/stm32g0xx_it.su ./Core/Src/subsystem.cyclo ./Core/Src/subsystem.d ./Core/Src/subsystem.o ./Core/Src/subsystem.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32g0xx.cyclo ./Core/Src/system_stm32g0xx.d ./Core/Src/system_stm32g0xx.o ./Core/Src/system_stm32g0xx.su ./Core/Src/tasktable_init.cyclo ./Core/Src/tasktable_init.d ./Core/Src/tasktable_init.o ./Core/Src/tasktable_init.su ./Core/Src/tasktable_master.cyclo ./Core/Src/tasktable_master.d ./Core/Src/tasktable_master.o ./Core/Src/tasktable_master.su ./Core/Src/temperature.cyclo ./Core/Src/temperature.d ./Core/Src/temperature.o ./Core/Src/temperature.su ./Core/Src/timer.cyclo ./Core/Src/timer.d ./Core/Src/timer.o ./Core/Src/timer.su ./Core/Src/usart_commsboard.cyclo ./Core/Src/usart_commsboard.d ./Core/Src/usart_commsboard.o ./Core/Src/usart_commsboard.su

.PHONY: clean-Core-2f-Src

