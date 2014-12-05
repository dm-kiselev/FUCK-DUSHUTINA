#include "ablibs.h"
#include "abimport.h"
#include <../../IndividualProject/includes.h>
#include <../../IndividualProject/logger.h>
#include "net.h"
#include "data.h"

#define FILE_NAME_CFG "graph_cfg"
#define SRV_NAME "srv_graph"
int processtask(task_t *ptask);

extern double_t px[POINT_MAX];
extern double_t py[POINT_MAX];
extern sem_t sem;
void *srv_graph(void *v) {
	name_attach_t* pnat;
	char srv_name[BUFFER_SIZE] = SRV_NAME;
	char cmd[BUFFER_SIZE], param[BUFFER_SIZE];
	int rcvid;
	frame_t frame;
	size_t i;
	iov_t *piov, header;

	FILE * pfcfg = fopen(FILE_NAME_CFG, "r");
	if(NULL != pfcfg) {
		while(0 == feof(pfcfg)) {
			fscanf(pfcfg, "%s %s", cmd, param);
			if(0 == strcmp(cmd, "srv_name")) {
				strcpy(srv_name, param);
			}
		}
		fclose(pfcfg);
	}

	//log_start("srv_graph_log", "srv_graph_err");
	pnat = name_attach(NULL, srv_name, NAME_FLAG_ATTACH_GLOBAL);
	SETIOV(&header, &frame, sizeof(frame_t));
	while(true) {
		//log_update();
		rcvid = MsgReceivev(pnat->chid, &header, 1, NULL);
		__ERROR_CONT(rcvid, -1, MSG_ERR_MSGREC);
		if(0 == rcvid) {
			switch(frame.header.code) {
			case _PULSE_CODE_DISCONNECT:
				printf("Client has gone\n");
				break;
			case _PULSE_CODE_UNBLOCK:
				printf("_PULSE_CODE_UNBLOCK\n");
				__ERROR_CHECK(MsgError(frame.header.value.sival_int, ETIME),-1,MSG_ERR_MSGERR);
				break;
			default:
				break;
			}
		} else {
			if (frame.header.type == _IO_CONNECT) {
				printf("Send OK\n");
				frame_reply(rcvid, NULL);
				continue;
			}
			frame_datareceive(rcvid, &frame);
			frame_reply(rcvid, NULL);
			i=0;
			while(i<frame.size) {
				processtask(frame.ptask+i);
				++i;
			}
			frame_destroy(&frame);
		}
		//log_stop();
	}
}

int processtask(task_t *ptask) {
		size_t i = 0, s, t ;
		switch(ptask->cmd) {
		case IMG_DRAW:
			s = ptask->n/(2*sizeof(double_t));
			sem_wait(&sem);
			while(i < s){
				t = roundf(((double_t*)ptask->px)[i] * 10);
				px[t] = ((double_t*)ptask->px)[i];
				py[t] = ((double_t*)ptask->px)[i+s] -1;
				++i;
			}
			sem_post(&sem);
			break;
		case IMG_CLEAR:
			sem_wait(&sem);
			while(i<POINT_MAX) {
				px[i] = -1;
				py[i] = -1;
				++i;
			}
			//memset(px, -1, POINT_MAX*sizeof(double_t));
			//memset(py, -1, POINT_MAX*sizeof(double_t));
			sem_post(&sem);
			break;
		}
		PtDamageWidget( AbWidgets[ABN_PtRawDots].wgt );
		return 0;
	}
