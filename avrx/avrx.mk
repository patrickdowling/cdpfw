###
## Simple makefile project for avrx projects
#

# Required variables
check_variable_list = \
    $(strip $(foreach 1,$1, \
        $(call __check_variable_list,$1,$(strip $(value 2)))))
__check_variable_list = \
    $(if $(value $1),, \
      $(error Undefined $1$(if $2, ($2))))

$(call check_variable_list, PROJECT_ROOT)
$(call check_variable_list, PROJECT PROJECT_SRCDIRS)
$(call check_variable_list, TARGET_MCU F_CPU)
$(call check_variable_list, LFUSE HFUSE EFUSE LOCK)
$(call check_variable_list, PROGRAMMER)

CFLAGS += -flto
LDFLAGS += -flto

OPTIMIZE ?= s

AVRDUDE ?= avrdude
AVRDUDE_OPTS += -q # quieter
AVRDUDE_OPTS += -u -c $(PROGRAMMER)
ifneq (,$(PROGRAMMER_PORT))
AVRDUDE_OPTS += -P $(PROGRAMMER_PORT)
endif
AVRDUDE_OPTS += -p $(TARGET_MCU)
# AVRDUDE_OPTS += -e # erase
AVRDUDE_EXTRA ?= -B 1

BUILD_DIR   = $(PROJECT_ROOT)/build
TARGET_ELF  = $(BUILD_DIR)/$(PROJECT).elf
TARGET_DIS  = $(TARGET_ELF:.elf=.S)
TARGET_MAP  = $(TARGET_ELF:.elf=.map)
TARGET_SIZE = $(TARGET_ELF:.elf=.size)
TARGET_SYM  = $(TARGET_ELF:.elf=.sym)

VPATH += $(PROJECT_SRCDIRS)
CC_FILES = $(notdir $(wildcard $(patsubst %,%/*.cc,$(PROJECT_SRCDIRS))))
C_FILES  = $(notdir $(wildcard $(patsubst %,%/%.c,$(PROJECT_SRCDIRS))))

OBJS = $(patsubst %,$(BUILD_DIR)/%,$(CC_FILES:.cc=.o) $(C_FILES:.c=.o) $(EXTERN_C_FILES:.c=.o))
DEPS = $(OBJS:.o=.d)

SYSTEM_DEFINES += F_CPU=$(F_CPU)
SYSTEM_DEFINES += __PROG_TYPES_COMPAT__
SYSTEM_DEFINES += $(shell echo $(TARGET_MCU) | tr  '[:lower:]' '[:upper:]')

CFLAGS += -c
CFLAGS += -g0
CFLAGS += -Wall -Werror -Wextra -Wshadow #-Wconversion
CFLAGS += -ffunction-sections -fdata-sections -fshort-enums \
	-funsigned-char \
	-funsigned-bitfields \
	-ffast-math \
	-freciprocal-math \
	-fpack-struct \
	-finline-functions-called-once \
	-finline-functions 

CFLAGS += -MMD -MP
CFLAGS += -O$(OPTIMIZE)
CFLAGS += -mmcu=$(TARGET_MCU)
CFLAGS += $(addprefix -D, $(SYSTEM_DEFINES))
CFLAGS += $(addprefix -D, $(PROJECT_DEFINES))
CFLAGS += $(addprefix -I, $(PROJECT_SRCDIRS))
CFLAGS += $(addprefix -I, $(INCLUDES))

LDFLAGS += -mmcu=$(TARGET_MCU) -Wl,-Map=$(TARGET_MAP) -Wl,--start-group -Wl,-lm -Wl,-lc -Wl,--end-group -Wl,-gc-sections 

CXXFLAGS += -fno-exceptions -fno-rtti -std=c++1z -fno-use-cxa-atexit \
	-Wpedantic \
	-Wnon-virtual-dtor \
	-Woverloaded-virtual

###
## Executable paths
#
ifdef VERBOSE
AT :=
ECHO := @true
LDFLAGS += -Wl,--verbose
else
AT := @
ECHO := @echo
endif

TOOLCHAIN_PATH ?=
CC    = $(addprefix $(TOOLCHAIN_PATH),avr-gcc)
CXX   = $(addprefix $(TOOLCHAIN_PATH),avr-c++)
LD    = $(addprefix $(TOOLCHAIN_PATH),avr-ld)
CP    = $(addprefix $(TOOLCHAIN_PATH),avr-objcopy)
OD    = $(addprefix $(TOOLCHAIN_PATH),avr-objdump)
AS    = $(addprefix $(TOOLCHAIN_PATH),avr-as)
SIZE  = $(addprefix $(TOOLCHAIN_PATH),avr-size)
NM    = $(addprefix $(TOOLCHAIN_PATH),avr-nm)
MKDIR = $(AT)mkdir -p
RM    = $(AT)rm -f

###
## Targets
#
.PHONY: all
all: size

.PHONY: size
size: $(TARGET_SIZE)
	$(AT)cat $(TARGET_SIZE)

.PHONY: symbols
symbols: $(TARGET_SYM)
	$(AT)cat $(TARGET_SYM)

.PHONY: disassemble
disassemble: $(TARGET_DIS)

$(BUILD_DIR): Makefile
	$(MKDIR) $(BUILD_DIR)

.PHONY: fuses
fuses:
	$(AVRDUDE) $(AVRDUDE_OPTS) -B 100 -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m -U lock:w:$(LOCK):m

.PHONY: upload
upload: $(TARGET_ELF)
	$(AVRDUDE) $(AVRDUDE_OPTS) $(AVRDUDE_EXTRA) -U flash:w:$(TARGET_ELF)

.PHONY: clean
clean:
	$(RM) $(OBJS) $(DEPS)
	$(RM) $(TARGET_ELF)
	$(RM) $(TARGET_DIS) $(TARGET_MAP) $(TARGET_SIZE) $(TARGET_SYM)

CPPCHECK_FLAGS ?= --enable=all -inconclusives
CPPCHECK_FLAGS += --platform=avr8
CPPCHECK_FLAGS += --suppress=missingIncludeSystem
CPPCHECK_FLAGS += $(addprefix -I,$(CPPCHECK_INCLUDES))
CPPCHECK_FLAGS += $(addprefix -D,$(CPPCHECK_DEFINES))
# CPPCHECK_EXTRA=--check-config

.PHONY: check
check:
	$(AT)cppcheck $(CPPCHECK_FLAGS) $(CPPCHECK_EXTRA) $(CPPCHECK_SRC)

###
## Build rules
#
$(BUILD_DIR)/%.o: %.cc
	$(ECHO) "CC $<..."
	$(AT)$(CXX) $(CFLAGS) $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.c
	$(ECHO) "C $<..."
	$(AT)$(CC) $(CFLAGS) $< -o $@

$(TARGET_ELF): $(OBJS)
	$(ECHO) "Linking $@..."
	$(AT)$(CC) $(LDFLAGS) -Wl,--start-group $^ -Wl,--end-group -o $@

$(TARGET_DIS): $(TARGET_ELF)
	$(ECHO) "Dissing $@..."
	$(AT)$(OD) -dC -h -S $< > $@ 2>/dev/null

$(TARGET_SIZE): $(TARGET_ELF)
	$(AT)$(SIZE) -B --target=elf32-avr $< > $(TARGET_SIZE)

$(TARGET_SYM): $(TARGET_ELF)
	$(AT)$(NM) --size-sort -CrS $< > $(TARGET_SYM)

$(OBJS) : | $(BUILD_DIR)

# Automatic dependency generation
-include $(DEPS)
