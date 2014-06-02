########################################################################
# Makefile for vrprotein.
# Copyright (c) 2014 Pablo Cruz
# Adapted from Vrui Example Makefiles. 
#
########################################################################

# Directory containing the Vrui build system. The directory below
# matches the default Vrui installation; if Vrui's installation
# directory was changed during Vrui's installation, the directory below
# must be adapted.
VRUI_MAKEDIR := $(HOME)/Vrui-3.0/share/make

# Base installation directory for the example programs. If this is set
# to the default of $(PWD), the example programs do not have to be
# installed to be run. Created executables and resources will be
# installed in the bin and share directories under the given base
# directory, respectively.
# Important note: Do not use ~ as an abbreviation for the user's home
# directory here; use $(HOME) instead.
INSTALLDIR := $(shell pwd)

########################################################################
# Everything below here should not have to be changed
########################################################################

# Include definitions for the system environment and system-provided
# packages
include $(VRUI_MAKEDIR)/SystemDefinitions
include $(VRUI_MAKEDIR)/Packages.System
include $(VRUI_MAKEDIR)/Configuration.Vrui
include $(VRUI_MAKEDIR)/Packages.Vrui

# Set installation directory structure:
EXECUTABLEINSTALLDIR = $(INSTALLDIR)/$(EXEDIR)
ETCINSTALLDIR = $(INSTALLDIR)/$(CONFIGDIR)

########################################################################
# Specify additional compiler and linker flags
########################################################################

CFLAGS += -Wall -Wextra -Wno-unused-parameter -pedantic -std=c++0x
#DEBUG: Compile with trace symbols
CFLAGS += -g -rdynamic
# More warnings:
CFLAGS += -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization \
          -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations \
          -Wmissing-include-dirs -Wnoexcept -Wold-style-cast \
          -Woverloaded-virtual -Wredundant-decls -Wshadow -Wstrict-null-sentinel \
          -Wstrict-overflow=5 -Wswitch-default -Wundef \
          -Wmissing-field-initializers \
          -Wpointer-arith -Wwrite-strings
# -Wzero-as-null-pointer-constant (disabled; has false positives)

########################################################################
# List common packages used by all components of this project
# (Supported packages can be found in $(VRUI_MAKEDIR)/Packages.*)
########################################################################

PACKAGES = MYVRUI

########################################################################
# Specify all final targets
########################################################################

ALL = $(EXEDIR)/vrprotein

PHONY: all
all: $(ALL)

########################################################################
# Specify other actions to be performed on a `make clean'
########################################################################

.PHONY: extraclean
extraclean:

.PHONY: extrasqueakyclean
extrasqueakyclean:

# Include basic makefile
include $(VRUI_MAKEDIR)/BasicMakefile

########################################################################
# Specify build rules for executables
########################################################################

APP_SOURCES = $(wildcard src/*.cpp)

# $(OBJDIR)/NanotechConstructionKit.o: CFLAGS += -DNANOTECHCONSTRUCTIONKIT_CFGFILENAME='"$(ETCINSTALLDIR)/NCK.cfg"'

$(EXEDIR)/vrprotein: $(APP_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: vrprotein
vrprotein: $(EXEDIR)/vrprotein

install: $(ALL)
	@echo Installing vrprotein in $(INSTALLDIR)...
	@install -d $(INSTALLDIR)
	@install -d $(EXECUTABLEINSTALLDIR)
	@install $(ALL) $(EXECUTABLEINSTALLDIR)
	@install -d $(ETCINSTALLDIR)
#	@install -m u=rw,go=r $(CONFIGDIR)/NCK.cfg $(ETCINSTALLDIR)
