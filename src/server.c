#include "includes.h"
#include "net.h"

volatile bool working = true;	// server works while it's true
void* worker(void* v);			// agent
// void* router(void* v);
void wrk_sigusr1(int signo, siginfo_t *info, void *other);	// handler for SIGUSR1
// int chid;						// channel ID

void fsigint(int i);			// handler for SIGINT that stops the server
int processtask(task_t *ptask, task_t *ptaskt, spline_t *pspline);
//int connectslaves(slave_t *pslave);
int main(int argc, char **argv) {
//*****************************************************************************
	FILE *pfcfg;
	// sem_t wrk_sem;
	// sem_t *pwrk_sem = &wrk_sem;
	// struct timespec wait_wrk_sem;
	// size_t i, wrk_amount = CLIENT_MAX, client_max = CLIENT_MAX;
	// frame_t frame;
	// iov_t *piov, *pheader;
	// pheader = malloc(sizeof(iov_t));
	// SETIOV(pheader, &frame, sizeof(frame_t));
	// char buf[BUFFER_SIZE], cmd[BUFFER_SIZE], param[BUFFER_SIZE];
	// int rcvid, slave_chid;
	// char srv_name[BUFFER_SIZE] = SRV_NAME;
	size_t wrk_amount = CLIENT_MAX, client_max = CLIENT_MAX;

	// Slave support
	// slave_t slave;
	// slave.amount = 0;
	// uint32_t *proute;
//*****************************************************************************
	// Configs
	printf("%s\n", MSG_VER_START);
	// wait_wrk_sem.tv_nsec = WRK_SEM_TIMEOUT;
	// wait_wrk_sem.tv_sec = 0;
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
			// else if(0 == strcmp(cmd, CFG_PAR_WRKSEMTIME)) {
			// 	wait_wrk_sem.tv_nsec = atol(param);
			// }
			// else if(0 == strcmp(cmd, CFG_PAR_MASTER)) {
			// 	slave.amount = atoi(param);
			// }
			// else if(0 == strcmp(cmd, CFG_PAR_SLAVE)) {
			// 	strcpy(srv_name, SLAVE_NAME);
			// 	strcat(srv_name, param);
			// }
			else if(0 == strcmp(cmd, CFG_PAR_CLIENTMAX)) {
				client_max = atoi(param);
			}
		}
		fclose(pfcfg);
	} else {
		perror(MSG_ERR_CFGFILE);
	}
	printf("%s\n", MSG_VER_CFG);
	signal(SIGINT, fsigint);

	name_attach_t* pnat;

	// Net
	pnat = name_attach(NULL, SRV_NAME, NAME_FLAG_ATTACH_GLOBAL);
	__ERROR_EXIT(pnat, NULL, "name_attach");

	// chid = pnat->chid;

	cash_t *pcash;

	// Cash
	pcash = malloc(sizeof(cash_t)*client_max);
	// proute = malloc(sizeof(uint32_t)*CLIENT_MAX);
	// Slaves
	// if(slave.amount > 0) {
	// 	slave.pslave = malloc(sizeof(slave_info_t)*slave.amount);
	// 	for(int i = 0; i< slave.amount; ++i) {
	// 		slave.pslave[i].name = malloc(sizeof(char)*BUFFER_SIZE);
	// 		strcpy(slave.pslave[i].name, SLAVE_NAME);
	// 		itoa(i, buf, 10);
	// 		strcat(slave.pslave[i].name, buf);
	// 		slave.pslave[i].status = DOWN;
	// 		slave.pslave[i].clientmax = CLIENT_MAX;
	// 		slave.pslave[i].clientnow = 0;
	// 		sem_init(&slave.pslave[i].sem, 0, 1);
	// 	}
	// 	connectslaves(&slave);
	// 	slave_chid = ChannelCreate(0);
	// 	__ERROR_EXIT(slave_chid, -1, "ChannelCreate");
	// }


	syncsig_t *psync;
	pthread_t *pworker;

	// Workers
	// __ERROR_EXIT(sem_init(pwrk_sem, 0, 0), -1, "pwrk_sem: sem_init");
	psync = malloc(sizeof(syncsig_t)*wrk_amount);
	pworker = malloc(sizeof(pthread_t)*wrk_amount);

	wrk_info_t wrk_info;
	// wrk_info.psem = pwrk_sem;
	wrk_info.chid = pnat->chid;
	wrk_info.pcash = pcash;
	// wrk_info.pslave = &slave;
	wrk_info.client_amount = client_max;
	wrk_info.wrk_amount = wrk_amount;
	// wrk_info.proute = proute;
	wrk_info.psync = psync;

	// if(slave.amount > 0) {
	// 	for(size_t i = 0; i<wrk_amount; ++i) {
	// 		wrk_info.id = i;
	// 		pthread_create(&pworker[i], NULL, router, &wrk_info);
	// 	}
	// } else {
		for(size_t i = 0; i<wrk_amount; ++i) {
			wrk_info.id = i;
			pthread_create(&pworker[i], NULL, worker, &wrk_info);
		}
	// }
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
	// if(slave.amount > 0) {
	// 	for(size_t i = 0; i<slave.amount; ++i) {
	// 		free(slave.pslave[i].name);
	// 	}
	// 	free(slave.pslave);
	// }
	name_detach(pnat, NULL);
	free(pworker);
	free(psync);
	// free(proute);
	return EXIT_SUCCESS;
}

// int connectslaves(slave_t *pslave) {
// 	for(int i=0; i<pslave->amount; ++i) {
// 		if((pslave->pslave[i].coid != ConnectServerInfo(0, pslave->pslave[i].coid, NULL))
// 			|| (pslave->pslave[i].status == DOWN)) {
// 			pslave->pslave[i].coid = name_open(pslave->pslave[i].name, NAME_FLAG_ATTACH_GLOBAL);
// 			pslave->pslave[i].clientnow = 0;
// 			if(pslave->pslave[i].coid == -1) {
// 				perror(pslave->pslave[i].name);
// 				pslave->pslave[i].status = DOWN;
// 			} else {
// 				pslave->pslave[i].status = READY;
// 			}
// 		}
// 	}
// 	return EXIT_SUCCESS;
// }

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

// void wrk_sigusr1 (int signo, siginfo_t *info, void *other) {
// 	syncsig_t *psync = (syncsig_t*)SIGEV_GET_VALUE(info);
// 	sem_wait(&psync->sem);
// 	__ERROR_CHECK(MsgError(psync->rcvid, ETIME),-1,MSG_ERR_MSGERR);
// 	sem_post(&psync->sem);
// }

void *worker(void *v) {
	wrk_info_t wrk_info = *(wrk_info_t*)v;
	// int result;
	// struct sigevent clientevent, timeoutevent;
	// struct sigaction timeoutact;
	iov_t pheader;
	frame_t frame;
	// timer_t timer;

	syncsig_t *psync = &wrk_info.psync[wrk_info.id];
	// struct itimerspec timeout, stoptimeout;

	// struct timespec slp;
	// slp.tv_nsec = 10000000;
	// slp.tv_sec = 0;

	// timeout.it_interval.tv_sec = 0;
	// timeout.it_interval.tv_nsec = 0;
	// timeout.it_value.tv_sec = 0;
	// timeout.it_value.tv_nsec = 0;
	// stoptimeout = timeout;
	// sem_init(&psync->sem, 0, 1);
	SETIOV(&pheader, &frame, sizeof(frame_t));
	// SIGEV_SIGNAL_VALUE_INIT(&timeoutevent, SIGUSR1, psync);
	// __ERROR_CHECK(timer_create(CLOCK_MONOTONIC, &timeoutevent, &timer),-1,"timer_create");
	// __ERROR_CHECK(signal(SIGUSR1, wrk_sigusr1),-1,"signal");

	while(true) {
		psync->rcvid = -1;
		wrk_info.psync[wrk_info.id].status = READY;
		psync->rcvid = MsgReceivev(wrk_info.chid, &pheader, 1, NULL);
		__ERROR_CONT(psync->rcvid, -1, MSG_ERR_MSGREC);
		wrk_info.psync[wrk_info.id].status = WORK;
		if( psync->rcvid == 0 ) {
			switch(frame.header.code) {
			case _PULSE_CODE_DISCONNECT:
				printf("Client has gone\n");
				break;
			case _PULSE_CODE_UNBLOCK:
				printf("_PULSE_CODE_UNBLOCK\n");
				for(size_t i=0; i<wrk_info.wrk_amount; ++i) {
					if(wrk_info.psync[i].rcvid == frame.header.value.sival_int) {
						if(wrk_info.psync[i].status != SEND) {
							__ERROR_CHECK(MsgError(frame.header.value.sival_int, ETIME),-1,MSG_ERR_MSGERR);
						}
						break;
					}
				}
			// 	break;
			// default:
			// 	break;
			}
		} else {
			if (frame.header.type == _IO_CONNECT) {
				printf("Send OK\n");
				frame_reply(psync->rcvid, NULL);
				continue;
			}
			// wrk_info.proute[frame.cid] = wrk_info.id;
			// timeout.it_value = frame.timeout;
			// sem_wait(&psync->sem);
			// if(frame.protocol != REPLY) {
			// 	__ERROR_CHECK(timer_settime(timer, 0, &timeout, NULL),-1,"timer_settime");
			// }
			frame_datareceive(psync->rcvid, &frame);
			// sem_post(&psync->sem);
			// nanosleep( &slp, NULL);

			// if(frame.protocol == NOREPLY) {
			// 	SIGEV_PULSE_INIT( &clientevent, frame.coid,
			// 						SIGEV_PULSE_PRIO_INHERIT, PULSE_CODE_DATA_READY, 0);
			// 	frame_reply(psync->rcvid, NULL);
			// }
			// if(frame.protocol != REPLY) {
				frame_repinit(&frame, &wrk_info.pcash[frame.cid].framerep);
				for(size_t i=0; i<frame.size; ++i) {
					processtask(frame.ptask+i, wrk_info.pcash[frame.cid].framerep.ptask+i,
							&wrk_info.pcash[frame.cid].spline);
				}
				// __ERROR_CHECK(timer_settime(timer, 0, &stoptimeout, NULL),-1,"timer_settime");
			// }

			wrk_info.psync[wrk_info.id].status = SEND;
			// if(frame.protocol == NOREPLY) {
			// 	result = -1;
			// 	for(int i=0; i<3 && result == -1; ++i) {
			// 		result = MsgDeliverEvent(psync->rcvid, &clientevent);
			// 		__ERROR_CHECK(result, -1, "MsgDeliverEvent");
			// 	}
			// } else {
				frame_reply(psync->rcvid, &wrk_info.pcash[frame.cid].framerep);
				frame_destroy(&wrk_info.pcash[frame.cid].framerep);
			// }
		}
	}
}
