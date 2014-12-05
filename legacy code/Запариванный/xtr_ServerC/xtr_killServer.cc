#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/iomsg.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>

#include "xtr_struct.h"

int main( int argc, char **argv)
{
	int chid, coid, srv_coid, rcvid;
	/*struct _struct_client_data msg;
	struct _struct_proc_ready msg_give;
	struct _struct_client_cdata cdata;
	struct _pulse pulse;

	//msg.timeOut = 10;
	//msg.timeOut = 10;
	
	/* we need a channel to receive the pulse notification on */
	chid = ChannelCreate( 0 ); 

	/* and we need a connection to that channel for the pulse to be
	delivered on */
	coid = ConnectAttach( 0, 0, chid, _NTO_SIDE_CHANNEL, 0 );

	/* fill in the event structure for a pulse */
	//SIGEV_PULSE_INIT( &msg.event, coid, SIGEV_PULSE_PRIO_INHERIT, MY_PULSE_CODE, 0 );
	//SIGEV_PULSE_INIT( &msg_give.event, coid, SIGEV_PULSE_PRIO_INHERIT, MY_PULSE_CODE, 0 );

	/* find the server */
	if ( (srv_coid = name_open( MY_SERV, 0 )) == -1)
	{
		printf("failed to find server, errno %d\n", errno );
		exit(1);
	}

	MsgSendPulse(srv_coid, sched_get_priority_max(SCHED_FIFO), EXIT_SERV, 224);

	return 0;
}

