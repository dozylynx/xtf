# Architecture specific files compiled and linked for arm64

# Include arm common files
include $(ROOT)/build/arm-common-files.mk

# Specific files for arm64
obj-perenv += $(ROOT)/arch/arm/arm64/cache.o
