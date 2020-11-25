MAKEFLAGS += -rR
ROOT := $(abspath $(CURDIR))
export ROOT

# Default to the all rule
all:

# Local settings and rules
-include Makefile.local

# $(xtfdir) defaults to $(ROOT) so development and testing can be done
# straight out of the working tree.
xtfdir  ?= $(ROOT)
DESTDIR ?= $(ROOT)/dist

ifeq ($(filter /%,$(xtfdir)),)
$(error $$(xtfdir) must be absolute, not '$(xtfdir)')
endif

ifneq ($(DESTDIR),)
ifeq ($(filter /%,$(DESTDIR)),)
$(error $$(DESTDIR) must be absolute, not '$(DESTDIR)')
endif
endif

xtftestdir := $(xtfdir)/tests

# Supported architectures
SUPPORTED_ARCH := x86
# Default architecture
ARCH ?= x86
# Check if specified architecture is supported
ifeq ($(filter $(ARCH),$(SUPPORTED_ARCH)),)
$(error Architecture '$(ARCH)' not supported)
endif

export DESTDIR ARCH xtfdir xtftestdir

ifeq ($(LLVM),) # GCC toolchain
CC              := $(CROSS_COMPILE)gcc
LD              := $(CROSS_COMPILE)ld
OBJCOPY         := $(CROSS_COMPILE)objcopy

else # LLVM toolchain

# Optional -$NUM version when multiple toolchains are installed
ver := $(filter -%,$(LLVM))
CC              := clang$(ver) $(if $(CROSS_COMPILE),--target=$(notdir $(CROSS_COMPILE:%-=%)))
LD              := ld.lld$(ver)
OBJCOPY         := llvm-objcopy$(ver)
undefine ver

endif

CPP             := $(CC) -E
INSTALL         := install
INSTALL_DATA    := $(INSTALL) -m 644 -p
INSTALL_DIR     := $(INSTALL) -d -p
INSTALL_PROGRAM := $(INSTALL) -p

# Best effort attempt to find a python interpreter, defaulting to Python 3 if
# available.  Fall back to just `python` if `which` is nowhere to be found.
PYTHON_INTERPRETER := $(word 1,$(shell which python3 python python2 2>/dev/null) python)
PYTHON             ?= $(PYTHON_INTERPRETER)

export CC LD CPP INSTALL INSTALL_DATA INSTALL_DIR INSTALL_PROGRAM OBJCOPY PYTHON

# By default enable all the tests
TESTS ?= $(wildcard tests/*)

.PHONY: all
all:
	@set -e; for D in $(TESTS); do \
		[ ! -e $$D/Makefile ] && continue; \
		$(MAKE) -C $$D build; \
	done

.PHONY: install
install:
	@$(INSTALL_DIR) $(DESTDIR)$(xtfdir)
	$(INSTALL_PROGRAM) xtf-runner $(DESTDIR)$(xtfdir)
	@set -e; for D in $(TESTS); do \
		[ ! -e $$D/Makefile ] && continue; \
		$(MAKE) -C $$D install; \
	done

define all_sources
	find include/ arch/ common/ tests/ -name "*.[hcsS]"
endef

.PHONY: cscope
cscope:
	$(all_sources) > cscope.files
	cscope -b -q -k

.PHONY: gtags
gtags:
	$(all_sources) | gtags -f -

.PHONY: clean
clean:
	find . \( -name "*.o" -o -name "*.d" -o -name "*.lds" \) -delete
	find tests/ \( -perm -a=x -name "test-*" -o -name "test-*.cfg" \
		-o -name "info.json" \) -delete

.PHONY: distclean
distclean: clean
	find . \( -name "*~" -o -name "cscope*" \) -delete
	rm -rf docs/autogenerated/ dist/

.PHONY: doxygen
doxygen: Doxyfile
	doxygen Doxyfile > /dev/null

.PHONY: pylint
pylint:
	-pylint --rcfile=.pylintrc xtf-runner
