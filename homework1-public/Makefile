PROJECT=router
SOURCES=router.cpp lib/lib.c
LIBRARY=nope
INCPATHS=include
LIBPATHS=.
LDFLAGS=
CFLAGS=-g -c -Wall -Werror -Wno-error=unused-variable
CC=g++

# Automatic generation of some important lists
OBJECTS=$(SOURCES:.cpp=.o)
OBJECTS+=$(SOURCES:.c=.o)
OBJECTS:=$(filter %.o,$(OBJECTS))

INCFLAGS=$(foreach TMP,$(INCPATHS),-I$(TMP))
LIBFLAGS=$(foreach TMP,$(LIBPATHS),-L$(TMP))

# Set up the output file names for the different output types
BINARY=$(PROJECT)

all: $(SOURCES) $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) $(LIBFLAGS) $(OBJECTS) $(LDFLAGS) -g -o $@

.c.o:
	$(CC) $(INCFLAGS) $(CFLAGS) -g -fPIC $< -o $@

clean:

	rm -rf router hosts_output $(OBJECTS) router_*

run_router0: all
	./router rtable0.txt rr-0-1 r-0 r-1

run_router1: all
	./router rtable1.txt rr-0-1 r-0 r-1
