include project/target/psp-allegrex.mk

MODULES += \
    lib/gfx \
	lib/font \
	lib/miniz \
	lib/mbedtls \
	lib/ff15 \
	lib/fdisk \
	lib/vsync \
	lib/gamepad \
	lib/evflag \
	app/aiseirigh

GLOBAL_COMPILEFLAGS += -flto -ffat-lto-objects