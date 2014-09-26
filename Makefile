CC = gcc

CFLAGS = -std=gnu99 -O3 -g -MD -rdynamic

AM_CFLAGS = -Wall

LIBS = -lm -lrt

TARGET = fsrt

OBJS = tool.o send.o

all: $(TARGET)
# ----------- Application --------------------------------------------------
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)
# ----------- OBJS ---------------------------------------------------------
.c.o:
	$(CC) ${CFLAGS} $(AM_CFLAGS)  -c -o $@ $< $(LIBS)

sources = *.c
-include $(sources:.c=.d) 
# ----------- CLEAN --------------------------------------------------------
clean:
	$(RM) *.o *.d fsrt