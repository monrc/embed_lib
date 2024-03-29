######################################################################################################################
# File automatically-generated by tool: [projectgenerator] version: [3.10.0-B14] date: [Sun Apr 03 10:05:00 CST 2022] 
######################################################################################################################

# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2015-07-22 - first version
#	2017-02-10 - Several enhancements + project update mode
# ------------------------------------------------

##############################################################################
# target
##############################################################################
TARGET = board


##############################################################################
# building variables
##############################################################################
# debug build?
DEBUG = 1

# optimization
OPT = -Og

# 编译选项
CFLAGS_OPT = 

# printf 重定向
# CFLAGS_OPT += -fno-builtin

# 链接选项
LDFLAGS_OPT = -u _printf_float
LDFLAGS_OPT += 

##############################################################################
# paths
##############################################################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
C_SOURCES =  \
CubeMX/Core/Src/stm32f1xx_it.c \
CubeMX/Core/Src/stm32f1xx_hal_msp.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio_ex.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_i2c.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_exti.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_iwdg.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rtc.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rtc_ex.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_sdmmc.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_sd.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_spi.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pcd.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pcd_ex.c \
CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_usb.c \
CubeMX/Core/Src/system_stm32f1xx.c \
CubeMX/Core/Src/main.c

# 自动添加c文件目录
C_SOURCES_PATH = application/APP \
				 application/BSP \
				 application/FreeRTOS \
				 application/FreeRTOS/portable \
				 application/Lib \
				 application/FatFs/core \
				 application/FatFs/sdio

#添加 C_SOURCES_PATH 目录下所有c文件列表
C_SOURCES += $(foreach dir,$(C_SOURCES_PATH),$(wildcard $(dir)/*.c))

# ASM sources
ASM_SOURCES =  \
startup_stm32f103xe.s


##############################################################################
# binaries
##############################################################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
##############################################################################
# CFLAGS
##############################################################################
# cpu
CPU = -mcpu=cortex-m3

# fpu
# NONE for Cortex-M0/M0+/M3

# float-abi


# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)


# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-D USE_HAL_DRIVER \
-D STM32F103xE



# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-I CubeMX/Core/Inc \
-I CubeMX/Drivers/STM32F1xx_HAL_Driver/Inc \
-I CubeMX/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy \
-I CubeMX/Drivers/CMSIS/Device/ST/STM32F1xx/Include \
-I CubeMX/Drivers/CMSIS/Include \
-I application/APP/inc \
-I application/BSP/inc \
-I application/FreeRTOS/include \
-I application/FreeRTOS/portable \
-I application/Lib/inc \
-I application/FatFs/core/inc


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(CFLAGS_OPT) $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

##############################################################################
# LDFLAGS
##############################################################################
# link script
LDSCRIPT = STM32F103ZETx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(CFLAGS_OPT) $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS)
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections
#LDFLAGS += $(LDFLAGS_OPT)

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin



##############################################################################
# build the application
##############################################################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(ASM_SOURCES:.s=.o))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

# 编译文件目录
OBJECTS_DIR = $(sort $(dir $(OBJECTS)))


$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@
	
$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	for x in $(OBJECTS_DIR); do \
		mkdir -p $$x; \
	done



##############################################################################
# clean up
##############################################################################
# keil 编译器文件目录
MDK_COMPEILE := CubeMX/MDK-ARM/board
clean:
	-rm -fR $(BUILD_DIR)
	find $(MDK_COMPEILE) -name *.d | xargs rm -f
	find $(MDK_COMPEILE) -name *.o | xargs rm -f
	find $(MDK_COMPEILE) -name *.crf | xargs rm -f

##############################################################################
# make file debug
##############################################################################
debug:
	echo $(C_SOURCES)

##############################################################################
# header file dependencies
##############################################################################
#-include $(wildcard $(BUILD_DIR)/*.d)
-include $(foreach dir,$(OBJECTS_DIR),$(wildcard $(dir)/*.d))
# *** EOF ***



# #获取当前工作路径
# TOP_DIR:=$(CURDIR)
# #设置目标名
# Target:=hello
# #设置源文件目录
# SRC_PATH:=$(TOP_DIR) $(TOP_DIR)/other
# #设置编译目录
# BUILD_PATH:=$(TOP_DIR)/build
# #设置编译临时目录
# OBJ_PATH:=$(BUILD_PATH)/temp
# #设置编译最终文件目录
# BIN_PATH:=$(BUILD_PATH)/bin
# #获取源文件目录下所有c文件列表
# SRC:=$(foreach dir,$(SRC_PATH),$(wildcard $(dir)/*.c))

# #去掉c文件目录
# SRC_WITHOUT_DIR:=$(notdir $(SRC))
# #生成.o文件列表
# OBJ_WITHOUT_DIR:=$(patsubst %.c,%.o,$(SRC_WITHOUT_DIR))
# #为.o文件列表加上编译目录
# OBJ_WITH_BUILD_DIR:=$(addprefix $(OBJ_PATH)/,$(OBJ_WITHOUT_DIR))
# $(info "OBJ_WITH_BUILD_DIR:$(OBJ_WITH_BUILD_DIR)")
# #添加头文件目录
# CFLAGS=$(addprefix -I,$(SRC_PATH))
# #为gcc添加源文件搜索目录
# VPATH=$(SRC_PATH)
# #编译目标
# all:build_prepare $(Target)
# #连接目标
# $(Target):$(OBJ_WITH_BUILD_DIR) 
# 	cc -o $(BIN_PATH)/$@ $^
# #编译生成.o文件
# $(OBJ_PATH)/%.o:%.c
# 	cc -c $(CFLAGS) -o $@ $<
# #创建编译目录
# build_prepare:
# 	@if [ ! -d $(BUILD_PATH) ]; then \
# 	mkdir -p $(OBJ_PATH); \
# 	mkdir -p $(BIN_PATH); \
# 	fi

# .PHONY:clean

# clean:
# 	-rm -rf $(BIN_PATH)/$(Target) $(OBJ_WITH_BUILD_DIR)