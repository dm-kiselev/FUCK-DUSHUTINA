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
#define ATTACH_POINT "inter_server"
// message send definitions

// messages
#define MT_WAIT_DATA        2       // message from client
#define MT_SEND_DATA        3       // message from client
#define MT_ERROR        4      		 // error message to client
// pulses
#define CODE_TIMER          1       // pulse from timer

// message reply definitions
#define MT_OK               0       // message to client
#define MT_TIMEDOUT         1       // message to client
typedef struct {
	double x;
	double y;
} point;
// message structure
typedef struct
{
    // contains both message to and from client
    int messageType;
    // optional data, depending upon message
    point data;
} ClientMessageT;

typedef union
{
    // a message can be either from a client, or a pulse
    ClientMessageT  msg;
    struct _pulse   pulse;
} MessageT;

// client table
#define MAX_CLIENT 16       // max # of simultaneous clients
struct clients
{
    //int in_use;             // is this client entry in use?
    int rcvid;              // receive ID of client
    int timeout;            // timeout left for client
	clients *next;   // указатель на следующего клиента
};  //clients [MAX_CLIENT];   // client table
using namespace std;
clients *curr;
clients *head;
clients *back; //указатель на текущий, и последний последний элемент списка (FIFO)
int     chid;               // channel ID (global)
int     debug = 0;          // set debug value, 1=on, 0=off
char    *progname = "inter_server.c";
int pid;

// forward prototypes
static  void setupPulseAndTimer (void);
static  void gotAPulse (void);
static  void gotAMessage (int rcvid, ClientMessageT *msg);

int main( int argc, char *argv[] )
{
    int rcvid;              // process ID of the sender
    MessageT msg;           // the message itself
	curr = NULL;
	head = NULL;
	name_attach_t*	msg_chanel;

	unsigned int flag;

	if (argc < 2)
	{
		cerr << "Server: cmd args error!"  << endl;
		exit(1);
	}

	if (strcmp(argv[1],"-l") == 0)		flag = 0;
	else if (strcmp(argv[1],"-g") == 0)	flag = NAME_FLAG_ATTACH_GLOBAL;
	else {
		cerr << "Server: cmd args error!" << endl;
		exit(1);
	}
		// Регистрация канала //
	pid=getpid();
	msg_chanel = name_attach( NULL, ATTACH_POINT, flag );
	if (flag){
	rcvid=MsgReceive(msg_chanel->chid, &msg, sizeof(msg), NULL);
	MsgReply(rcvid, 0, &msg, sizeof(msg));
	}
	chid=msg_chanel->chid;
	if (msg_chanel == NULL) {
		cerr <<  "Server: name attach error!" << endl;
		exit(2);
	}

    // set up the pulse and timer
    setupPulseAndTimer ();
		printf("pid = %d, chid=%d\n", getpid(),chid );
    // receive messages
    for (;;) {
        rcvid = MsgReceive (msg_chanel->chid, &msg, sizeof (msg), NULL);

        // determine who the message came from
        if (rcvid == 0) {
            // production code should check "code" field...
            gotAPulse ();
        } else {
            gotAMessage (rcvid, &msg.msg);
        }
    }

    // you'll never get here
    return (EXIT_SUCCESS);
}

/*
 *  setupPulseAndTimer
 *
 *  This routine is responsible for setting up a pulse so it
 *  sends a message with code MT_TIMER.  It then sets up a
 *  periodic timer that fires once per second.
*/

void
setupPulseAndTimer (void)
{
    timer_t             timerid;    // timer ID for timer
    struct sigevent     event;      // event to deliver
    struct itimerspec   timer;      // the timer data structure
    int                 coid;       // connection back to ourselves

    // create a connection back to ourselves
    coid = name_open( ATTACH_POINT, 0 );;
    if (coid == -1) {
        fprintf (stderr, "%s:  couldn't ConnectAttach to self!\n",
                 progname);
        perror (NULL);
        exit (EXIT_FAILURE);
    }

    // set up the kind of event that we want to deliver -- a pulse
    SIGEV_PULSE_INIT (&event, coid,
                      SIGEV_PULSE_PRIO_INHERIT, CODE_TIMER, 0);

    // create the timer, binding it to the event
    if (timer_create (CLOCK_REALTIME, &event, &timerid) == -1) {
        fprintf (stderr, "%s:  couldn't create a timer, errno %d\n",
                 progname, errno);
        perror (NULL);
        exit (EXIT_FAILURE);
    }

    // setup the timer (1s delay, 1s reload)
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_nsec = 0;
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_nsec = 0;

    // and start it!
    timer_settime (timerid, 0, &timer, NULL);
}

/*
 *  gotAPulse
 *
 *  This routine is responsible for handling the fact that a
 *  timeout has occurred.  It runs through the list of clients
 *  to see which client has timed out, and replies to it with
 *  a timed-out response.
 */

void
gotAPulse (void)
{
    ClientMessageT  msg;
    int             i=0;
	clients * buff;
    if (debug) {
        time_t  now;

        time (&now);
        printf ("Got a Pulse at %s", ctime (&now));
    }

    // prepare a response message
    msg.messageType = MT_TIMEDOUT;


   curr = head;
   buff = curr;
   while(curr) {
      if (--curr->timeout == 0) {
          // send a reply
          MsgReply (curr->rcvid, EOK, &msg,sizeof (msg));
		  printf("Client %d timeout\n", i);

         if (!i)
		 {
			head = curr->next;
			free(curr);
			curr= head;
		 }
		 else
		 {
			buff->next=curr->next;
			free(curr);
			curr = buff->next;
		 }

      }
      else
	  {
		buff = curr;
		curr = curr->next ;
	  }
	  i++;
	  }
}
/*
 *  gotAMessage
 *
 *  This routine is called whenever a message arrives.  We
 *  look at the type of message (either a "wait for data"
 *  message, or a "here's some data" message), and act
 *  accordingly.  For simplicity, we'll assume that there is
 *  never any data waiting.  See the text for more discussion
 *  about this.
*/

void
gotAMessage (int rcvid, ClientMessageT *msg)
{

	ClientMessageT msg2;
    // determine the kind of message that it is
    switch (msg -> messageType) {

    // client wants to wait for data
    case    MT_WAIT_DATA:

		if (!curr)
		{
			curr = (clients *)malloc(sizeof(clients));
			curr->rcvid = rcvid;
			curr->timeout = 5;
			curr->next  = NULL;
			head = curr;
			back = curr;
		}
		else
		{
			curr->next = (clients *)malloc(sizeof(clients));
			curr = curr->next;
			curr->rcvid = rcvid;
			curr->timeout = 5;
			curr->next  = NULL;
			back = curr;
		}
		return;
		break;

    // client with data
    case    MT_SEND_DATA:

		curr = head;
		if(curr)
		{
			msg -> messageType = MT_OK;
			printf("Data for ready");
                // reply to BOTH CLIENTS!

                msg2.data.x=msg->data.x;
                msg2.data.y=msg->data.y;
                				msg2.messageType=MT_OK;
                MsgReply (rcvid, EOK, &msg2, sizeof (msg2));
                MsgReply (curr->rcvid, EOK, &msg2,
                          sizeof (msg2));
				head = curr->next;
				free(curr);
				curr = NULL;
				return;
		}
        msg -> messageType = MT_ERROR;
        msg2.messageType=MT_ERROR;
		MsgReply (rcvid, EOK, &msg2, sizeof (msg2));
        fprintf (stderr, "Table empty, message from rcvid %d ignored, "
                         "client ansvered error\n", rcvid);
        break;
    }
}

