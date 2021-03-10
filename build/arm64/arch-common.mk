# Architecture specific configuration for arm64

ARCH_PATH := $(ROOT)/arch/arm
ALL_ENVIRONMENTS := arm64

arm64_arch   := arm64
arm64_guest  := arm64
defcfg-arm64 := $(ROOT)/config/default-arm.cfg.in

# Compilation recipe for arm64
# arm64 needs linking normally, then converting to a binary format
define build-arm64
	$(CC) $$(LDFLAGS_arm64) $$(DEPS-arm64) -o $$@-syms
	$(OBJCOPY) $$@-syms -O binary $$@
endef
