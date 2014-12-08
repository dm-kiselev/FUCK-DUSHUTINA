#include "includes.h"
#include "net.h"

int frame_init(frame_t *pframe) {
	memset(pframe, NULL, sizeof(frame_t));
	return 0;
}

int frame_reply(int rcvid, frame_t* pframe) {
	int result;
	if(pframe!=NULL) {
		iov_t *piov;
		piov = malloc(sizeof(iov_t)*(pframe->size*2+1));
		SETIOV(piov, pframe, sizeof(frame_t));
		for(size_t i = 0; i < pframe->size; ++i) {
			SETIOV(piov + 2*i+1, pframe->ptask+i, sizeof(task_t));
			SETIOV(piov + 2*i+2, pframe->ptask[i].px, pframe->ptask[i].n);
		}
		result = MsgReplyv(rcvid, EOK, piov, pframe->size*2+1);
		__ERROR_CHECK(result, -1, MSG_ERR_MSGREPLY);
		free(piov);
	} else {
		MsgReply(rcvid, EOK, NULL, NULL);
	}
	return result;
}

int frame_send(int coid, frame_t *psframe, frame_t *prframe) {
	int result;
	iov_t *psiov, *priov;
	psiov = malloc(sizeof(iov_t)*(psframe->size*2+1));
	SETIOV(psiov, psframe, sizeof(frame_t));

	if(prframe!=NULL) {
		priov = malloc(sizeof(iov_t)*(psframe->size*2+1));
		SETIOV(priov, prframe, sizeof(frame_t));
		for(size_t i = 0; i < psframe->size; ++i) {
			SETIOV(psiov + 2*i+1, psframe->ptask+i, sizeof(task_t));
			SETIOV(psiov + 2*i+2, psframe->ptask[i].px, psframe->ptask[i].n);
			SETIOV(priov + 2*i+1, prframe->ptask+i, sizeof(task_t));
			SETIOV(priov + 2*i+2, prframe->ptask[i].px, prframe->ptask[i].n);
		}
		result = MsgSendv(coid, psiov, psframe->size*2+1, priov, prframe->size*2+1);
		frame_repair(prframe, priov);
		free(priov);
	} else {
		for(size_t i = 0; i < psframe->size; ++i) {
			SETIOV(psiov + 2*i+1, psframe->ptask+i, sizeof(task_t));
			SETIOV(psiov + 2*i+2, psframe->ptask[i].px, psframe->ptask[i].n);
		}
		result = MsgSendv(coid, psiov, psframe->size*2+1, NULL, NULL);
	}
	free(psiov);
	__ERROR_CHECK(result, -1, MSG_ERR_MSGSEND);
	return result;
}

void frame_repair(frame_t *pframe, iov_t *piov) {
	pframe->ptask = GETIOVBASE(piov+1);
	for(size_t i=0; i<pframe->size; ++i) {
		pframe->ptask[i].px = GETIOVBASE(piov+2*i+2);
	}
}

int frame_datareceive(int rcvid, frame_t *pframe) {
	int result=0;
	size_t size, offset = sizeof(frame_t);
	iov_t *piov;
	piov = malloc(sizeof(iov_t)*(pframe->size*2));
	pframe->ptask = malloc(sizeof(task_t)*(pframe->size));
	for(size_t i=0; i<pframe->size; ++i) {
		SETIOV(piov+2*i, pframe->ptask+i, sizeof(task_t));
		MsgReadv(rcvid, piov+2*i, 1, offset);
		size = pframe->ptask[i].n;
		pframe->ptask[i].px = malloc(size);
		SETIOV(piov+2*i+1, pframe->ptask[i].px, size);
		offset += sizeof(task_t);
		MsgReadv(rcvid, piov+2*i+1, 1, offset);
		offset += size;
	}
	free(piov);
	return result;
}

void frame_destroy(frame_t* pframe) {
	for(size_t i=0; i<pframe->size; ++i) {
		free(pframe->ptask[i].px);
	}
	free(pframe->ptask);
	pframe->ptask = NULL;
}

void frame_repinit(frame_t *psframe, frame_t *prframe) {
	*prframe = *psframe;
	prframe->ptask = malloc(sizeof(task_t)*(psframe->size));
	for(size_t i=0; i<psframe->size; ++i) {
		switch(psframe->ptask[i].cmd){
		case SPLINE_GETVAL:
			prframe->ptask[i].n = psframe->ptask[i].n;
			prframe->ptask[i].px = malloc(psframe->ptask[i].n);
			break;
		default:
			prframe->ptask[i].n = 0;
			prframe->ptask[i].px = 0;
		}
	}
}

int retranslateframe() {
	return 0;
}
