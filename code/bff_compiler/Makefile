CC     = g++
CFLAGS = -Ofast -s -std=c++17
EXE    = bff-tool

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
	$(CC) $(CFLAGS) $(OBJS) -o $(EXE) -lbz2

clean:
	rm $(OBJS)
	rm $(EXE)