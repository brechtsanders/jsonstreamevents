ifeq ($(OS),)
OS = $(shell uname -s)
endif
PREFIX = /usr/local
CC   = gcc
AR   = ar
LIBPREFIX = lib
LIBEXT = .a
ifeq ($(OS),Windows_NT)
BINEXT = .exe
SOLIBPREFIX =
SOEXT = .dll
else ifeq ($(OS),Darwin)
BINEXT =
SOLIBPREFIX = lib
SOEXT = .dylib
else
BINEXT =
SOLIBPREFIX = lib
SOEXT = .so
endif
CFLAGS = -Os
COMMON_CFLAGS = -Iinclude
STATIC_CFLAGS = $(COMMON_CFLAGS) -DBUILD_JSONSTREAMEVENTS_STATIC
SHARED_CFLAGS = $(COMMON_CFLAGS) -DBUILD_JSONSTREAMEVENTS_DLL
LIBS =
LDFLAGS =
ifeq ($(OS),Darwin)
STRIPFLAG =
else
STRIPFLAG = -s
endif
MKDIR = mkdir -p
RM = rm -f
RMDIR = rm -rf
CP = cp -f
CPDIR = cp -rf
DOXYGEN = $(shell which doxygen)

OSALIAS := $(OS)
ifeq ($(OS),Windows_NT)
ifneq (,$(findstring x86_64,$(shell gcc --version)))
OSALIAS := win64
else
OSALIAS := win32
endif
endif

LIBJSONSTREAMEVENTS_OBJ = src/jsonstreamevents.o
LIBJSONSTREAMEVENTS_LDFLAGS = 
LIBJSONSTREAMEVENTS_SHARED_LDFLAGS =
ifneq ($(OS),Windows_NT)
SHARED_CFLAGS += -fPIC
endif
ifeq ($(OS),Windows_NT)
LIBJSONSTREAMEVENTS_SHARED_LDFLAGS += -Wl,--out-implib,$(LIBPREFIX)$@$(LIBEXT) -Wl,--output-def,$(@:%$(SOEXT)=%.def)
endif
ifeq ($(OS),Darwin)
OS_LINK_FLAGS = -dynamiclib -o $@
else
OS_LINK_FLAGS = -shared -Wl,-soname,$@ $(STRIPFLAG)
endif

YAJL_CFLAGS =
YAJL_LDFLAGS = -lyajl

LAXJSON_CFLAGS =
LAXJSON_LDFLAGS = -llaxjson

ifeq ($(LAXJSON),)
DEPENDENCY_CFLAGS += -DUSE_LIBYAJL $(YAJL_CFLAGS)
DEPENDENCY_LDFLAGS += $(YAJL_LDFLAGS)
else
DEPENDENCY_CFLAGS += -DLIBLAXJSON $(LAXJSON_CFLAGS)
DEPENDENCY_LDFLAGS += $(LAXJSON_LDFLAGS)
endif

UTILS_BIN = 
EXAMPLES_BIN = examples/jsonstreamevents_show_data$(BINEXT) examples/jsonstreamevents_manual_match$(BINEXT) examples/jsonstreamevents_match$(BINEXT)

COMMON_PACKAGE_FILES = README.md LICENSE Changelog.txt
SOURCE_PACKAGE_FILES = $(COMMON_PACKAGE_FILES) Makefile doc/Doxyfile include/*.h src/*.c build/*.workspace build/*.cbp build/*.depend

default: all

all: static-lib shared-lib utils examples

#%.o: %.c
#	$(CC) -c -o $@ $< $(CFLAGS) 

%.static.o: %.c
	$(CC) -c -o $@ $< $(STATIC_CFLAGS) $(DEPENDENCY_CFLAGS) $(CFLAGS) 

%.shared.o: %.c
	$(CC) -c -o $@ $< $(SHARED_CFLAGS) $(DEPENDENCY_CFLAGS) $(CFLAGS)

static-lib: $(LIBPREFIX)jsonstreamevents$(LIBEXT)

shared-lib: $(SOLIBPREFIX)jsonstreamevents$(SOEXT)

$(LIBPREFIX)jsonstreamevents$(LIBEXT): $(LIBJSONSTREAMEVENTS_OBJ:%.o=%.static.o)
	$(AR) cr $@ $^

$(SOLIBPREFIX)jsonstreamevents$(SOEXT): $(LIBJSONSTREAMEVENTS_OBJ:%.o=%.shared.o)
	$(CC) -o $@ $(OS_LINK_FLAGS) $^ $(LIBJSONSTREAMEVENTS_SHARED_LDFLAGS) $(LIBJSONSTREAMEVENTS_LDFLAGS) $(DEPENDENCY_LDFLAGS) $(LDFLAGS) $(LIBS)

utils: $(UTILS_BIN)
ifeq ($(EXAMPLES),)
examples: 
else
examples: $(EXAMPLES_BIN)
endif

$(addsuffix .o,$(basename $(UTILS_BIN) $(EXAMPLES_BIN))): %.o: %.c
	$(CC) -c -o $@ $< $(SHARED_CFLAGS) $(CFLAGS)

$(UTILS_BIN) $(EXAMPLES_BIN): %$(BINEXT): %.o $(SOLIBPREFIX)jsonstreamevents$(SOEXT)
	$(CC) $(STRIPFLAG) -o $@ $^ $(LIBJSONSTREAMEVENTS_LDFLAGS) $(LDFLAGS)

.PHONY: doc
doc:
ifdef DOXYGEN
	$(DOXYGEN) doc/Doxyfile
endif

install: all doc
	$(MKDIR) $(PREFIX)/include $(PREFIX)/lib $(PREFIX)/bin
	$(CP) include/*.h $(PREFIX)/include/
	$(CP) *$(LIBEXT) $(PREFIX)/lib/
ifneq ($(UTILS_BIN),)
	$(CP) $(UTILS_BIN) $(PREFIX)/bin/
endif
ifeq ($(OS),Windows_NT)
	$(CP) *$(SOEXT) $(PREFIX)/bin/
	$(CP) *.def $(PREFIX)/lib/
else
	$(CP) *$(SOEXT) $(PREFIX)/lib/
endif
ifdef DOXYGEN
	$(CPDIR) doc/man $(PREFIX)/
endif

.PHONY: version
version:
	sed -ne "s/^#define\s*JSONSTREAMEVENTS_VERSION_[A-Z]*\s*\([0-9]*\)\s*$$/\1./p" include/jsonstreamevents.h | tr -d "\n" | sed -e "s/\.$$//" > version

.PHONY: package
package: version
	tar cfJ jsonstreamevents-$(shell cat version).tar.xz --transform="s?^?jsonstreamevents-$(shell cat version)/?" $(SOURCE_PACKAGE_FILES)

.PHONY: package
binarypackage: version
ifneq ($(OS),Windows_NT)
	$(MAKE) PREFIX=binarypackage_temp_$(OSALIAS) install
	tar cfJ jsonstreamevents-$(shell cat version)-$(OSALIAS).tar.xz --transform="s?^binarypackage_temp_$(OSALIAS)/??" $(COMMON_PACKAGE_FILES) binarypackage_temp_$(OSALIAS)/*
else
	$(MAKE) PREFIX=binarypackage_temp_$(OSALIAS) install DOXYGEN=
	cp -f $(COMMON_PACKAGE_FILES) binarypackage_temp_$(OSALIAS)
	rm -f jsonstreamevents-$(shell cat version)-$(OSALIAS).zip
	cd binarypackage_temp_$(OSALIAS) && zip -r9 ../jsonstreamevents-$(shell cat version)-$(OSALIAS).zip $(COMMON_PACKAGE_FILES) * && cd ..
endif
	rm -rf binarypackage_temp_$(OSALIAS)

.PHONY: clean
clean:
	$(RM) src/*.o example/*.o *$(LIBEXT) *$(SOEXT) $(UTILS_BIN) version jsonstreamevents-*.tar.xz doc/doxygen_sqlite3.db
ifeq ($(OS),Windows_NT)
	$(RM) *.def
endif
	$(RMDIR) doc/html doc/man

