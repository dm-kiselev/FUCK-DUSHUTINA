#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/iomsg.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>

#include "xtr_struct.h"
#include "IMSL.h"

void xtrFunction(struct data_calc_struct * data, struct data_processed *cdata);

int main( int argc, char **argv)
{
	int chid, coid, srv_coid, rcvid;
	struct _struct_proc_ready msg_ready;
	struct _struct_proc_cdata msg_cdata;
	struct data_calc_struct msg_data;
	struct _pulse pulse;
	
	
	/* we need a channel to receive the pulse notification on */
	chid = ChannelCreate( 0 ); 

	/* and we need a connection to that channel for the pulse to be
	delivered on */
	coid = ConnectAttach( 0, 0, chid, _NTO_SIDE_CHANNEL, 0 );

	/* fill in the event structure for a pulse */
	SIGEV_PULSE_INIT( &msg_ready.event, coid, SIGEV_PULSE_PRIO_INHERIT, MY_PULSE_CODE, 0 );
	

	/* find the server */
	if ( (srv_coid = name_open( MY_SERV, 0  )) == -1)
	{
		printf("failed to find server, errno %d\n", errno );
		exit(1);
	}

	/* give the pulse event we initialized above to the server for
	later delivery */
	//printf("PROC: size %d\n", sizeof(msg_ready));

	//msg_ready.type = PROC_READY;
	//MsgSend( srv_coid, &msg_ready, sizeof(msg_ready), NULL, 0 );

	/* wait for the pulse from the server */
	//rcvid = MsgReceivePulse( chid, &pulse, sizeof( pulse ), NULL );
	//printf("got pulse with code %d, waiting for %d\n", pulse.code, MY_PULSE_CODE );
	//printf("PROC: SERVER CONNECTED\n");
	SIGEV_PULSE_INIT( &msg_ready.event, coid, SIGEV_PULSE_PRIO_INHERIT, MY_PULSE_CODE, 0 );
	while(1)
	{
		msg_ready.type = PROC_READY;
		MsgSend( srv_coid, &msg_ready, sizeof(msg_ready), NULL, 0 );
		printf("PROC: READY \n");
		/* wait for the pulse from the server */
		rcvid = MsgReceivePulse( chid, &pulse, sizeof( pulse ), NULL );
		printf("PROC: PROC_REQUEST_DATA  \n");
		msg_ready.type = PROC_REQUEST_DATA;
		MsgSend( srv_coid, &msg_ready, sizeof(msg_ready), &msg_data, sizeof(msg_data));
		printf("PROC: DATA TRANSMISSION\n");
		//printf("%f\n", msg_data.f);
		
		xtrFunction(&msg_data, &msg_cdata.data);
		printf("%f\n", msg_cdata.data.f);
		/* wait for the pulse from the server */
		//rcvid = MsgReceivePulse( chid, &pulse, sizeof( pulse ), NULL );
		//printf("PROC: got pulse with code %d, waiting for %d\n", pulse.code, MY_PULSE_CODE );
		
		msg_cdata.type = PROC_TRANSMISSION_DATA;
		//msg_cdata.data.f = 1.653;
		//SIGEV_PULSE_INIT( &msg_cdata.event, coid, SIGEV_PULSE_PRIO_INHERIT, MY_PULSE_CODE, 0 );
		MsgSend( srv_coid, &msg_cdata, sizeof(msg_cdata), NULL, 0 );
		printf("PROC: END\n");
		/* wait for the pulse from the server */
		//rcvid = MsgReceivePulse( chid, &pulse, sizeof( pulse ), NULL );
		//printf("PROC: got pulse with code %d, waiting for %d\n", pulse.code, MY_PULSE_CODE );
		
	}

	return 0;
}

void xtrFunction(struct data_calc_struct * data, struct data_processed *cdata)
{
	extrapolation line;
	cdata->f = line.calcExtrapolationXY(data->x, data->y, AMOUNT_POINT, data->x_min);
}
