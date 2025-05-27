################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := Dekoder
BUILD_ARTIFACT_EXTENSION := axf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all:
	+@$(MAKE) --no-print-directory main-build && $(MAKE) --no-print-directory post-build

# Main-build Target
main-build: Dekoder.axf

# Tool invocations
Dekoder.axf: $(OBJS) $(USER_OBJS) makefile $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"C:\Users\251497\Documents\Lib_CMSISv1p30_LPC13xx\Debug" -L"C:\Users\251497\Documents\Lib_MCU\Debug" -L"C:\Users\251497\Documents\Lib_EaBaseBoard\Debug" -Xlinker --gc-sections -Xlinker -Map=Dekoder.map -mcpu=cortex-m3 -mthumb -T Dekoder_Debug.ld -o "Dekoder.axf" $(OBJS) $(USER_OBJS) $(LIBS) -lLib_EaBaseBoard -lLib_MCU -lLib_CMSISv1p30_LPC13xx
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) Dekoder.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size Dekoder.axf; # arm-none-eabi-objcopy -O ihex Dekoder.axf Dekoder.hex ;
	-@echo ' '

.PHONY: all clean dependents main-build post-build

-include ../makefile.targets
