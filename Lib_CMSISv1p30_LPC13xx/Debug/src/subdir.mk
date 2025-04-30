################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/core_cm3.c \
../src/system_LPC13xx.c 

C_DEPS += \
./src/core_cm3.d \
./src/system_LPC13xx.d 

OBJS += \
./src/core_cm3.o \
./src/system_LPC13xx.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__CODE_RED -D__REDLIB__ -I"C:\Users\251497\Documents\Lib_CMSISv1p30_LPC13xx\inc" -O1 -g3 -gdwarf-4 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m3 -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/core_cm3.d ./src/core_cm3.o ./src/system_LPC13xx.d ./src/system_LPC13xx.o

.PHONY: clean-src

