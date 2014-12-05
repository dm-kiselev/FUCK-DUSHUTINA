#include <cstdlib>
#include <iostream>
#include <stdio.h>

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h> 
#include <process.h>
#include <unistd.h>
#include <sys/trace.h>


#include <stddef.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>

#include <errno.h>

#include "xtr_struct.h"


#define PULSE_CODE 5

int main(int argc, char *argv[]) 
{
	int errvalue;
	errno = EOK;
	int coid, coid_serv, chid;
	_client_data_to_calc msg_data_to_serv;
	struct sigevent event;
	struct _pulse pulse;
	iov_t iov[3];
	io_write_t whdr;
	
	
	chid = ChannelCreate( 0 ); 

		/* and we need a connection to that channel for the pulse to be
		delivered on */
	coid = ConnectAttach( 0, 0, chid, _NTO_SIDE_CHANNEL, 0 );
	SIGEV_PULSE_INIT( &event, coid, SIGEV_PULSE_PRIO_INHERIT, PULSE_CODE, 0 );


	if ( (coid_serv = name_open( MY_SERV, 0 )) == -1)
	{
		printf("failed to find server, errno %d\n", errno );
		exit(1);
	}
	
	msg_data_to_serv.step = 1.23;
	SETIOV(iov + 0, &whdr,  sizeof(whdr));
	SETIOV(iov + 1, &event, sizeof(event));
	SETIOV(iov + 2, &msg_data_to_serv,  sizeof(msg_data_to_serv));
	whdr.i.type = ENUM_CLIENT_TRANSMISSION_DATA;
	whdr.i.nbytes = sizeof(msg_data_to_serv) + sizeof(event);

	MsgSendv(coid_serv, iov, 3, 0, 0);
	 
	printf("[CLIENT] wait pulse\n");
	int rcvid = MsgReceivePulse( chid, &pulse, sizeof( pulse ), NULL );
	if(pulse.code == 5)
	{
		printf("[CLIENT] TIMEOUT \n");
		//return EXIT_FAILURE;
	}
	
	
	ConnectDetach(coid); 
	return EXIT_SUCCESS;
}


