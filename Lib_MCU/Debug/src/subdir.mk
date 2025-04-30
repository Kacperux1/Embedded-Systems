################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/adc.c \
../src/clkconfig.c \
../src/gpio.c \
../src/i2c.c \
../src/ssp.c \
../src/timer16.c \
../src/timer32.c \
../src/uart.c 

C_DEPS += \
./src/adc.d \
./src/clkconfig.d \
./src/gpio.d \
./src/i2c.d \
./src/ssp.d \
./src/timer16.d \
./src/timer32.d \
./src/uart.d 

OBJS += \
./src/adc.o \
./src/clkconfig.o \
./src/gpio.o \
./src/i2c.o \
./src/ssp.o \
./src/timer16.o \
./src/timer32.o \
./src/uart.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC13xx -D__CODE_RED -D__REDLIB__ -I"C:\Users\251497\Documents\Lib_MCU\inc" -I"C:\Users\251497\Documents\Lib_CMSISv1p30_LPC13xx\inc" -O0 -g3 -gdwarf-4 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m3 -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/adc.d ./src/adc.o ./src/clkconfig.d ./src/clkconfig.o ./src/gpio.d ./src/gpio.o ./src/i2c.d ./src/i2c.o ./src/ssp.d ./src/ssp.o ./src/timer16.d ./src/timer16.o ./src/timer32.d ./src/timer32.o ./src/uart.d ./src/uart.o

.PHONY: clean-src

