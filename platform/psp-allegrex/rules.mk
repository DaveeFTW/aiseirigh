LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ARCH := allegrex

MODULE_DEPS += \
    lib/cbuf

MODULE_SRCS += \
	$(LOCAL_DIR)/platform.c \
	$(LOCAL_DIR)/debug.c \
	$(LOCAL_DIR)/display.c
#	$(LOCAL_DIR)/intc.c

MEMBASE ?= 0x88000000 # the start of kernel memory
MEMSIZE ?= 0x02000000 # 32MB

MODULE_SRCS += \
	external/platform/iplsdk/src/cpu/cpu.S \
	external/platform/iplsdk/src/cpu/cache.S \
	external/platform/iplsdk/src/exception/exception.c \
	external/platform/iplsdk/src/exception/exception.S \
	external/platform/iplsdk/src/interrupt/interrupt.c \
	external/platform/iplsdk/src/interrupt/interrupt.S \
	external/platform/iplsdk/src/display/display.c \
	external/platform/iplsdk/src/dmacplus/dmacplus.c \
	external/platform/iplsdk/src/emcddr/emcddr.c \
	external/platform/iplsdk/src/emcddr/emcddr.S \
	external/platform/iplsdk/src/emcsm/emcsm.c \
	external/platform/iplsdk/src/emcsm/chip.c \
	external/platform/iplsdk/src/emcsm/ecc.c \
	external/platform/iplsdk/src/emcsm/scramble.c \
	external/platform/iplsdk/src/emcsm/state.c \
	external/platform/iplsdk/src/gpio/gpio.c \
	external/platform/iplsdk/src/kirk/kirk.c \
	external/platform/iplsdk/src/lcd/lcd.c \
	external/platform/iplsdk/src/lcd/phatlcd.c \
	external/platform/iplsdk/src/lcd/hibari.c \
	external/platform/iplsdk/src/lcd/samantha.c \
	external/platform/iplsdk/src/lcd/tmdlcd.c \
	external/platform/iplsdk/src/lcd/streetlcd.c \
	external/platform/iplsdk/src/lcdc/lcdc.c \
	external/platform/iplsdk/src/led/led.c \
	external/platform/iplsdk/src/lflash/lflash.c \
	external/platform/iplsdk/src/model/model.c \
	external/platform/iplsdk/src/mspro/mspro.c \
	external/platform/iplsdk/src/pwm/pwm.c \
	external/platform/iplsdk/src/spi/spi.c \
	external/platform/iplsdk/src/syscon/syscon.c \
	external/platform/iplsdk/src/syscon/comms.c \
	external/platform/iplsdk/src/syscon/handshake.c \
	external/platform/iplsdk/src/sysreg/sysreg.c \
	external/platform/iplsdk/src/uart/uart.c \
	external/platform/iplsdk/src/utils/utils.c \
	external/platform/iplsdk/src/utils/delay.S

#	external/platform/iplsdk/src/interrupt/interrupt.c
#	external/platform/iplsdk/src/cpu/cpu.S \
#	external/platform/iplsdk/src/cpu/cache.S
#	external/platform/iplsdk/src/exception/exception.c

GLOBAL_INCLUDES += \
	external/platform/iplsdk/src/cpu \
	external/platform/iplsdk/src/display \
	external/platform/iplsdk/src/dmacplus \
	external/platform/iplsdk/src/emcddr \
	external/platform/iplsdk/src/emcsm \
	external/platform/iplsdk/src/exception \
	external/platform/iplsdk/src/gpio \
	external/platform/iplsdk/src/interrupt \
	external/platform/iplsdk/src/kirk \
	external/platform/iplsdk/src/lcd \
	external/platform/iplsdk/src/lcdc \
	external/platform/iplsdk/src/led \
	external/platform/iplsdk/src/lflash \
	external/platform/iplsdk/src/model \
	external/platform/iplsdk/src/mspro \
	external/platform/iplsdk/src/pwm \
	external/platform/iplsdk/src/spi \
	external/platform/iplsdk/src/syscon \
	external/platform/iplsdk/src/sysreg \
	external/platform/iplsdk/src/uart \
	external/platform/iplsdk/src/utils

MODULE_DEPS += \

LKLOADER_SRCS := \
	external/platform/iplsdk/src/cpu/cpu.S \
	external/platform/iplsdk/src/cpu/cache.S \
	external/platform/iplsdk/src/emcddr/emcddr.c \
	external/platform/iplsdk/src/emcddr/emcddr.S \
	external/platform/iplsdk/src/gpio/gpio.c \
	external/platform/iplsdk/src/kirk/kirk.c \
	external/platform/iplsdk/src/spi/spi.c \
	external/platform/iplsdk/src/syscon/syscon.c \
	external/platform/iplsdk/src/syscon/comms.c \
	external/platform/iplsdk/src/syscon/handshake.c \
	external/platform/iplsdk/src/sysreg/sysreg.c \
	external/platform/iplsdk/src/uart/uart.c \
	$(LOCAL_DIR)/lk_loader/main.c \
	$(LOCAL_DIR)/lk_loader/crt0.S


# generate the lk loader
GENERATED += \
	$(BUILDDIR)/lk_loader_linker.ld

# rules for generating the linker
LKLOADER_MEMBASE ?= 0x04000000 # the start of kernel memory
LKLOADER_MEMSIZE ?= 0x00200000 # 2MB
LKLOADER_KERNEL_BASE ?= $(LKLOADER_MEMBASE)
LKLOADER_KERNEL_LOAD_OFFSET ?= 0
LKLOADER_VECTOR_BASE_PHYS ?= 0

$(BUILDDIR)/lk_loader_linker.ld: $(LOCAL_DIR)/lk_loader/linker.ld $(wildcard arch/*.ld) linkerscript.phony
	@echo generating $@
	@$(MKDIR)
	$(NOECHO)sed "s/%MEMBASE%/$(LKLOADER_MEMBASE)/;s/%MEMSIZE%/$(LKLOADER_MEMSIZE)/;s/%KERNEL_BASE%/$(LKLOADER_KERNEL_BASE)/;s/%KERNEL_LOAD_OFFSET%/$(LKLOADER_KERNEL_LOAD_OFFSET)/;s/%VECTOR_BASE_PHYS%/$(LKLOADER_VECTOR_BASE_PHYS)/" < $< > $@.tmp
	@$(call TESTANDREPLACEFILE,$@.tmp,$@)

linkerscript.phony:
.PHONY: linkerscript.phony

LKLOADER_LINKER_SCRIPT := $(BUILDDIR)/lk_loader_linker.ld

LKLOADER_BIN := $(BUILDDIR)/lk_loader
LKLOADER_ELF := $(BUILDDIR)/lk_loader.elf

LKLOADER_CSRCS := $(filter %.c,$(LKLOADER_SRCS))
LKLOADER_ASMSRCS := $(filter %.S,$(LKLOADER_SRCS))

LKLOADER_COBJS := $(call TOBUILDDIR,$(patsubst %.c,%.ldr.c.o,$(LKLOADER_CSRCS)))
LKLOADER_ASMOBJS := $(call TOBUILDDIR,$(patsubst %.S,%.ldr.S.o,$(LKLOADER_ASMSRCS)))

GEN_LK_PATH := $(BUILDDIR)/genfile
BIN2C_LK := $(GEN_LK_PATH)/lk_loader_lk.h

$(BIN2C_LK): $(OUTBIN)
	@$(MKDIR)
	$(info lk bin2c $@)
	bin2c $(OUTBIN) $(BIN2C_LK) lk

# lk_header.phony:
# .PHONY: lk_header.phony

GENERATED += $(BIN2C_LK)

$(LKLOADER_COBJS): $(BUILDDIR)/%.ldr.c.o: %.c $(BIN2C_LK)
	@$(MKDIR)
	$(info ldr compiling $@)
	$(CC) $(GLOBAL_OPTFLAGS) $(MODULE_OPTFLAGS) $(GLOBAL_COMPILEFLAGS) $(ARCH_COMPILEFLAGS) $(ARCH_COMPILEFLAGS_NOFLOAT) $(GLOBAL_CFLAGS) $(ARCH_CFLAGS) $(THUMBCFLAGS) $(GLOBAL_INCLUDES) -I$(GEN_LK_PATH) -c $< -MD -MP -MT $@ -MF $(@:%o=%d) -o $@

$(LKLOADER_ASMOBJS): $(BUILDDIR)/%.ldr.S.o: %.S
	@$(MKDIR)
	$(info ldr compiling $<)
	$(CC) $(GLOBAL_OPTFLAGS) $(MODULE_OPTFLAGS) $(GLOBAL_COMPILEFLAGS) $(ARCH_COMPILEFLAGS) $(ARCH_COMPILEFLAGS_NOFLOAT) $(GLOBAL_ASMFLAGS) $(ARCH_ASMFLAGS) $(MODULE_ASMFLAGS) $(THUMBCFLAGS) $(GLOBAL_INCLUDES) -c $< -MD -MP -MT $@ -MF $(@:%o=%d) -o $@

LKLOADER_OBJECT := $(call TOBUILDDIR,lk_loader.o)
$(LKLOADER_OBJECT): $(LKLOADER_COBJS) $(LKLOADER_ASMOBJS)
	@$(MKDIR)
	$(info ldr linking $@)
	$(LD) $(GLOBAL_MODULE_LDFLAGS) -r $^ -o $@

$(LKLOADER_ELF): $(LKLOADER_COBJS) $(LKLOADER_ASMOBJS) $(LKLOADER_LINKER_SCRIPT)
	$(info ldr linking $@)
	$(SIZE) -t --common $(sort $(LKLOADER_COBJS) $(LKLOADER_ASMOBJS))
	$(LD) $(GLOBAL_LDFLAGS) $(ARCH_LDFLAGS) -d -T $(LKLOADER_LINKER_SCRIPT) $(LKLOADER_COBJS) $(LKLOADER_ASMOBJS) -o $@

$(LKLOADER_BIN): $(LKLOADER_ELF)
	$(info ldr generating image: $@)
	$(SIZE) $<
	$(OBJCOPY) -O binary $< $@

# take the result of the build and generate a ipl file
LK_IPL := $(basename $(OUTBIN)).ipl
MAKE_IPL_TOOL := $(LOCAL_DIR)/tools/make_ipl.py
FAMILY_ID := 0xe48bff56 # UF2 family id
$(LK_IPL): $(OUTBIN) $(LKLOADER_BIN) $(MAKE_IPL_TOOL)
	@$(MKDIR)
	echo generating $@; \
	$(MAKE_IPL_TOOL) $(LKLOADER_BIN) $(LK_IPL) reset_block

EXTRA_BUILDDEPS += $(LK_IPL)
GENERATED += $(LK_IPL)

include make/module.mk
