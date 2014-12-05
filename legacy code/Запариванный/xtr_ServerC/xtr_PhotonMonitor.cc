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

#include <unistd.h>
#include <Pt.h>

void timerInit();

int timer_cb( PtWidget_t *widget, void *data, PtCallbackInfo_t *cbinfo );
void *thPhoton(void*  arg);

typedef struct info_server
{
	int amountFreeProc;
	int amountClient;
};


static  PtWidget_t  *feed_toggle_wgt, *mtrend_wgt;
sigset_t set;
struct info_server infoServer;


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
	
	pthread_t thPhoton_tid;
	pthread_create(&thPhoton_tid,	NULL, &thPhoton, NULL);

	timerInit();
	//initPhoton();
	
	
	msg_give.type = MONITOR_REQUEST_DATA;
	ChannelDestroy(coid);
	while(1)
	{
		srv_coid = ConnectAttach( nd, atoi(argv[2]), atoi(argv[1]), 0, 0 );
		MsgSend( srv_coid, &msg_give, sizeof(msg_give), &msg, sizeof(msg));
	
		infoServer.amountFreeProc = 0;
		printf("INFO: STATUS PROC: ");
		for(int j = 0; j < AMOUNT_PROC; j++)
		{
			if(msg.data.procState[j][1] == 0)
				infoServer.amountFreeProc++;
			//printf("%d ", infoServer.amountFreeProc);
			printf("%d ", msg.data.procState[j][1]);
		}
		printf("\n");
		
		printf("INFO: ID: ");
		for(int j = 0; j < AMOUNT_PROC; j++)
		{
			printf("%d ", msg.data.procState[j][0]);
		}
		printf("\n");
		infoServer.amountClient = 0;
		printf("INFO: CLIENT ID: ");
		for(int j = 0; j < AMOUNT_PROC; j++)
		{
			if(msg.data.procState[j][2] != -1)
				infoServer.amountClient++;
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
	itime.it_interval.tv_sec = 0;
	/* 500 million nsecs = .5 secs */
	itime.it_interval.tv_nsec = 500000000; 
	timer_settime(timer_id, 0, &itime, NULL);
}

int timer_cb( PtWidget_t *widget, void *data, PtCallbackInfo_t *cbinfo )
{
        int val1 = infoServer.amountClient,
            val2 = infoServer.amountFreeProc;
        PtMTrendAddData( mtrend_wgt, 0, &val1, 1 );
        PtMTrendAddData( mtrend_wgt, 1, &val2, 1 );
    return Pt_CONTINUE;
}

void *thPhoton(void*  arg)
{
	int argc;
	char *argv[20];
	PtWidget_t     *window, *wgt;
    PtArg_t         args[20];
    int             i;

    // application window
    i = 0;
    PtSetArg( &args[i++], Pt_ARG_WINDOW_TITLE, "Server status", 0 );
    PtSetArg( &args[i++], Pt_ARG_HEIGHT, 400, 0 );
    PtSetArg( &args[i++], Pt_ARG_WIDTH,  800, 0 );

    if( NULL == ( window = PtAppInit( NULL, &argc, argv, i, args ) ) ) {
        perror( "PtAppInit()" );
        //return 1;
        exit(EXIT_FAILURE);
    }

    // feed toggle
    i = 0;
    // mtrend
    {
        PhArea_t        area = { {20, 30}, {760,330} };
        PtMTrendAttr_t  graph1_attr[] = { Pt_MTREND_STATE_SHOWN, Pg_RED, 1, Pg_MITER_JOIN, 0, 20 };
        PtMTrendAttr_t  graph2_attr[] = { Pt_MTREND_STATE_SHOWN, Pg_BLUE, 1, Pg_MITER_JOIN, 0, 20 };

        i = 0;
        PtSetArg( &args[i++], Pt_ARG_MTREND_N_GRAPHS, 2, 0 );
        PtSetArg( &args[i++], Pt_ARG_MTREND_N_SAMPLES, 100,  0 );
        PtSetArg( &args[i++], Pt_ARG_MTREND_FLAGS, Pt_TRUE, Pt_MTREND_BLIT );
        PtSetArg( &args[i++], Pt_ARG_MTREND_GRAPH_ATTR, &graph1_attr, 0 );
        PtSetArg( &args[i++], Pt_ARG_MTREND_GRAPH_ATTR, &graph2_attr, 1 );
        PtSetArg( &args[i++], Pt_ARG_AREA, &area, 0 );

        mtrend_wgt = PtCreateWidget( PtMTrend, NULL, i, args );
    }

    // timer
    {
        PtCallback_t callback = { timer_cb, NULL };

        i = 0;
        PtSetArg( &args[i++], Pt_ARG_TIMER_INITIAL, 100, 0 );
        PtSetArg( &args[i++], Pt_ARG_TIMER_REPEAT, 100, 0 );
        PtSetArg( &args[i++], Pt_CB_TIMER_ACTIVATE, &callback, 0 );

        PtCreateWidget( PtTimer, NULL, i, args );
    }

    PtRealizeWidget( window );
	PtMainLoop();
}

