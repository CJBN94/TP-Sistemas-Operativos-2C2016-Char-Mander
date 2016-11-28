################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../pokedexClient.c 

OBJS += \
./pokedexClient.o 

C_DEPS += \
./pokedexClient.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -DFUSE_USE_VERSION=27 -DFILE_OFFSET_BITS=64 -I"/home/utnso/projects/tp-2016-2c-SegmentationFault/Conexiones" -include"/home/utnso/projects/tp-2016-2c-SegmentationFault/Conexiones/conexiones.c" -O0 -g3 -Wall -c -fmessage-length=0 -DFUSE_USE_VERSION=27 -D_FILE_OFFSET_BITS=64 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


