CC     = g++
CFLAGS = -Ofast -g -Og -std=c++17
EXE    = bff-tool

.PHONY: all clean

all: $(EXE)

OBJS= \
	bff_main.o \
	common.o \
	write.o

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

# unix: -lz -lbz2
# win32: -L. -lzlib /usr/x86_64-w64-mingw32/lib/libbz2.a -static-libgcc -static-libstdc++
$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXE) -lz -lbz2

clean:
	rm $(OBJS)
	rm $(EXE)