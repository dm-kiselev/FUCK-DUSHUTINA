
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <sys/siginfo.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>

#define srvName "InterpolationSrv"

// pulses definition
#define TIMER_CODE		1		// pulse from timer
// messages from client definition
#define MT_SEND_DATA	2       // message from client
// replies to client definition
#define MT_OK			3
#define MT_TIMEOUT		4
#define MT_ERROR        5

typedef struct {
	double x;
	double y;
} point;

// message structure
typedef struct {
    // contains both message to and from client
    int messageType;
    // optional data, depending upon message
    point data;
} ClientMessageT;

typedef union {
    // a message can be either from a client, or a pulse
    ClientMessageT  msg;
    struct _pulse   pulse;
} MessageT;

using namespace std;
