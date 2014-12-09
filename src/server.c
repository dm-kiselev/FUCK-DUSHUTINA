#include "includes.h"
#include "net.h"

volatile bool working = true;	// server works while it's true
void* worker(void* v);			// agent

void fsigint(int i);			// handler for SIGINT that stops the server
int processtask(task_t *ptask, task_t *ptaskt, spline_t *pspline);

int main(int argc, char **argv) {

//*****************************************************************************
	FILE *pfcfg;		// server config file descriptor
	size_t	wrk_amount = CLIENT_MAX,	// amount of agents
			client_max = CLIENT_MAX;	// max available amount of clients
//*****************************************************************************

	printf("%s\n", MSG_VER_START);

	// reading server configuration from file
	if( argc > 1 ) {
		pfcfg = fopen(argv[1], "r");
	} else {
		pfcfg = fopen(FILE_NAME_CFG, "r");
	}
	if( pfcfg ) {
		char cmd[BUFFER_SIZE], param[BUFFER_SIZE];
		while( !feof(pfcfg) ) {
			fscanf(pfcfg, "%s %s", cmd, param);
			if(0 == strcmp(cmd, CFG_PAR_WRKAM)) {
				wrk_amount = atoi(param);
			}
			else if(0 == strcmp(cmd, CFG_PAR_CLIENTMAX)) {
				client_max = atoi(param);
			}
		}
		fclose(pfcfg);
	} else {
		perror(MSG_ERR_CFGFILE);
	}

	printf("%s\n", MSG_VER_CFG);

	// set handler for SIGINT1
	signal(SIGINT, fsigint);

	// Register a server name and create a channel
	name_attach_t* pnat = name_attach(NULL, SRV_NAME, NAME_FLAG_ATTACH_GLOBAL);
	// if( pnat == NULL ) { perror("name_attach"); return EXIT_FAILURE; }
	__ERROR_EXIT(pnat, NULL, "name_attach");

	// allocate memory for clients cash
	cash_t *pcash = malloc(sizeof(cash_t)*client_max);

	syncsig_t *psync = malloc(sizeof(syncsig_t)*wrk_amount);

	pthread_t *pworker = malloc(sizeof(pthread_t)*wrk_amount);

	wrk_info_t wrk_info;
	wrk_info.chid = pnat->chid;
	wrk_info.pcash = pcash;
	wrk_info.client_amount = client_max;
	wrk_info.wrk_amount = wrk_amount;
	wrk_info.psync = psync;

	for(size_t i = 0; i<wrk_amount; ++i) {
		wrk_info.id = i;
		pthread_create(&pworker[i], NULL, worker, &wrk_info);
	}

	printf("%s\n", MSG_VER_WORK);
//*****************************************************************************
	while(working) {
		sleep(1);
	}
//*****************************************************************************
	for(size_t i = 0; i<wrk_amount; ++i) {
		pthread_kill(pworker[i], SIGKILL);
	}
	for(size_t i = 0; i<wrk_amount; ++i) {
		pthread_join(pworker[i], NULL);
	}
	name_detach(pnat, NULL);
	free(pworker);
	free(psync);
	free(pcash);
	return EXIT_SUCCESS;
}

int processtask(task_t *ptask, task_t *prep, spline_t *pspline) {
	prep->cmd = ptask->cmd;
	switch(ptask->cmd) {
	case SPLINE_INIT:
		spline_init(pspline, (double_t*)ptask->px,
					(double_t*)ptask->px + ptask->n/(sizeof(double_t)*2),
						ptask->n/(sizeof(double_t)*2));
		break;
	case SPLINE_DESTROY:
		spline_destroy(pspline);
		break;
	case SPLINE_GETVAL:
		spline_getvaluev(pspline, (double_t*)ptask->px,
						(double_t*)prep->px, ptask->n/sizeof(double_t));
		break;
	}
	return 0;
}

void fsigint (int i) {
	working = false;
}

void *worker(void *v) {

	// duplicate wrk_info from the main thread
	// это не самая лучшая реализация, потому что, если главный поток перейдёт на следующую итерацию цикла
	// и успеет инкрементировать wrk_info.id прежде, чем рабочий поток сохранит у себя копию этой структуры,
	// то у этого потока будет совсем не тот id, который должен быть.
	// возможно появление нескольких потоков с одинаковым id
	wrk_info_t wrk_info = *(wrk_info_t*)v;
	syncsig_t *psync = &wrk_info.psync[wrk_info.id];

	iov_t pheader;		// this is for MsgReceivev function
	frame_t frame;		// this is for our message

	SETIOV(&pheader, &frame, sizeof(frame_t));		// this is a kind of magic

	while(true) {
		psync->rcvid = -1;
		wrk_info.psync[wrk_info.id].status = READY;
		psync->rcvid = MsgReceivev(wrk_info.chid, &pheader, 1, NULL);
		__ERROR_CONT(psync->rcvid, -1, MSG_ERR_MSGREC);
		wrk_info.psync[wrk_info.id].status = WORK;
		if( psync->rcvid == 0 ) {
			switch(frame.header.code) {
			// server receives this pulse from OS when client call name_close function
			case _PULSE_CODE_DISCONNECT:
				printf("Client has gone\n");
				break;
			// server receives this pulse from OS when client is too long in SEND- or REPLY-blocked state
			// see also TimerTimeout call in client.c
			case _PULSE_CODE_UNBLOCK:
				printf("_PULSE_CODE_UNBLOCK\n");
				for(size_t i=0; i<wrk_info.wrk_amount; ++i) {
					if(wrk_info.psync[i].rcvid == frame.header.value.sival_int) {
						if(wrk_info.psync[i].status != SEND) {
							// if client wants to unblock and results are not ready yet, reply client with error
							__ERROR_CHECK(MsgError(frame.header.value.sival_int, ETIME),-1,MSG_ERR_MSGERR);
							wrk_info.psync[i].status = LATE;
						}
						break;
					}
				}
			}
		} else {
			// server receives this message from OS when client call name_open function
			if (frame.header.type == _IO_CONNECT) {
				printf("Send OK\n");
				frame_reply(psync->rcvid, NULL);
				continue;
			}
			frame_datareceive(psync->rcvid, &frame);	// parse received data
			frame_repinit(&frame, &wrk_info.pcash[frame.cid].framerep);	// init reply message
			for(size_t i=0; i<frame.size; ++i) {		// doing all tasks from client
				processtask(frame.ptask+i, wrk_info.pcash[frame.cid].framerep.ptask+i,
						&wrk_info.pcash[frame.cid].spline);
			}
			// reply client with results if not too late
			if( wrk_info.psync[wrk_info.id].status != LATE ) {
				wrk_info.psync[wrk_info.id].status = SEND;
				frame_reply(psync->rcvid, &wrk_info.pcash[frame.cid].framerep);
			}
			frame_destroy(&wrk_info.pcash[frame.cid].framerep);
		}
	}
}
