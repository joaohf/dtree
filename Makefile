CC := gcc
AR := ar
CFLAGS := -std=gnu99 -Wall -pedantic -Wextra -g -fPIC

Q ?= @

all: libdtree.a libdtree.so
libdtree.a: dtree_error.o dtree_procfs.o dtree_properties.o dtree.o bcd_arith.o
	$(Q) $(AR) rcs $@ $^

libdtree.so: dtree_error.o dtree_procfs.o dtree_properties.o dtree.o bcd_arith.o
	$(Q) $(CC) -shared -o $@ $^

busio: busio.o
	$(CC) $(LDFLAGS) $^ -L. -ldtree -o $@
busio.o: busio.c

lua-test:
	$(CC) -o lua-test -DTEST lua_dtree.c -llua -L. -ldtree

install-lib:
	install -d $(DESTDIR)/usr/lib
	install -m 0644 libdtree.so $(DESTDIR)/usr/lib/

install-includes:
	install -d $(DESTDIR)/usr/include
	install -m 0644 dtree.h $(DESTDIR)/usr/include/

install: all install-lib install-includes

clean:
	$(Q) $(RM) *.o

distclean: clean
	$(Q) $(RM) libdtree.a
	$(Q) $(RM) libdtree.so
	$(Q) $(RM) busio
