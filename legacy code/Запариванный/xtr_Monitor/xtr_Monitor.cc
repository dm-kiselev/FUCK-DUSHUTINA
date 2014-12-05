#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/iomsg.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/netmgr.h>
#include <string.h>
#include "xtr_struct.h"

void timerInit();

sigset_t 				set;

int main( int argc, char *argv[])
{
	int chid, coid, srv_coid;
	struct _struct_monitor_request msg;
	struct _struct_proc_ready msg_give;

	
	/* we need a channel to receive the pulse notification on */
	chid = ChannelCreate( 0 ); 

	/* and we need a connection to that channel for the pulse to be
	delivered on */
	coid = ConnectAttach( 0, 0, chid, _NTO_SIDE_CHANNEL, 0 );

	/* fill in the event structure for a pulse */
	SIGEV_PULSE_INIT( &msg.event, coid, SIGEV_PULSE_PRIO_INHERIT, MY_PULSE_CODE, 0 );
	SIGEV_PULSE_INIT( &msg_give.event, coid, SIGEV_PULSE_PRIO_INHERIT, MY_PULSE_CODE, 0 );

	/* find the server */
	
	char * const fileFIFO = (char *)(strdup("QNX_2"));
	int nd = netmgr_strtond( fileFIFO, NULL);
	
	

	timerInit();
	
	msg_give.type = MONITOR_REQUEST_DATA;
	ChannelDestroy(coid);
	while(1)
	{
		srv_coid = ConnectAttach( nd, atoi(argv[1]), 2, 0, 0 );
		MsgSend( srv_coid, &msg_give, sizeof(msg_give), &msg, sizeof(msg));
	
		
		printf("INFO: STATUS PROC: ");
		for(int j = 0; j < AMOUNT_PROC; j++)
		{
			printf("%d ", msg.data.procState[j][1]);
		}
		printf("\n");
		
		printf("INFO: ID: ");
		for(int j = 0; j < AMOUNT_PROC; j++)
		{
			printf("%d ", msg.data.procState[j][0]);
		}
		printf("\n");
		printf("INFO: CLIENT ID: ");
		for(int j = 0; j < AMOUNT_PROC; j++)
		{
			printf("%d ", msg.data.procState[j][2]);
		}
		printf("\n\n");

		ConnectDetach(srv_coid);
		sigwaitinfo(&set, NULL);
		
	}
	
	return 0;
}

void timerInit()
{
	struct sigevent         event;
	struct itimerspec       itime;
	timer_t                 timer_id;

	struct sigaction 		act;
    
    
    sigemptyset( &set );
    sigaddset( &set, SIGUSR1 );
    act.sa_flags = 0;
    act.sa_mask = set;
  //  act.sa_sigaction = &handler;
    sigaction( SIGUSR1, &act, NULL );

	SIGEV_SIGNAL_INIT( &event, SIGUSR1 );
	timer_create(CLOCK_REALTIME, &event, &timer_id);

	itime.it_value.tv_sec = 3;
	/* 500 million nsecs = .5 secs */
	itime.it_value.tv_nsec = 0; 
	itime.it_interval.tv_sec = 2;
	/* 500 million nsecs = .5 secs */
	//itime.it_interval.tv_nsec = 500000000; 
	timer_settime(timer_id, 0, &itime, NULL);
}


