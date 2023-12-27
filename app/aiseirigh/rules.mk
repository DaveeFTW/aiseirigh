LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/main.c \
	$(LOCAL_DIR)/screen.c \
	$(LOCAL_DIR)/textraster.c \
	$(LOCAL_DIR)/psar.c \
	$(LOCAL_DIR)/workbuffer.c \
	$(LOCAL_DIR)/crypt.c \
	$(LOCAL_DIR)/des.c \
	$(LOCAL_DIR)/personalisation.c \
	$(LOCAL_DIR)/prx.c \
	$(LOCAL_DIR)/prxtype2.c \
	$(LOCAL_DIR)/prxtype8.c \
	$(LOCAL_DIR)/primaryscreen.c \
	$(LOCAL_DIR)/ipl.c \
	$(LOCAL_DIR)/appstate.c \
	$(LOCAL_DIR)/fferr.c \
	$(LOCAL_DIR)/statemachine.c \
	$(LOCAL_DIR)/states/errorstate.c \
	$(LOCAL_DIR)/states/initstate.c \
	$(LOCAL_DIR)/states/menustate.c \
	$(LOCAL_DIR)/states/mountmsstate.c \
	$(LOCAL_DIR)/states/checkupdaterstate.c \
	$(LOCAL_DIR)/states/processpsarstate.c \
	$(LOCAL_DIR)/states/partitionflashstate.c \
	$(LOCAL_DIR)/states/formatflashstate.c \
	$(LOCAL_DIR)/states/mountflashstate.c \
	$(LOCAL_DIR)/states/createdirectoriesstate.c \
	$(LOCAL_DIR)/states/flashfilesstate.c \
	$(LOCAL_DIR)/states/finishstate.c

MODULE_INCLUDES += \
	$(LOCAL_DIR)

include make/module.mk
