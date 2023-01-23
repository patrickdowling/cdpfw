###
## Simple makefile project for attiny85 amp cpntrol
#
PROJECT         = amp_control
PROJECT_ROOT    = .
PROJECT_SRCDIRS = .
# PROJECT_DEFINES +=

OPTIMIZE   ?= s
TARGET_MCU = attiny85
F_CPU      ?= 8000000

CFLAGS += -flto
LDFLAGS += -flto

# http://eleccelerator.com/fusecalc/fusecalc.php?chip=attiny85&LOW=E2&HIGH=DF&EXTENDED=FF&LOCKBIT=FF
ifeq "8000000" "$(F_CPU)"
LFUSE = 0xE2
else ifeq "1000000" "$(F_CPU)"
LFUSE = 0x62
else
$(error Invalid F_CPU '$(F_CPU)')
endif
HFUSE = 0xDF
EFUSE = 0xFF
LOCK  = 0xFF

PROGRAMMER ?= stk500mk2
AVRDUDE_EXTRA ?= -B 100

CPPCHECK_SRC = $(wildcard ./*.cc)
CPPCHECK_INCLUDES = $(PROJECT_SRCDIRS) $(INCLUDES)
CPPCHECK_DEFINES = ATMEL_AVR

include ./avrx/avrx.mk
