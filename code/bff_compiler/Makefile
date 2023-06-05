VERSION       = 1
VERSION_UPDATE= 1
VERSION_PATCH = 0
CC            = g++
CFLAGS        = -D_NOMAD_VERSION=$(VERSION) -D_NOMAD_VERSION_UPDATE=$(VERSION_UPDATE) -D_NOMAD_VERSION_PATCH=$(VERSION_PATCH) -DBFF_COMPILER \
				-Og -g -std=c++17
EXE           = bff-tool

.PHONY: all clean

all: $(EXE)

OBJS= \
	bff_main.o \
	common.o \
	script.o \
	sound.o \
	texture.o \
	level.o \
	write.o \
	zone.o

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXE)

clean:
	rm $(OBJS)
	rm $(EXE)