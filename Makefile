
PREFIX := /usr/local
LIBDIR := $(PREFIX)/lib
BINDIR := $(PREFIX)/bin
SBINDIR := $(PREFIX)/sbin
INCLUDEDIR := $(PREFIX)/include
LIBINCLUDEDIR := $(INCLUDEDIR)
MANDIR := $(PREFIX)/man

# Build and install static library
BUILD_STATIC_LIB := 1

# Some often-used commands with default options
MKDIR := mkdir -p
RMDIR := rmdir
RM := rm -f
MV := mv -f
CC := gcc
AR := ar
INSTALL := install
LN := ln -sf
GREP := grep
AWK := awk
SED := sed

CFLAGS := -std=gnu99 -Wall -pedantic -Wextra -g -fPIC

ALL_CPPFLAGS := -I.
ALL_CFLAGS := -Wall

ifeq ($(DEBUG),1)
ALL_CPPFLAGS += -DDEBUG
ALL_CFLAGS += -O -g
else
ALL_CFLAGS += -O2
endif

ifeq ($(WARN),1)
ALL_CFLAGS += -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
            -Wcast-align -Wwrite-strings -Wnested-externs -Winline -W \
            -Wmissing-prototypes -Wundef
endif

ALL_CPPFLAGS += $(CPPFLAGS)
ALL_CFLAGS += $(CFLAGS)

PROGCPPFLAGS := $(ALL_CPPFLAGS)
PROGCFLAGS := $(ALL_CFLAGS)
LIBCPPFLAGS := $(ALL_CPPFLAGS)
LIBCFLAGS := -fpic -D_REENTRANT $(ALL_CFLAGS)
ARCPPFLAGS := $(ALL_CPPFLAGS)
ARCFLAGS := $(ALL_CFLAGS)

Q ?= @

# The main and minor version of the library
# The library soname (major number) must be changed if and only if the interface is
# changed in a backward incompatible way.  The interface is defined by
# the public header files - in this case they are error.h and sensors.h.
LIBMAINVER := 0
LIBMINORVER := 2.1
LIBVER := $(LIBMAINVER).$(LIBMINORVER)

# The static lib name, the shared lib name, and the internal ('so') name of
# the shared lib.
LIBSHBASENAME := libdtree.so
LIBSHLIBNAME := libdtree.so.$(LIBVER)
LIBSTLIBNAME := libdtree.a
LIBSHSONAME := libdtree.so.$(LIBMAINVER)

LIBCSOURCES := dtree_error.c dtree_procfs.c dtree_properties.c dtree.c bcd_arith.c
LIBOTHEROBJECTS :=

LIBSHOBJECTS := $(LIBCSOURCES:.c=.lo) $(LIBOTHEROBJECTS:.o=.lo)
LIBSTOBJECTS := $(LIBCSOURCES:.c=.ao) $(LIBOTHEROBJECTS:.o=.ao)

LIBHEADERFILES := dtree.h

LIBTARGETS := $(LIBSHLIBNAME) $(LIBSHSONAME) $(LIBSHBASENAME)
ifeq ($(BUILD_STATIC_LIB),1)
LIBTARGETS += $(LIBSTLIBNAME)
endif

all: $(LIBTARGETS)

# How to create the shared library
$(LIBSHLIBNAME): $(LIBSHOBJECTS)
	$(CC) -shared $(LDFLAGS) -Wl,--version-script=libdtree.map -Wl,-soname,$(LIBSHSONAME) -o $@ $^

$(LIBSHSONAME): $(LIBSHLIBNAME)
	$(RM) $@
	$(LN) $(LIBSHLIBNAME) $@

$(LIBSHBASENAME): $(LIBSHLIBNAME)
	$(RM) $@ 
	$(LN) $(LIBSHLIBNAME) $@

# And the static library
$(LIBSTLIBNAME): $(LIBSTOBJECTS)
	$(RM) $@
	$(AR) rcvs $@ $^

busio: busio.o
	$(CC) $(LDFLAGS) $^ -L. -ldtree -o $@
busio.o: busio.c

lua-test:
	$(CC) -o lua-test -DTEST lua_dtree.c -llua -L. -ldtree

install-lib:
	$(MKDIR) $(DESTDIR)$(LIBDIR)
ifeq ($(BUILD_STATIC_LIB),1)
	$(INSTALL) -m 644 $(LIBSTLIBNAME) $(DESTDIR)$(LIBDIR)
endif
	$(INSTALL) -m 755 $(LIBSHLIBNAME) $(DESTDIR)$(LIBDIR)
	$(LN) $(LIBSHLIBNAME) $(DESTDIR)$(LIBDIR)/$(LIBSHSONAME)
	$(LN) $(LIBSHSONAME) $(DESTDIR)$(LIBDIR)/$(LIBSHBASENAME)

install-includes:
	$(MKDIR) $(DESTDIR)$(LIBINCLUDEDIR)
	$(INSTALL) -m 644 $(LIBHEADERFILES) $(DESTDIR)$(LIBINCLUDEDIR)	

install: all install-lib install-includes

clean:
	$(RM) *.o 
	$(RM) *.ld *.ad
	$(RM) *.lo *.ao
	$(RM) $(LIBTARGETS) $(LIBEXTRACLEAN)

distclean: clean
	$(Q) $(RM) libdtree.a
	$(Q) $(RM) libdtree.so
	$(Q) $(RM) busio

.SUFFIXES:

# We need to create dependency files. Tricky. The sed rule puts dir/file.d and
# dir/file.c in front of the dependency file rule.


# .ro files are used for programs (as opposed to modules)
%.ro: %.c
	$(CC) $(PROGCPPFLAGS) $(PROGCFLAGS) -c $< -o $@

%.rd: %.c
	$(CC) -M -MG $(PROGCPPFLAGS) $(PROGCFLAGS) $< | \
	$(SED) -e 's@^\(.*\)\.o:@$*.rd $*.ro: Makefile '`dirname $*.rd`/Module.mk' @' > $@

# .ao files are used for static archives
%.ao: %.c
	$(CC) $(ARCPPFLAGS) $(ARCFLAGS) -c $< -o $@

%.ad: %.c
	$(CC) -M -MG $(ARCPPFLAGS) $(ARCFLAGS) $< | \
	$(SED) -e 's@^\(.*\)\.o:@$*.ad $*.ao: Makefile '`dirname $*.ad`/Module.mk' @' > $@

# .lo files are used for shared libraries
%.lo: %.c
	$(CC) $(LIBCPPFLAGS) $(LIBCFLAGS) -c $< -o $@

%.ld: %.c
	$(CC) -M -MG $(LIBCPPFLAGS) $(LIBCFLAGS) $< | \
	$(SED) -e 's@^\(.*\)\.o:@$*.ld $*.lo: Makefile '`dirname $*.ld`/Module.mk' @' > $@
