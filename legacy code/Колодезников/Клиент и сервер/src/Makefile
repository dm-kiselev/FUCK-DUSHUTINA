DEBUG = -g
CC = qcc
LD = qcc
FLAGS =  -Wc,-std=c99 -lang-c -D _QNX_SOURCE

TARGET = -Vgcc_ntox86
#TARGET = -Vgcc_ntoarmv7le

CFLAGS += $(DEBUG) $(TARGET) $(FLAGS)
LDFLAGS+= $(DEBUG) $(TARGET) $(FLAGS)

BINS = test_lib server client testspline

all: $(BINS)

clean:
	rm -f *.o *.tmp *.so *.d *.i *.a *.s $(BINS)

spline:
	$(CC) $(CFLAGS) -o spline.o -c spline.c
	$(CC) $(LDFLAGS) -shared -o libspline.so spline.o
	#$(CC) $(LDFLAGS) -a libspline.a spline.o
	
logger:
	$(CC) $(CFLAGS) -o logger.o -c logger.c
	
utils:
	$(CC) $(CFLAGS) -o utils.o -c utils.c

test_lib: spline
	$(CC) $(CFLAGS) -P -o test_lib.i -c test_lib.c
	$(CC) $(CFLAGS) -o test_lib.o -c test_lib.c
	$(CC) $(LDFLAGS) -o test_lib test_lib.o -L./ -lspline
	
server: spline logger net
	$(CC) $(CFLAGS) -P -o server.i -c server.c
	$(CC) $(CFLAGS) -o server.o -c server.c
	$(CC) $(LDFLAGS) -o server server.o net.o logger.o -L./ -lspline
	
client: net utils
	$(CC) $(CFLAGS) -P -o client.i -c client.c
	$(CC) $(CFLAGS) -o client.o -c client.c 
	$(CC) $(LDFLAGS) -o client client.o net.o utils.o -lm
	
net:
	$(CC) $(CFLAGS) -o net.o -c net.c 
	
testspline: spline
	$(CC) $(CFLAGS) -P -o testspline.i -c testspline.c
	$(CC) $(CFLAGS) -o testspline.o -c testspline.c
	$(CC) $(LDFLAGS) -o testspline testspline.o -lm -L./ -lspline