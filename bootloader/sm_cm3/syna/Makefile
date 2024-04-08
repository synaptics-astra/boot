##################################################
#
##      Usage
#
#       Generate default target for default platform
#       $ make
#
#       Generate default target for specified platform
#       $ make PLATFORM=bg2q4k
#
#       Generate specified target for specified platform
#       $ make ram PLATFORM=bg2q4k
#
#       Generate specified target for specified platform,
#       and re-generate rom binary/table
#       $ make all PLATFORM=bg2q4k MAKE_ROM=1
#
#       Default Target
#       sketch: pre clean ram
#
##################################################

###########################################################
## Assertions about attributes of the target
###########################################################

# $(1): The file to check
define get-file-size
stat -c "%s" "$(1)" | tr -d '\n'
endef

# $(1): The file(s) to check (often $@)
# $(2): Target size limit
define assert-max-image-size
$(if $(2), \
  size=$$(for i in $(1); do $(call get-file-size,$$i); echo +; done; echo 0); \
  total=$$(( $$( echo "$$size" ) )); \
  printname=$$(echo -n "$(1)" | tr " " +); \
  maxsize=$$(($(2))); \
  if [ "$$total" -gt "$$maxsize" ]; then \
    echo "error: $$printname too large ($$total > $$maxsize)"; \
    false; \
  elif [ "$$total" -gt $$((maxsize - 32768)) ]; then \
    echo "WARNING: $$printname approaching size limit ($$total now; limit $$maxsize)"; \
  fi \
 , \
  true \
 )
endef

##################################################
#
##	Used Directory
#
##################################################

CONFIG_FILE ?= .config

include $(CONFIG_FILE)
SM_SIZE=40960

CHIP_NAME=$(shell echo $(BERLIN_CHIP) | tr '[A-Z]' '[a-z]')

BL_CHIP_DIR := ../customization
CUSTOM_DIR := customization/$(CHIP_NAME)

PLATFORM_DEFAULT = $(CHIP_NAME)-rdk

PLATFORM_VALID = $(shell \
		 if [ -d $(CUSTOM_DIR)/$(PLATFORM) ]; then \
		     echo $(PLATFORM); \
		 else \
		     echo $(PLATFORM_DEFAULT); \
		 fi;)

INSTALL := install -p -m 644
SRC_DIR := .
WORK_DIR := $(OUTPUT_DIR)/output_sm
OBJ_DIR := $(WORK_DIR)/obj
BUILD_DIR := $(WORK_DIR)/bin
FREERTOS_DIR := ../FreeRTOS
SYNA_DIR := .

##################################################
#
##	Cross compiling
#
##################################################

### This toolchain should be auto selected later.

CC := $(SM_CROSS_COMPILE)gcc
AS := $(SM_CROSS_COMPILE)gcc
LD := $(SM_CROSS_COMPILE)ld
AR := $(SM_CROSS_COMPILE)ar
OBJCOPY := $(SM_CROSS_COMPILE)objcopy
OBJDUMP := $(SM_CROSS_COMPILE)objdump
DISAS := $(SM_CROSS_COMPILE)objdump
SIZE := $(SM_CROSS_COMPILE)size
NM := $(SM_CROSS_COMPILE)nm
#libgcc.a
LIBGCC := $(shell $(CC) --print-libgcc-file-name)


##################################################
#
##	Local source files
#
##################################################

sSRCS += \
	 $(SYNA_DIR)/src/sm_entry.S

#FreeRTOS source files
cSRCS += \
	 $(FREERTOS_DIR)/Source/croutine.c \
	 $(FREERTOS_DIR)/Source/event_groups.c \
	 $(FREERTOS_DIR)/Source/list.c \
	 $(FREERTOS_DIR)/Source/queue.c \
	 $(FREERTOS_DIR)/Source/stream_buffer.c \
	 $(FREERTOS_DIR)/Source/tasks.c \
	 $(FREERTOS_DIR)/Source/timers.c \
	 $(FREERTOS_DIR)/Source/portable/MemMang/heap_4.c

#FreeRTOS arm related source files
#CM3
cSRCS += \
	 $(FREERTOS_DIR)/Source/portable/GCC/ARM_CM3/port.c

cSRCS += \
	 $(SYNA_DIR)/src/sm_main.c \
	 $(SYNA_DIR)/src/sm_uart.c \
	 $(SYNA_DIR)/src/sm_comm.c \
	 $(SYNA_DIR)/src/sm_printf.c \
	 $(SYNA_DIR)/src/sm_rt_module.c \
	 $(SYNA_DIR)/src/sm_exception.c \
	 $(SYNA_DIR)/src/sm_state.c \
	 $(SYNA_DIR)/src/sm_ictl.c \
	 $(SYNA_DIR)/src/sm_gpio.c \
	 $(SYNA_DIR)/src/sm_wdt.c \
	 $(SYNA_DIR)/src/sm_ddr_ctrl.c \
	 $(SYNA_DIR)/src/sm_temperature.c \
	 $(SYNA_DIR)/src/sm_led.c \
	 $(SYNA_DIR)/src/sm_wifibt.c \
	 $(SYNA_DIR)/src/sm_apb_timer.c \
	 $(SYNA_DIR)/src/unit_test/gpio_test.c \
	 $(SYNA_DIR)/src/power/sm_power_flow.c \
	 $(SYNA_DIR)/src/power/sm_power_op.c \
	 $(SYNA_DIR)/src/power/sm_power_timer.c \
	 $(SYNA_DIR)/src/power/sm_power_eth.c \
	 $(SYNA_DIR)/src/power/sm_power_misc.c \
	 $(SYNA_DIR)/src/sm_i2c_raw.c \
	 $(SYNA_DIR)/src/sm_fxl6408.c \
 	 $(SYNA_DIR)/src/btn/sm_gpiobtn.c \
	 $(SYNA_DIR)/src/btn/sm_adcbtn.c \
	 $(SYNA_DIR)/src/cec/sm_cec.c \
	 $(SYNA_DIR)/src/cec/cec_srv.c \
	 $(SYNA_DIR)/src/power/sm_power_wol.c \
	 $(SYNA_DIR)/src/ir/sm_ir.c \
	 $(SYNA_DIR)/src/ir/ir_nec.c \
	 $(SYNA_DIR)/src/ir/ir_xmp.c \
	 $(SYNA_DIR)/src/ir/ir_rc5.c \
	 $(SYNA_DIR)/src/ir/ir_tc9012.c \
	 
#chip and platform customization
cSRCS += \
   $(CUSTOM_DIR)/chip_customization.c \
   $(CUSTOM_DIR)/$(PLATFORM_VALID)/platform_customization.c

#common library
cSRCS += $(COMM_DIR)/lib/string.c

##################################################
#
##	Local C & LD flags
#
##################################################

CFLAGS := \
	-I$(SYNA_DIR)/inc \
	-I$(SYNA_DIR)/inc/$(CHIP_NAME) \
	-I$(SYNA_DIR)/inc/$(CHIP_NAME)/fw_include \
	-I$(FREERTOS_DIR)/Source/include \
	-I$(COMM_DIR)/inc \
	-I$(COMM_DIR)/driver/ram_ts \

CFLAGS += \
	-I$(FREERTOS_DIR)/Source/portable/GCC/ARM_CM3

CFLAGS += -DSM_ENABLE

CFLAGS += \
	  -I$(CUSTOM_DIR) \
	  -I$(CUSTOM_DIR)/$(PLATFORM_VALID)

CFLAGS += -Wall -Werror -nostdlib -fno-builtin -fno-stack-protector -O2 -U_FORTIFY_SOURCE

#include $(BL_CHIP_DIR)/chipversions

CFLAGS += -mcpu=cortex-m3 -mthumb -MD

CFLAGS += \
	  -mlittle-endian -Os \
	  -fomit-frame-pointer -fno-common \
	  -fno-strict-aliasing -fno-dwarf2-cfi-asm \
	  -ffunction-sections -fdata-sections
	  #-mlittle-endian -march=armv8-m.base -Os

LDFLAGS := -nostdlib -nostartfiles --gc-sections

##################################################
#
##	Customized C & LD flags
#
##################################################

#CFLAGS += -DFPGA_V
ifeq ($(CONFIG_PRODUCTION_BUILD), y)
CFLAGS += -DCONFIG_PRODUCTION_BUILD
endif

ifeq ($(CONFIG_SM_MENUCONFIG), y)
  ifeq ($(CONFIG_SM_TEMPERATURE_MONITOR), y)
    CFLAGS += -DTEMPENABLE
    ifeq ($(CONFIG_SM_OVERHEAT_MONITOR), y)
      CFLAGS += -DOVER_HEAT_TEMPERATURE
    endif
  endif
  ifeq ($(CONFIG_SM_WAKE_ONLINE), y)
    ifeq ($(CONFIG_SM_GIGA_ETHERNET), y)
      CFLAGS += -DGETHENABLE
    else
      CFLAGS += -DWOLENABLE
    endif
  endif
  ifeq ($(CONFIG_SM_IR_PROTOCOL), y)
    CFLAGS += -DIRENABLE
    ifeq ($(CONFIG_SM_IRP_NEC), y)
      CFLAGS += -DNEC_PROTOCAL
    endif
    ifeq ($(CONFIG_SM_IRP_RC5), y)
      CFLAGS += -DRC5_PROTOCAL
    endif
    ifeq ($(CONFIG_SM_IRP_XMP), y)
      CFLAGS += -DXMP_PROTOCAL
    endif
    ifeq ($(CONFIG_SM_IRP_TC9012), y)
      CFLAGS += -DTC9012_PROTOCAL
    endif
    ifeq ($(CONFIG_SM_IR_RC_VENDOR), NEC_SYNA_RC)
      CFLAGS += -DNEC_SYNA_RC
    endif
    ifeq ($(CONFIG_SM_IR_RC_VENDOR), NEC_SKY_RC)
      CFLAGS += -DNEC_SKY_RC
    endif
    ifeq ($(CONFIG_SM_IR_RC_VENDOR), NEC_ARCADYAN_RC)
      CFLAGS += -DNEC_ARCADYAN_RC
    endif
	ifeq ($(CONFIG_SM_IR_RC_VENDOR), RC5_SYNA_RC)
      CFLAGS += -DRC5_SYNA_RC
    endif
	ifeq ($(CONFIG_SM_IR_RC_VENDOR), NEC_SYNA_RC)
      CFLAGS += -DXMP_SYNA_RC
    endif
	ifeq ($(CONFIG_SM_IR_RC_VENDOR), NEC_SYNA_RC)
      CFLAGS += -DTC9012_SYNA_RC
    endif
  endif
  ifeq ($(CONFIG_SM_CEC), y)
    CFLAGS += -DCECENABLE -DCEC_SM_SOC_COMM -DBUDDY_BOX_SM_CEC -DHWCEC
  endif
  ifeq ($(CONFIG_SM_WATCHDOG_MONITOR), y)
    CFLAGS += -DWDTENABLE
  endif
  ifeq ($(CONFIG_SM_ACTIVE_STANDBY), y)
    CFLAGS += -DWFIENABLE
  endif
  ifeq ($(CONFIG_SM_LED), y)
    CFLAGS += -DLEDENABLE
  endif
  ifeq ($(CONFIG_SM_GPIOBTN), y)
    CFLAGS += -DGPIOBTNENABLE
  endif
  ifeq ($(CONFIG_SM_ADCBTN), y)
    CFLAGS += -DADCBTNENABLE
  endif
  ifeq ($(CONFIG_FUNCTION_BUTTON_HIGH_LEV_VALID), y)
	CFLAGS += -DCONFIG_FUNCTION_BUTTON_HIGH_LEV_VALID
  endif

  ifeq ($(CONFIG_SM_RAM_PARAM_ENABLE), y)
    CFLAGS += -DSM_RAM_PARAM_ENABLE
  endif
  ifeq ($(CONFIG_SM_RAM_TS_ENABLE), y)
    CFLAGS += -DSM_RAM_TS_ENABLE
    cSRCS += $(COMM_DIR)/driver/ram_ts/ram_ts.c
  endif

  CFLAGS += -DLTIMER_ENABLE
  CFLAGS += -DCONSOLEENABLE

  ifeq ($(CONFIG_SM_MICMUTE_ADC), y)
    CFLAGS += -DCONFIG_SM_MICMUTE_ADC
    CFLAGS += -DCONFIG_SM_MICMUTE_ADC_CH=$(CONFIG_SM_MICMUTE_ADC_CH)
    ifeq ($(CONFIG_SM_MICMUTE_HIGH_LEV_VALID), y)
	CFLAGS += -DCONFIG_SM_MICMUTE_HIGH_LEV_VALID
    endif
    CFLAGS += -DCONFIG_SM_MICMUTE_EVENT=$(CONFIG_SM_MICMUTE_EVENT)
  endif
else
  include $(CUSTOM_DIR)/$(PLATFORM_VALID)/platform.mk
endif

# platform customization
#include $(CUSTOM_DIR)/$(PLATFORM_VALID)/platform.mk

# Suppress annoying output unless V is set
#define V
ifndef V
	SILENT_CC       = @echo '  [CC]       ' $<;
	SILENT_AS       = @echo '  [AS]       ' $<;
	SILENT_CXX      = @echo '  [CXX]      ' $<;
	SILENT_LD       = @echo '  [LD]       ' $@;
	SILENT_AR       = @echo '  [AR]       ' $@;
	SILENT_OBJCOPY  = @echo '  [OBJCOPY]  ' $@;
	SILENT_OBJDUMP  = @echo '  [OBJDUMP]  ' $@;
	SILENT_DISAS    = @echo '  [DISAS]    ' $@;
	SILENT_NM       = @echo '  [NM]       ' $@;
endif
#endef

##################################################
#
##	Target & Object files
#
##################################################

ELF_TARGET       := $(OBJ_DIR)/sm.elf
RAW_TARGET       := $(OBJ_DIR)/sm.bin
DISAS_TARGET     := $(OBJ_DIR)/sm.disas
MAP_TARGET       := $(OBJ_DIR)/sm.map
SIZE_TARGET      := $(OBJ_DIR)/sm.sizes
SYM_TARGET       := $(OBJ_DIR)/sm.sym

OBJECTS := $(sSRCS:%.S=$(OBJ_DIR)/%.o) $(cSRCS:%.c=$(OBJ_DIR)/%.o)
ROMOBJECTS := $(sROMSRCS:%.S=$(OBJ_DIR)/%.o) $(cROMSRCS:%.c=$(OBJ_DIR)/%.o)

LDS = $(CUSTOM_DIR)/sm.lds

##################################################
#
##	General rules
#
##################################################

.PHONY: all clean install pre sketch
.DEFAULT_GOAL := sketch

# Default Target
sketch: pre ram

ram: $(ROM_TABLE) $(ELF_TARGET) $(RAW_TARGET) $(DISAS_TARGET) $(SYM_TARGET)
	@mkdir -p $(BUILD_DIR)
	@-cp -fu --preserve=timestamp $(ELF_TARGET) $(RAW_TARGET) $(BUILD_DIR)
	@find $(WORK_DIR) -iname *.o | xargs $(SIZE) -t > $(SIZE_TARGET)
	@echo "final size:"
	@$(SIZE) $(ELF_TARGET)

all: $(ROM_TABLE) $(ROM_RAW_TARGET) $(ROM_DISAS_TARGET) $(ELF_TARGET) $(RAW_TARGET) $(DISAS_TARGET) $(SYM_TARGET)
	@mkdir -p $(BUILD_DIR)
	@-cp -fu --preserve=timestamp $(ELF_TARGET) $(RAW_TARGET) $(ROM_ELF_TARGET) $(ROM_RAW_TARGET) $(BUILD_DIR)
	@find $(WORK_DIR) -iname *.o | xargs $(SIZE) -t > $(SIZE_TARGET)
	@echo "final size:"
	@$(SIZE) $(ELF_TARGET) $(ROM_ELF_TARGET)

$(ELF_TARGET): $(OBJECTS) $(LDS)
	@mkdir -p $(dir $@)
	$(SILENT_LD) $(LD) -T$(LDS) $(LDFLAGS) $(OBJECTS) $(LIBGCC) -o $@ -Map $(MAP_TARGET)

$(RAW_TARGET): $(ELF_TARGET)
	@mkdir -p $(dir $@)
	$(SILENT_OBJCOPY) $(OBJCOPY) -S -O binary $< $@
	$(OBJDUMP) -D $< > $@.dump

$(DISAS_TARGET): $(ELF_TARGET)
	@mkdir -p $(dir $@)
	$(SILENT_DISAS) $(DISAS) -x -d $< > $@

$(SYM_TARGET): $(ELF_TARGET)
	@mkdir -p $(dir $@)
	$(SILENT_NM) $(NM) -n $< > $@


# FIXME Force depend on clean here to avoid unexpected failures.
#$(OBJECTS): clean

# General Rules
$(OBJ_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(SILENT_AS) $(AS) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(SILENT_CC) $(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -fr $(WORK_DIR)

# Copy sm.bin and sm.elf to output dir
install: sketch
	@mkdir -p $(SYS_INSTALL_DIR)
	@-cp -fu --preserve=timestamp $(RAW_TARGET) $(SYS_INSTALL_DIR)
	@-cp -fu --preserve=timestamp $(ROM_RAW_TARGET) $(SYS_INSTALL_DIR)
	@echo "Installed System Manager binary to " $(SYS_INSTALL_DIR)"/sm.bin"

ci: sketch
	@$(call assert-max-image-size,${OUTPUT_DIR}/output_sm/bin/sm.bin,$(SM_SIZE))

pre:
ifneq ($(PLATFORM), $(PLATFORM_VALID))
	$(warning "$(PLATFORM)" is not valid, use default "$(PLATFORM_DEFAULT)"!)
endif
