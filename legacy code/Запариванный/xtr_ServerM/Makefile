DEBUG = -g
CC = qcc
LD = qcc
FLAGS = -Wc,-std=c99 -lang-c

CFLAGS += $(DEBUG) $(TARGET) $(FLAGS)
LDFLAGS+= $(DEBUG) $(TARGET) $(FLAGS)

BINS = logger



all: $(BINS)

logger: $(CC) $(CFLAGS)  -o server -ñ xtr_Server.cc

clean:
	rm -f *.o *.tmp *.so $(BINS)