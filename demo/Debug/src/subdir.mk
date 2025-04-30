################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/cr_startup_lpc13.c \
../src/main.c 

C_DEPS += \
./src/cr_startup_lpc13.d \
./src/main.d 

OBJS += \
./src/cr_startup_lpc13.o \
./src/main.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC13xx -D__CODE_RED -D_LPCXpresso_ -D__REDLIB__ -I"C:\Users\251497\Documents\Lib_MCU\inc" -I"C:\Users\251497\Documents\Lib_EaBaseBoard\inc" -I"C:\Users\251497\Documents\Lib_CMSISv1p30_LPC13xx\inc" -O0 -g3 -gdwarf-4 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m3 -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/cr_startup_lpc13.d ./src/cr_startup_lpc13.o ./src/main.d ./src/main.o

.PHONY: clean-src

