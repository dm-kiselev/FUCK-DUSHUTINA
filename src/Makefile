DEBUG = -g
CC = qcc
LD = qcc
FLAGS =  -Wc,-std=c99 -lang-c -D _QNX_SOURCE

TARGET = -Vgcc_ntox86

CFLAGS += $(DEBUG) $(TARGET) $(FLAGS)
LDFLAGS+= $(DEBUG) $(TARGET) $(FLAGS)

BINS = server client

all: $(BINS)

clean:
	rm -f *.o *.tmp *.so *.d *.i *.a *.s $(BINS)

spline:
	$(CC) $(CFLAGS) -o spline.o -c spline.c
	$(CC) $(LDFLAGS) -shared -o libspline.so spline.o
	
utils:
	$(CC) $(CFLAGS) -o utils.o -c utils.c
	
server: spline net
	$(CC) $(CFLAGS) -P -o server.i -c server.c
	$(CC) $(CFLAGS) -o server.o -c server.c
	$(CC) $(LDFLAGS) -o server server.o net.o -L./ -lspline
	
client: net utils
	$(CC) $(CFLAGS) -P -o client.i -c client.c
	$(CC) $(CFLAGS) -o client.o -c client.c 
	$(CC) $(LDFLAGS) -o client client.o net.o utils.o -lm
	
net:
	$(CC) $(CFLAGS) -o net.o -c net.c 