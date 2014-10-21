################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/LSM303.cpp \
../src/cr_cpp_config.cpp \
../src/cr_startup_lpc407x_8x.cpp \
../src/main.cpp 

C_SRCS += \
../src/crp.c \
../src/sysinit.c 

OBJS += \
./src/LSM303.o \
./src/cr_cpp_config.o \
./src/cr_startup_lpc407x_8x.o \
./src/crp.o \
./src/main.o \
./src/sysinit.o 

C_DEPS += \
./src/crp.d \
./src/sysinit.d 

CPP_DEPS += \
./src/LSM303.d \
./src/cr_cpp_config.d \
./src/cr_startup_lpc407x_8x.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -D__NEWLIB__ -DDEBUG -D__CODE_RED -DCORE_M4 -D__USE_LPCOPEN -DCPP_USE_HEAP -D__LPC407X_8X__ -I"/Users/patdag/workspace/lpcxpresso_4088_3/lpc_board_ea_devkit_4088/inc" -I"/Users/patdag/workspace/lpcxpresso_4088_3/lpc_chip_40xx/inc" -Og -g3 -Wall -std=c++11 -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fsingle-precision-constant -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__NEWLIB__ -DDEBUG -D__CODE_RED -DCORE_M4 -D__USE_LPCOPEN -DCPP_USE_HEAP -D__LPC407X_8X__ -I"/Users/patdag/workspace/lpcxpresso_4088_3/lpc_board_ea_devkit_4088/inc" -I"/Users/patdag/workspace/lpcxpresso_4088_3/lpc_chip_40xx/inc" -Og -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fsingle-precision-constant -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


