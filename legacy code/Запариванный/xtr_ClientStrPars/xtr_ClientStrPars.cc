#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/iomsg.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>

#include "xtr_struct.h"

void formatPacket(struct data_calc_struct * data);

int main( int argc, char **argv)
{
	int chid, coid, srv_coid, rcvid;
	struct _struct_client_data msg;
	struct _struct_proc_ready msg_give;
	struct _struct_client_cdata cdata;
	struct _pulse pulse;

	msg.timeOut = 18;
	msg.timeOut = 18;
	
	/* we need a channel to receive the pulse notification on */
	chid = ChannelCreate( 0 ); 

	/* and we need a connection to that channel for the pulse to be
	delivered on */
	coid = ConnectAttach( 0, 0, chid, _NTO_SIDE_CHANNEL, 0 );

	/* fill in the event structure for a pulse */
	SIGEV_PULSE_INIT( &msg.event, coid, SIGEV_PULSE_PRIO_INHERIT, MY_PULSE_CODE, 0 );
	SIGEV_PULSE_INIT( &msg_give.event, coid, SIGEV_PULSE_PRIO_INHERIT, MY_PULSE_CODE, 0 );

	/* find the server */
	if ( (srv_coid = name_open( MY_SERV, 0 )) == -1)
	{
		printf("failed to find server, errno %d\n", errno );
		exit(1);
	}

	/* give the pulse event we initialized above to the server for
	later delivery */

	msg.type = CLIENT_TRANSMISSION_DATA;
	formatPacket(&msg.data);
	//printf("CLIENT: size %d\n", sizeof(msg));
	
	
	MsgSend( srv_coid, &msg, sizeof(msg), NULL, 0 );

	/* wait for the pulse from the server */
	rcvid = MsgReceivePulse( chid, &pulse, sizeof( pulse ), NULL );
	if(pulse.code == TIMEOUT)
	{
		printf("CLIENT: TIMEOUT \n");
		return EXIT_FAILURE;
	}
	
	//printf("CLIENT: got pulse with code %d, waiting for %d\n", pulse.code, MY_PULSE_CODE );
	
	msg_give.type = CLIENT_REQUEST_DATA;
	MsgSend( srv_coid, &msg_give, sizeof(msg_give), &cdata, sizeof(cdata));
	if(pulse.code == 1)
	{
		printf("CLIENT: TIMEOUT %d %d\n", pulse.code, TIMEOUT);
		return EXIT_FAILURE;
	}
	printf("CLIENT: data %f\n", cdata.data.f);

	return 0;
}

void formatPacket(struct data_calc_struct * data)
{
	for(int i = 0; i < AMOUNT_POINT; i++)
	{
		data->x[i] = i;
		data->y[i] = i * i;
	}
	data->x_max = 5;
	data->x_min = 5;
	data->step = 1;
}
