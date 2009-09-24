
TOP=

APACHE_INSTALL=/usr/local/apache2
TARGET=libmite
INCLUDES=-I. -I../thirdparty/sqlite3 -I$(APACHE_INSTALL)/include \
	-I../thirdparty/yajl/build/yajl-1.0.5/include
MITE_SOURCES=blackbox.c db.c http.c json.c meta.c soap.c sql.c \
	url.c vtab.c xml.c
LIBMITE_OBJ= blackbox.o db.o http.o json.o meta.o soap.o sql.o url.o vtab.o \
	xml.o
LIBS=-lc
LIB_DIR=
LD=ld
LD_LINK=$(LD) $(LDFLAGS) -L. $(LIB_DIRS) $(LIBS) -o $@
VERSION=1.0
DISTDIR=$(TOP)$(TARGET)-$(VERSION)
TOP_BUILDDIR=
# these are from Apache 2, probably should use autoconf/automake
# to get these automatically; needed to include APR headers
# (and link the APR libraries)
DEFS=-DLINUX=2 -D_REENTRANT -D_GNU_SOURCE -D_LARGEFILE64_SOURCE
CC_SHARED=gcc
C_SHARED_COMPILE=$(CC_SHARED) -shared $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)

DEP_FILES=.deps/blackbox.P .deps/db.P .deps/http.P .deps/json.P .deps/meta.P \
	.deps/soap.P .deps/sql.P .deps/url.P .deps/vtab.P .deps/xml.P \
	.deps/db.P .deps/http.P .deps/json.P .deps/meta.P .deps/options.P \
	.deps/soap.P .deps/sql.P .deps/url.P .deps/util.P .deps/vtab.P \
	.deps/wsdl.P .deps/xml.P

all: $(TARGET).so

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)
-include $(DEP_FILES)

%.o: %.c
	@echo '$(C_SHARED_COMPILE) -c $<'; \
	$(C_SHARED_COMPILE) $(C_DEPENDENCIES) -o $@ -c $<

libmite.so: $(LIBMITE_OBJ)
	$(LD_LINK) $^

tags: 


clean:
	rm -f *.o *.obj .deps/*.P .lo

.PHONY: dist

dist:
	rm -rf $(DISTDIR)
	mkdir $(DISTDIR)
	cp $(MITE_SOURCES) Makefile $(DISTDIR)
	tar -cvzf $(DISTDIR).tar.gz $(DISTDIR)
	rm -rf $(DISTDIR)
