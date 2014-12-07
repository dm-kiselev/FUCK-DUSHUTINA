
#include "common.h"

static void gotAPulse(void);
static void gotAMessage( int rcvid, ClientMessageT *msg );

void main( int argc, char *argv[] ) {
	
	unsigned int flag = NAME_FLAG_ATTACH_GLOBAL;
	
	// Register server name in the global namespace and create a channel
	name_attach_t * msg_channel = name_attach( NULL, srvName, flag );
	if( msg_channel == NULL ) {
		cerr << srvName << ": name attach error!" << endl;
		exit(EXIT_FAILURE);
	}
	int chid = msg_channel->chid;	// channel ID
	MessageT msg;					// message buffer
	
	// if the name registered globally we should receive a message
	// and reply to it after registering
	int rcvid = MsgReceive( chid, &msg, sizeof(msg), NULL);
	MsgReply( rcvid, 0, &msg, sizeof(msg));

    // create a connection back to ourselves
    int coid = name_open( srvName, 0 );
    if (coid == -1) {
    	cerr << srvName << ": cannot connect to myself!" << endl;
        exit(EXIT_FAILURE);
    }

    struct sigevent event;      // Structure that describes an event
    // The initialization macro for sigevent structure
    SIGEV_PULSE_INIT( &event, coid, SIGEV_PULSE_PRIO_INHERIT, TIMER_CODE, 0);

    timer_t timerID;

    // create the timer and binding it to the event
    if( timer_create( CLOCK_REALTIME, &event, &timerID) == -1 ) {
    	cerr << srvName << ": cannot create a timer" << endl;
        exit(EXIT_FAILURE);
    }

    struct itimerspec timer_spec;      // the timer data structure
    
    // setup the timer (1s delay, 1s reload)
    timer_spec.it_value.tv_sec = 1;
    timer_spec.it_value.tv_nsec = 0;
    timer_spec.it_interval.tv_sec = 1;
    timer_spec.it_interval.tv_nsec = 0;

    // and start it!
    timer_settime( timerID, 0, &timer_spec, NULL );

    while() {
    	rcvid = MsgReceive( chid, &msg, sizeof(msg), NULL);
    	if( rcvid == 0 ) gotAPulse();
    	else gotAMessage( rcvid, &msg.msg );
    }
    
}

// clients table
struct clients {
    int rcvid;              // receive ID of client
    int timeout;            // timeout left for client
	clients *next;			// pointer to the next client
};

clients *current;
clients *head;
clients *tail;

/*
 *  This routine is called whenever a message arrives.  We
 *  look at the type of message (either a "wait for data"
 *  message, or a "here's some data" message), and act
 *  accordingly.  For simplicity, we'll assume that there is
 *  never any data waiting.  See the text for more discussion
 *  about this.
*/

void gotAMessage( int rcvid, ClientMessageT *msg ) {

	ClientMessageT msg2;
    // determine the kind of message that it is
    switch( msg->messageType ) {

    // client wants to wait for data
    case    MT_WAIT_DATA:

		if( !current ) {		// if there are no clients
			current = (clients *)malloc(sizeof(clients));
			head = current;
		}
		else {
			current->next = (clients *)malloc(sizeof(clients));
			current = current->next;
		}
		current->rcvid = rcvid;
		current->timeout = 5;
		current->next = NULL;
		tail = current;
		break;

    // client with data
    case    MT_SEND_DATA:

		current = head;
		if( current ) {
			
                // reply to BOTH CLIENTS!

                msg2.data = msg->data;
				msg2.messageType = MT_OK;
                MsgReply( rcvid, EOK, &msg2, sizeof(msg2) );
                MsgReply( current->rcvid, EOK, &msg2, sizeof(msg2) );
				head = current->next;
				free(current);
				current = NULL;
		}
		else {
        msg2.messageType = MT_ERROR;
		MsgReply( rcvid, EOK, &msg2, sizeof(msg2));
        fprintf (stderr, "Table empty, message from rcvid %d ignored, "
                         "client answered error\n", rcvid);
		}
    }
}

/*
 *  This routine is responsible for handling the fact that a
 *  timeout has occurred.  It runs through the list of clients
 *  to see which client has timed out, and replies to it with
 *  a timed-out response.
 */
void gotAPulse() {
    ClientMessageT  msg;
    int             i=0;
	clients * buf;
	
    // prepare a response message
    msg.messageType = MT_TIMEOUT;


   current = head;
   buf = current;
   while(current) {
      if( --current->timeout == 0 ) {
          // send a reply
          MsgReply( current->rcvid, EOK, &msg, sizeof(msg) );
		  printf("Client %d timeout\n", i);

         if (!i) {
			head = current->next;
			free(current);
			current = head;
		 }
		 else {
			buf->next = current->next;
			free(current);
			current = buf->next;
		 }
      }
      else {
		buf = current;
		current = current->next ;
	  }
	  i++;
	}
}

