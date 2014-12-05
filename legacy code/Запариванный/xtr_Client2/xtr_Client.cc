#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/iomsg.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>

typedef struct _pulse msg_header_t;

struct my_msg
{
	msg_header_t hdr;
	short type;
	struct sigevent event;
	int data;
};

struct _result_proc
{
   int data;
};

#define MY_PULSE_CODE _PULSE_CODE_MINAVAIL+5
#define MSG_GIVE_PULSE _IO_MAX+4
#define MSG_GIVE_RESULT _IO_MAX+5
#define MY_SERV "my_server_name"

int main( int argc, char **argv)
{
	int chid, coid, srv_coid, rcvid;
	struct my_msg msg ;
	struct _pulse pulse;
	struct _result_proc msg2;

	/* we need a channel to receive the pulse notification on */
	chid = ChannelCreate( 0 ); 

	/* and we need a connection to that channel for the pulse to be
	delivered on */
	coid = ConnectAttach( 0, 0, chid, _NTO_SIDE_CHANNEL, 0 );

	/* fill in the event structure for a pulse */
	SIGEV_PULSE_INIT( &msg.event, coid, SIGEV_PULSE_PRIO_INHERIT, MY_PULSE_CODE, 0 );
	msg.type = MSG_GIVE_PULSE;

	/* find the server */
	if ( (srv_coid = name_open( MY_SERV, 0 )) == -1)
	{
		printf("failed to find server, errno %d\n", errno );
		exit(1);
	}

	/* give the pulse event we initialized above to the server for
	later delivery */
	MsgSend( srv_coid, &msg, sizeof(msg), NULL, 0 );

	/* wait for the pulse from the server */
	rcvid = MsgReceivePulse( chid, &pulse, sizeof( pulse ), NULL );
	printf("got pulse with code %d, waiting for %d\n", pulse.code, MY_PULSE_CODE );
	
	msg.type = MSG_GIVE_RESULT;
	
	MsgSend( srv_coid, &msg, sizeof(msg), &msg2, sizeof(msg2) );
	printf("data: %d\n", msg2.data);
	//sleep(6);
	return 0;
}
