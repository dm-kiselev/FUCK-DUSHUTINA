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
	uint32_t cid = atoi(argv[1]);//!!
	struct timespec timeout, stime, etime;
	frame_t frame, rframe;
	char buf[BUFFER_SIZE], cmd[BUFFER_SIZE], param[BUFFER_SIZE];
	int itr = -1, n, k;
	FILE *pfile = 0;
	uint32_t itr_max = ITR;
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

	int srv_coid = name_open(SRV_NAME, NAME_FLAG_ATTACH_GLOBAL);
	__ERROR_EXIT(srv_coid, -1, SRV_NAME);
	frame.ptask = malloc(sizeof(task_t)*2);
	frame.ptask[0].cmd = SPLINE_INIT;
	frame.ptask[0].n = SIZE*2*sizeof(double_t);
	frame.ptask[0].px = (void*)pfcn;
	frame.timeout = timeout;
	frame.ptask[1].cmd = SPLINE_GETVAL;
	frame.ptask[1].n = 2*sizeof(double_t);
	frame.ptask[1].px = (void*)psig;
	frame_repinit(&frame, &rframe);

	__ERROR_CHECK(TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_SEND | _NTO_TIMEOUT_REPLY,
									NULL, &timeoutsend, NULL), -1, "TimerTimeout");
	__ERROR_CHECK(frame_send(srv_coid, &frame, &rframe), -1, "sendv");
	frame_destroy(&rframe);
	free(frame.ptask);

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
		trace_msg_start();
		__ERROR_CONT(frame_send(srv_coid, &frame, NULL), -1, "sendv");
		__ERROR_CONT(frame_send(srv_coid, &frame, &rframe), -1, "sendv");
		trace_msg_stop();
		clock_gettime(CLOCK_MONOTONIC, &etime);
		 fprintf(pfile, "%f  %f  %u  %u\n", psig[n], *(double_t*)rframe.ptask->px,
				etime.tv_sec-stime.tv_sec, etime.tv_nsec-stime.tv_nsec);
		 printf("I receive data\n");
	}
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


