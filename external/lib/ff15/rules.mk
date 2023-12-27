LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/source

MODULE_SRCS += \
	$(LOCAL_DIR)/source/pspio.c \
	$(LOCAL_DIR)/source/ff.c \
	$(LOCAL_DIR)/source/ffsystem.c \
	$(LOCAL_DIR)/source/ffunicode.c

include make/module.mk
