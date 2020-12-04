# ----------------------------------------------------------- -*- mode: makefile -*-
# Config
# ----------------------------------------------------------------------------

VERS       ?= 0.0.0
ROOT       ?= /tmp/nemo-qml-plugin-dbus
PLUGIN_DIR ?= /opt/tests/nemo-qml-plugins-qt5/dbus/
DBUS_DIR   ?= /usr/share/dbus-1

DBUS_GMAIN_DIR := dbus-gmain

# ----------------------------------------------------------------------------
# DBUS_GMAIN
# ----------------------------------------------------------------------------

# The dbus-gmain submodule contains sources that have
# issues and do not compile cleanly. As the content is
# what it is, silence warnings etc when compiling source
# files from there...

DBUS_GMAIN_CPPFLAGS += -I.
DBUS_GMAIN_CFLAGS   += -Wno-unused-parameter
DBUS_GMAIN_CFLAGS   += -Wno-cast-function-type
DBUS_GMAIN_CFLAGS   += -Wno-missing-field-initializers

$(DBUS_GMAIN_DIR)/%.o : CPPFLAGS += $(DBUS_GMAIN_CPPFLAGS)
$(DBUS_GMAIN_DIR)/%.o : CFLAGS   += $(DBUS_GMAIN_CFLAGS)

# ----------------------------------------------------------------------------
# List of targets to build
# ----------------------------------------------------------------------------

TARGETS += dbustestd

# ----------------------------------------------------------------------------
# Top level targets
# ----------------------------------------------------------------------------

.PHONY: build install clean distclean mostlyclean

build:: $(TARGETS)

install:: build

clean:: mostlyclean
	$(RM) $(TARGETS)

distclean:: clean

mostlyclean::
	$(RM) *.o *~ *.bak

# ----------------------------------------------------------------------------
# Default flags
# ----------------------------------------------------------------------------

CPPFLAGS += -D_GNU_SOURCE
CPPFLAGS += -D_FILE_OFFSET_BITS=64

COMMON   += -Wall
COMMON   += -Os
COMMON   += -g

CFLAGS   += $(COMMON)
CFLAGS   += -std=c99

CXXFLAGS += $(COMMON)

LDFLAGS  += -g

LDLIBS   += -Wl,--as-needed

# ----------------------------------------------------------------------------
# Flags from pkg-config
# ----------------------------------------------------------------------------

PKG_NAMES += glib-2.0
PKG_NAMES += dbus-1

maintenance  = clean distclean mostlyclean
intersection = $(strip $(foreach w,$1, $(filter $w,$2)))
ifneq ($(call intersection,$(maintenance),$(MAKECMDGOALS)),)
PKG_CONFIG   ?= true
endif

ifneq ($(strip $(PKG_NAMES)),)
PKG_CONFIG   ?= pkg-config
PKG_CFLAGS   := $(shell $(PKG_CONFIG) --cflags $(PKG_NAMES))
PKG_LDLIBS   := $(shell $(PKG_CONFIG) --libs   $(PKG_NAMES))
PKG_CPPFLAGS := $(filter -D%,$(PKG_CFLAGS)) $(filter -I%,$(PKG_CFLAGS))
PKG_CFLAGS   := $(filter-out -I%, $(filter-out -D%, $(PKG_CFLAGS)))
endif

CPPFLAGS += $(PKG_CPPFLAGS)
CFLAGS   += $(PKG_CFLAGS)
LDLIBS   += $(PKG_LDLIBS)

# ----------------------------------------------------------------------------
# Explicit dependencies
# ----------------------------------------------------------------------------

dbustestd : dbustestd.o $(DBUS_GMAIN_DIR)/dbus-gmain.o

install::
	install -m755 -d $(ROOT)$(PLUGIN_DIR)/bin
	install -m755 dbustestd $(ROOT)$(PLUGIN_DIR)/bin/

	install -m755 -d $(ROOT)$(DBUS_DIR)/services/
	install -m755 dbus/org.nemomobile.dbustestd.service $(ROOT)$(DBUS_DIR)/services/
