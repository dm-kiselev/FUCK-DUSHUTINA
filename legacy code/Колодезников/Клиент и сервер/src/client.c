#include "includes.h"
#include "net.h"

#define SIZE 40
#define ITR 10
#define STEP 0.2
#define SHIFT 0.1
#define CFGFILENAME "cl_cfg"
#define OUTFILENAME "cl_out"

int main(int argc, char **argv) {
	if(argc != 2) {
		printf("Set client id\n");
		return EXIT_SUCCESS;
	}
	bool send_graph = false;
	char srv_graph_name[BUFFER_SIZE] = "srv_graph";
	int srv_coid, rec_coid, rec_chid, srv_graph_coid;
	uint32_t cid = atoi(argv[1]);//!!
	struct timespec timeout, stime, etime;
	struct sigevent notify;
	frame_t frame, rframe, graph_frame;
	char buf[BUFFER_SIZE], cmd[BUFFER_SIZE], param[BUFFER_SIZE];
	struct _pulse pulse;
	int itr = -1, n, k;
	FILE *pfile = 0;
	uint32_t itr_max = ITR, graph_size = 1;
	double_t *pfcn, *psig, init;
	uint64_t timeoutsend = 200000000;

//*****************************************************************************
	timeout.tv_nsec = 0;
	timeout.tv_sec = 1;

	strcpy(buf, CFGFILENAME);
	strcat(buf, argv[1]);
	pfile = fopen(buf, "r");
	if(NULL != pfile) {
		while(0 == feof(pfile)) {
			fscanf(pfile, "%s %s", cmd, param);
			if(0 == strcmp(cmd, "itr_max")) {
				itr_max = atoi(param);
			}
			else if(0 == strcmp(cmd, "timeoutsend")) {
				timeoutsend = atol(param);
			}
			else if(0 == strcmp(cmd, "timeout_sec")) {
				timeout.tv_sec = atol(param);
			}
			else if(0 == strcmp(cmd, "timeout_nsec")) {
				timeout.tv_nsec = atol(param);
			}
			else if(0 == strcmp(cmd, "send_graph")) {
				send_graph = atoi(param);
			}
			else if(0 == strcmp(cmd, "srv_graph")) {
				strcpy(srv_graph_name, param);
			}
			else if(0 == strcmp(cmd, "graph_size")) {
				graph_size = atoi(param);
			}
		}
		fclose(pfile);
	} else {
		perror(MSG_ERR_CFGFILE);
	}

	strcpy(buf, OUTFILENAME);
	strcat(buf, argv[1]);
	pfile = fopen(buf, "w");
	fprintf(pfile, "itr_max %i\ntimeoutsend   %llu\ntimeout %lu %lu\n",
			itr_max, timeoutsend,
			timeout.tv_sec, timeout.tv_nsec);

	pfcn = malloc(sizeof(double_t)*(SIZE*2));
	psig = malloc(sizeof(double_t)*((SIZE-1)*2));

	srand(time(NULL));
	for(size_t i = 0; i<SIZE; i++) {
		pfcn[i] = i*STEP;
		pfcn[i+SIZE] = sin(pfcn[i])+2;
	}

	for(size_t i = 0; i<SIZE-1; i++) {
		psig[i] = SHIFT + (rand()%SIZE)*STEP;
		psig[i+SIZE-1] = sin(psig[i])+2;
	}

	frame.cid = cid;
	frame.fid = 0;
	frame.size = 2;

//*****************************************************************************

	srv_coid = name_open(SRV_NAME, NAME_FLAG_ATTACH_GLOBAL);
	__ERROR_EXIT(srv_coid, -1, SRV_NAME);
	rec_chid = ChannelCreate(0);
	__ERROR_EXIT(rec_chid, -1, "ChannelCreate");
	rec_coid = ConnectAttach( 0, 0, rec_chid, _NTO_SIDE_CHANNEL, 0 );
	__ERROR_EXIT(rec_coid, -1, "ConnectAttach");
	frame.coid = rec_coid;
	frame.ptask = malloc(sizeof(task_t)*2);
	frame.protocol = STANDART;
	frame.ptask[0].cmd = SPLINE_INIT;
	frame.ptask[0].n = SIZE*2*sizeof(double_t);
	frame.ptask[0].px = (void*)pfcn;
	frame.timeout = timeout;
	frame.ptask[1].cmd = SPLINE_GETVAL;
	frame.ptask[1].n = 2*sizeof(double_t);
	frame.ptask[1].px = (void*)psig;
	frame_repinit(&frame, &rframe);

	if(send_graph) {
		srv_graph_coid = name_open(srv_graph_name, NAME_FLAG_ATTACH_GLOBAL);
		__ERROR_CHECK(srv_graph_coid, -1, srv_graph_name);
		if(-1 == srv_graph_coid) {
			send_graph = false;
		} else {
			graph_frame.cid = cid;
			graph_frame.fid = 0;
			graph_frame.size = 1;
			graph_frame.coid = rec_coid;
			graph_frame.protocol = STANDART;
			graph_frame.ptask = malloc(sizeof(task_t)*1);
			graph_frame.ptask[0].cmd = IMG_CLEAR;
			graph_frame.ptask[0].n = 0;
			graph_frame.ptask[0].px = 0;
			graph_frame.timeout.tv_nsec = 0;
			graph_frame.timeout.tv_sec = 0;
			__ERROR_CHECK(frame_send(srv_graph_coid, &graph_frame, NULL), -1, "sendv");
			graph_frame.ptask[0].cmd = IMG_DRAW;
			graph_frame.ptask[0].n = graph_size*2*sizeof(double_t);
			graph_frame.ptask[0].px = malloc(graph_frame.ptask[0].n);
		}
	}

	__ERROR_CHECK(TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_SEND | _NTO_TIMEOUT_REPLY,
									NULL, &timeoutsend, NULL), -1, "TimerTimeout");
	__ERROR_CHECK(frame_send(srv_coid, &frame, &rframe), -1, "sendv");
	//printf("R#%i: %f\n", -1, *(double_t*)rframe.ptask[1].px);
	frame_destroy(&rframe);
	free(frame.ptask);
	//frame_destroy(&frame);

	frame.ptask = malloc(sizeof(task_t));
	frame.ptask[0].cmd = SPLINE_GETVAL;
	frame.ptask[0].n = sizeof(double_t);
	frame.size = 1;
	frame_repinit(&frame, &rframe);
	k=0;
	for(uint32_t i=0; i<itr_max; ++i) {
		frame.fid = i;
		n = rand()%(SIZE-1);
		frame.ptask[0].px = (void*)&psig[n];
		frame.protocol = NOREPLY;
		//__ERROR_CHECK(frame_send(srv_coid, &frame, &rframe), -1, "sendv");
		clock_gettime(CLOCK_MONOTONIC, &stime);
		trace_msg_start();
		__ERROR_CONT(frame_send(srv_coid, &frame, NULL), -1, "sendv");
		__ERROR_CHECK(MsgReceivePulse(rec_chid, &pulse, sizeof( pulse ), NULL), -1, "MsgReceivePulse");
		frame.protocol = REPLY;
		__ERROR_CONT(frame_send(srv_coid, &frame, &rframe), -1, "sendv");
		trace_msg_stop();
		clock_gettime(CLOCK_MONOTONIC, &etime);
		if(send_graph) {
			((double_t*)(graph_frame.ptask[0].px))[k] = psig[n];
			((double_t*)(graph_frame.ptask[0].px))[k+graph_size] = *(double_t*)rframe.ptask->px;
			k++;
			if(k == graph_size) {
				__ERROR_CHECK(frame_send(srv_graph_coid, &graph_frame, NULL), -1, "sendv");
				k = 0;
			}
		}
		//printf("R#%i: %f %f %f\n", i, psig[n], *(double_t*)rframe.ptask->px, psig[n+SIZE-1]);
		fprintf(pfile, "%f  %f  %u  %u\n", psig[n], *(double_t*)rframe.ptask->px,
				etime.tv_sec-stime.tv_sec, etime.tv_nsec-stime.tv_nsec);
	}
	frame.protocol = STANDART;
	frame.ptask[0].cmd = SPLINE_DESTROY;
	frame.ptask[0].n = 0;
	frame.ptask[0].px = 0;
	frame.fid++;
	__ERROR_CHECK(frame_send(srv_coid, &frame, &rframe), -1, "sendv");
	fclose(pfile);
	frame_destroy(&rframe);
	free(pfcn);
	free(psig);

	__ERROR_CHECK(name_close(srv_coid), -1, "name_close");

	return EXIT_SUCCESS;
}


