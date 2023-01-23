###
## Simple makefile project for Atmega328p controller board 
#

PROJECT         = cdp_control
PROJECT_ROOT    = .
PROJECT_SRCDIRS = . menus resources drivers ui util
PROJECT_DEFINES += ENABLE_USART_TX
# PROJECT_DEFINES += DEBUG_IRMP
# PROJECT_DEFINES += DEBUG_MUTE_SYSTICK
# PROJECT_DEFINES += DEBUG_FORCE_LID
PROJECT_DEFINES += ENABLE_SERIAL_TRACE
# PROJECT_DEFINES += ENABLE_SLEEP
PROJECT_DEFINES += SERIAL_BAUD=115200

VERSION_STRING ?= 0.0.0
PROJECT_DEFINES += CDPFW_VERSION_STRING=\"$(VERSION_STRING)\"

#IRMP
PROJECT_DEFINES += F_INTERRUPTS=\(16*1024\)
IRMP_DIR = ./extern/irmp
INCLUDES += $(IRMP_DIR)
EXTERN_C_FILES = $(notdir $(IRMP_DIR)/irmp.c)
VPATH += $(IRMP_DIR)

CFLAGS += -flto
LDFLAGS += -flto
# LDFLAGS +=-Wl,-u,vfprintf -lprintf_min

LDFLAGS += -Tatmega328p.xn

TARGET_MCU = atmega328p
F_CPU      = 20000000
OPTIMIZE   ?= 3


LFUSE = 0xFF
HFUSE = 0xD9
EFUSE = 0xFD
LOCK  = 0xFF
# 0xFD is the same as 0x05 because only the bottom three bits are used. i.e. 0xFD = 0xF8 | 0x05

PROGRAMMER ?= stk500v2

CPPCHECK_SRC = $(wildcard ./*.cc) menus resources drivers ui
CPPCHECK_INCLUDES = $(PROJECT_SRCDIRS) $(INCLUDES)
CPPCHECK_DEFINES = ATMEL_AVR

include ./avrx/avrx.mk
