#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/iomsg.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>

#include <unistd.h>
#include <Pt.h>

#include "xtr_struct.h"

void formatPacket(struct data_calc_struct * data);
void *thPhoton(void*  arg);



struct _struct_client_cdata cdata;
struct _struct_client_data msg;
struct _struct_proc_ready msg_give;

int main( int argc, char **argv)
{
	int chid, coid, srv_coid, rcvid;
	
	
	struct _pulse pulse;

	msg.timeOut = 10;
	msg.timeOut = 10;
	
	
	
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
	for(int i = 0; i < ((msg.data.x_max - msg.data.x_min )/ msg.data.step + 1); i++)
		printf("CLIENT: data %f\n", cdata.data.f[i]);
	
	pthread_t thPhoton_tid;
	pthread_create(&thPhoton_tid, NULL, &thPhoton, NULL);
	pthread_join(thPhoton_tid, NULL);
	return 0;
}


void formatPacket(struct data_calc_struct * data)
{
	for(int i = 0; i < AMOUNT_POINT; i++)
	{
		data->x[i] = i;
		data->y[i] = i * i;
	}
	data->x_min = 5;
	data->x_max = 20;
	data->step = 0.5;
}


void *thPhoton(void*  arg)
{
	int argc;
	char *argv[20];
	PtWidget_t     *window, *wgt;
	PtWidget_t  *feed_toggle_wgt, *mtrend_wgt;
    PtArg_t         args[20];
    int             i;
    
    int WindowWidth = 800;
    int WindowHeigh = 600;
   
    // application window
    i = 0;
    PtSetArg( &args[i++], Pt_ARG_WINDOW_TITLE, "Result", 0 );
    PtSetArg( &args[i++], Pt_ARG_HEIGHT, WindowHeigh, 0 );
    PtSetArg( &args[i++], Pt_ARG_WIDTH,  WindowWidth, 0 );

    if( NULL == ( window = PtAppInit( NULL, &argc, argv, i, args ) ) ) {
        perror( "PtAppInit()" );
        //return 1;
        exit(EXIT_FAILURE);
    }

    // feed toggle
    i = 0;
    // mtrend
    
    {
    	//PhPoint_t points[]={ { 0, 0}, {40,40}, {40, 5}, { 0, 0}};
    	//PhPoint_t points[]={ { 0, 0}, {40,40}};
    	PhPoint_t points[100];
    	int j = 0;
    	for(; j < ((msg.data.x_max - msg.data.x_min )/ msg.data.step + 1); j++)
    	{
    		points[j].x = (int)((msg.data.x_min + j * msg.data.step) * 10);
    		//printf("x: %d %f\n", points[j].x, msg.data.x_min + j * msg.data.step);
    		points[j].y = WindowHeigh - (int) ((cdata.data.f[j]) * 1);
    		//printf("y: %d %f\n", points[j].y, cdata.data.f[j]);
    		//printf("x y: %d %d\n", points[j].x, points[j].y);
    	}
    	
    	PtSetArg(&args[i++], Pt_ARG_POINTS, points, j ) ;
    	PtCreateWidget( PtPolygon, NULL, i, args ) ;
    }


    PtRealizeWidget( window );
	PtMainLoop();
}
