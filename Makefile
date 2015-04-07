# makefile
CC = gcc
CFLAGS = -D__USE_INLINE__
LIBS = -lauto
DEPS = 
OBJ = dt2chunkybas.o 

all: dt2chunkybas

%.o: %.c $(DEPS)
	 $(CC) -c -o $@ $< $(CFLAGS)

dt2chunkybas: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
