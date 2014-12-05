#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/iomsg.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>

#include <unistd.h>
#include <sys/netmgr.h>

#include <errno.h>
#include <string.h>
#include <time.h>

#include "xtr_struct.h"


#define SIZE_MAX_RECEIVE_BUFF 100000000

struct _struct_client_id
{
	int clientId;
	int procId;
	void *_struct_client_transmission;
	void *_struct_proc_transmission;
};


#define TIME_PULSE_CODE   _PULSE_CODE_MINAVAIL + 3

int searchClientForID(int id, _struct_client_id  **inf_client);
int searchProcForID(int id, _struct_client_id ** inf_client);
int findFreeProc(int ** arrID); // return id free proc
int findProcId(int ** arrID, int id);
void timeInit();
void handler_timer(int signo, siginfo_t *info, void *other);
void gotAPulse( _struct_client_id ** inf_client);
int findPendingClient( _struct_client_id ** inf_client);

name_attach_t *attach;

int main( int argc, char **argv)
{
	int rcvid = -1;
	struct receive_msg msg;
	bool flagSendMsg = true;
	int errvalue;
	
	struct _struct_client_data msg_data;
	
	//int procID[AMOUNT_PROC][2];
	
	int** procID = new int*[AMOUNT_PROC];
	for(int i = 0; i < AMOUNT_PROC; i++) 
	{
		procID[i] = new int[2];
	}
	int proc_scoid[AMOUNT_PROC][2];

	//struct _struct_client_id *info_client[AMOUNT_PROC];  //база данных клиента
	struct _struct_client_id** info_client = new struct _struct_client_id*[AMOUNT_PROC];
	for(int i = 0; i < AMOUNT_PROC; i++) 
	{
		info_client[i] = new _struct_client_id;
	}
	
	void *proc_ptr_struct[AMOUNT_PROC];		// указатель на принятую структуру
	//void *client_ptr_struct[AMOUNT_PROC];
	
	for(int i = 0; i < AMOUNT_PROC; i++)
	{
		procID[i][0] = -1;
		procID[i][1] = 1;
		info_client[i]->clientId = -1;
		info_client[i]->procId = -1;
		proc_scoid[i][0] = -1;
		proc_scoid[i][1] = -1;
		info_client[i]->_struct_proc_transmission = NULL;
	}
	
	
	
	/* attach the name the client will use to find us */
	/* our channel will be in the attach structure */
	if ( (attach = name_attach( NULL, MY_SERV, 0 )) == NULL)
	{
		printf("SERVER:failed to attach name, errno %d\n", errno );
		exit(1);
	}
	printf( "SERVER: attach ID: %d %d\n", attach->chid, getpid());
	timeInit();
	struct _msg_info info;

	/* wait for the message from the client */
	while(1)
	{
		if(flagSendMsg)
		{
			msg.type = 0;
			rcvid = MsgReceive( attach->chid, &msg, SIZE_MAX_RECEIVE_BUFF, &info );
			if(msg.type != 0)
			{
				printf( "SERVER: CONECTED ID: %d TYPE: %d\n", rcvid, msg.type);
				printf( "SERVER: CONECTED : scoid %d\n", info.scoid);
			}
			if (rcvid == -1) 
			{/* Error condition, exit */
				errvalue = errno;
				printf( "SERVER: The error generated was %d\n", errvalue );
				printf( "SERVER: That means: %s\n", strerror( errvalue ) );
				continue;
			}
			if (rcvid == 0) 	// обрабатываем отсоеденение клиента и возможные ложные срабатывания
			{/* Pulse received */
				printf("SERVER: PULSE CODE %d\n", ((_pulse*)&msg)->code);
				if( ((_pulse*)&msg)->code == EXIT_SERV)
				{
					return 0;
					//break;
				}
				
				switch (msg.hdr.code) 
				{
					
					case TIME_PULSE_CODE:
					{
						//printf( "SERVER: TIMER \n");
						gotAPulse(info_client);
						break;
					}
					
					
					
					case _PULSE_CODE_DISCONNECT:
					{
						/*
						* A client disconnected all its connections (called
						* name_close() for each name_open() of our name) or
						* terminated
						*/
						printf("SERVER: DISCONNECT %d\n", msg.hdr.scoid);
						for(int index_s = 0; index_s < AMOUNT_PROC; index_s++)
						{
							if(proc_scoid[index_s][1] == msg.hdr.scoid)
							{
								printf( "SERVER: PROC DISCONNECT %d %d\n", proc_scoid[index_s][1], proc_scoid[index_s][0] );
								//printf( " %d %d \n", proc_scoid[index_s][1], proc_scoid[index_s][0] );
								proc_scoid[index_s][1] = -1;
								proc_scoid[index_s][0] = -1;
								procID[index_s][0] = -1;
								procID[index_s][1] = 1;
							}
						}
						//printf("\n");
						ConnectDetach(msg.hdr.scoid);
						break;
					}
					case _PULSE_CODE_UNBLOCK:
					{
						/*
						* REPLY blocked client wants to unblock (was hit by
						* a signal or timed out).  It's up to you if you
						* reply now or later.
						*/
						break;
					}
					
					default:
					{
	
						break;
					}
				}
				
				continue;
			}
		}
		flagSendMsg = true;
		
		switch(msg.type)
		{
			case CLIENT_TRANSMISSION_DATA:
			{
				printf("SERVER: 		CLIENT_TRANSMISSION_DATA\n");
				//printf("SERVER: CLIENT CONNECTED\n");
				MsgReply(rcvid, 0, NULL, 0);

				struct _struct_client_data * client_packet = new struct _struct_client_data;		//выделяем память под пакет
				//printf("SERVER: size: %d\n", sizeof(msg_data));
				//printf("SERVER: size: %d\n", sizeof(msg));
				int indexClientData = searchClientForID(rcvid, info_client);					//ищем по базе данных клиента
				memcpy(client_packet, &msg, sizeof(msg_data));								//копируем пакет в структуру клиента
				info_client[indexClientData]->_struct_client_transmission = client_packet;		//запоминаем пакет
				
				printf("NEW: _struct_client_data %d\n",indexClientData );
				
				info_client[indexClientData]->clientId = rcvid;									//запонинаем id
				int indexFreeProc = findFreeProc(procID);										//ищем свободный поток
				if(indexFreeProc == -1)
				{
					msg.type = 0;
					break;
				}
				printf("SERVER: index free proc %d\n", indexFreeProc);
				info_client[indexClientData]->procId = procID[indexFreeProc][0];				// привязываем клиент к потоку
				info_client[indexClientData]->_struct_proc_transmission = proc_ptr_struct[indexFreeProc];
				procID[indexFreeProc][1] = 1;
				
				// отсылаем пульс потоку привязанному к клиенту
				MsgDeliverEvent(procID[indexFreeProc][0], &(((struct _struct_proc_ready*)proc_ptr_struct[indexFreeProc])->event) );
				printf("SERVER: %d SEND TO PROC %d %d\n", rcvid,
						procID[indexFreeProc][0],
						((struct _struct_proc_ready*)proc_ptr_struct[indexFreeProc])->type );
				break;
			}
			
			case CLIENT_REQUEST_DATA:
			{
				printf("SERVER: 		CLIENT_REQUEST_DATA\n");
				int index_client = searchClientForID(rcvid, info_client);					//ищем по базе данных клиента
				
				struct _struct_client_cdata cdata;
				memcpy(&cdata.data, &(((_struct_proc_cdata*)info_client[index_client]->_struct_proc_transmission)->data), 
						sizeof(cdata.data));
				
				memcpy(&cdata.event, &(msg.event), sizeof(msg.event));

				MsgReply(rcvid, 0, &cdata , sizeof(cdata));
				info_client[index_client]->clientId = -1;
				//delete ((struct _struct_proc_cdata*)info_client[index_client]->_struct_proc_transmission); //удалили старую структуру
				delete ((struct _struct_proc_ready*)info_client[index_client]->_struct_client_transmission); //удалили старую структуру
				
				//printf("DELETE: (_struct_proc_cdata)_proc_transmission: %d\n", index_client);
				printf("DELETE: (_struct_proc_ready)client_transmission: %d\n", index_client);
				
				printf("SERVER: END\n\n");
				break;
			}
			
			case PROC_TRANSMISSION_DATA:
			{
				printf("SERVER: 		PROC_TRANSMISSION_DATA\n");
				MsgReply(rcvid, 0, NULL, 0);
				int index_proc = searchProcForID(rcvid, info_client);
				delete ((struct _struct_proc_ready*)info_client[index_proc]->_struct_proc_transmission); //удалили старую структуру
				printf("DELETE: (_struct_proc_ready)_struct_proc_transmission: %d\n", index_proc);
				struct _struct_proc_cdata * _proc_packet = new struct _struct_proc_cdata;
				struct _struct_proc_cdata msg_cdata;
				memcpy(_proc_packet, &msg, sizeof(msg_cdata));	
				info_client[index_proc]->_struct_proc_transmission = _proc_packet;
				
				printf("NEW: _struct_proc_cdata %d\n", index_proc );
				
				printf("SERVER: DATA %f\n", _proc_packet->data.f);
				MsgDeliverEvent(info_client[index_proc]->clientId, 
						&(((_struct_client_data*)info_client[index_proc]->_struct_client_transmission)->event));
				//printf("SERVER: SEND TO CLIENT\n");
				break;
			}
			
			case PROC_REQUEST_DATA:
			{
				printf("SERVER: 		PROC_REQUEST_DATA\n");
				//printf("SERVER: ERORR\n");
				
				int index_proc = searchProcForID(rcvid, info_client);
				if(index_proc == -1)
					break;
				
				memcpy(info_client[index_proc]->_struct_proc_transmission, &msg, sizeof(info_client[index_proc]->_struct_proc_transmission));	//копируем пакет в структуру клиента
				MsgReply(rcvid, 0, &((_struct_client_data*)info_client[index_proc]->_struct_client_transmission)->data, 
						sizeof(((_struct_client_data*)info_client[index_proc]->_struct_client_transmission)->data));
				
				//int index_procid = findProcId(procID, rcvid);
				//procID[index_procid][0] = -1;
				//procID[index_procid][1] = 0;
				
				//printf("SERVER: SEND TO PROC\n");
				break;
			}
				
			case PROC_READY:
			{
				printf("SERVER: 		PROC_READY\n");
				MsgReply(rcvid, 0, NULL, 0);
				struct _struct_proc_ready * procStruct = new _struct_proc_ready;	//выделяем пямять под принятый пакет
				int index_free = 0;
				
				while(procID[index_free][0] != rcvid)
				{
					index_free++;
					if(index_free >= AMOUNT_PROC)
						break;
					
				}
				if(index_free == AMOUNT_PROC)
				{
					index_free = 0;
					for(;(procID[index_free][0] != -1); index_free++);
				}
				procID[index_free][0] = rcvid;
				procID[index_free][1] = 0;					//поток готов
				
				proc_scoid[index_free][0] = rcvid;
				proc_scoid[index_free][1] = info.scoid;
				
				
				proc_ptr_struct[index_free] = procStruct;
				printf("NEW: _struct_proc_ready %d\n", index_free );
				struct _struct_proc_ready msg_p_ready;
				memcpy(procStruct, &msg, sizeof(msg_p_ready));	//копируем пакет
				//printf("SERVER: TYPE %d\n", ((_struct_proc_ready*)proc_ptr_struct[index_free])->type );
				printf("SERVER: PROC CONNECTED\n\n");
				int fPC = findPendingClient(info_client);
				if(fPC != -1)
				{
					flagSendMsg = false;
					
					info_client[fPC]->procId = rcvid;
					rcvid = info_client[fPC]->clientId;
					struct _struct_client_data msg_ready;
					memcpy(&msg, (_struct_client_data*)info_client[fPC]->_struct_client_transmission, sizeof(msg_ready));
					delete (_struct_client_data*)info_client[fPC]->_struct_client_transmission;
					
					printf("SERVER: SEND TO PROC %d %d\n",procID[index_free][0],
											((struct _struct_proc_ready*)proc_ptr_struct[index_free])->type );
					printf("DELETE: (_struct_client_data)client_transmission: %d\n", fPC);
											
					printf("SERVER: id pending client %d\n",info_client[fPC]->clientId);
					printf("SERVER: type %d\n", msg.type);
				}
				
				break;
			}
			
			case MONITOR_REQUEST_DATA:
			{
				//printf("SERVER: SEND TO MONITOR\n");
				struct _struct_monitor_request msg_to_monitor;
				for(int i = 0; i < AMOUNT_PROC; i++)
				{
					msg_to_monitor.data.procState[i][0] = procID[i][0];
					msg_to_monitor.data.procState[i][1] = procID[i][1];
					msg_to_monitor.data.procState[i][2] = info_client[i]->clientId;
				}
				MsgReply(rcvid, 0, &msg_to_monitor, sizeof(msg_to_monitor));
				break;
			}
			
			default:
			{
				printf("SERVER: DONT KNOWN TYPE %d\n", msg.type);
				continue;
				break;
			}
		
		}
	}

	for(int i = 0; i < AMOUNT_PROC; i++) delete[] procID[i];
		delete[] procID;
		
	for(int i = 0; i < AMOUNT_PROC; i++) delete[] info_client[i];	
		delete[] info_client;	
		
	return 0;
}

int searchClientForID(int id, _struct_client_id ** inf_client)
{
	int i = 0;
	for(;i < AMOUNT_PROC; i++ )
	{
		if(inf_client[i]->clientId == id)
		{
			break;
		}
	}
	if(i == AMOUNT_PROC)
	{
		i = 0;
		while(inf_client[i]->clientId != -1)
				i++;
	}
	if(i == AMOUNT_PROC)
	{
		printf("SERVER: NO FOUND\n");
		return 0;
	}
	
	return i; //index
}
/*
 * поиск по id потока
 */
int searchProcForID(int id, _struct_client_id ** inf_client)
{
	//printf("SERVER: FINDid %d\n ",id);
	int i = 0;
	for(;i < AMOUNT_PROC; i++ )
	{
		//printf("SERVER: FIND %d\n ",inf_client[i]->procId);
		if(inf_client[i]->procId == id)
			break;
	}
	if(i == AMOUNT_PROC)
	{
		printf("SERVER: NO FIND\n");
		return -1;
	}
	
	return i; //index
}


int findFreeProc(int ** arrID)
{
	printf("SERVER: FREE PROC: ");
	for(int j = 0; j < AMOUNT_PROC; j++)
	{
		printf("%d ", arrID[j][1]);
	}
	printf("\n");
	
	printf("SERVER: ID: ");
	for(int j = 0; j < AMOUNT_PROC; j++)
	{
		printf("%d ", arrID[j][0]);
	}
	printf("\n");
	
	int i = 0;
	for(;i < AMOUNT_PROC; i++)
	{
		if(arrID[i][1] == 0)
			break;
	}
	
	if(i == AMOUNT_PROC)
	{
		printf("SERVER: dont free proc\n");
		return -1;
	}
	
	
	return i;	//index
}

int findProcId(int ** arrID, int id)
{
	printf("SERVER: FREE PROC: ");
	for(int j = 0; j < AMOUNT_PROC; j++)
	{
		printf("%d ", arrID[j][1]);
	}
	printf("\n");
	
	printf("SERVER: ID: ");
	for(int j = 0; j < AMOUNT_PROC; j++)
	{
		printf("%d ", arrID[j][0]);
	}
	printf("\n");
	
	int i = 0;
	for(;i < AMOUNT_PROC; i++)
	{
		if(arrID[i][0] == id)
			break;
	}

	if(i == AMOUNT_PROC)
	{
		printf("SERVER: dont find\n");
		return -1;
	}
	
	return i;	//index
}

void timeInit()
{
	struct sigevent         event;
	struct itimerspec       itime;
	timer_t                 timer_id;
	int                     chid;

	if ( (chid = name_open( MY_SERV, 0  )) == -1)
	{
		printf("failed to find server, errno %d\n", errno );
		exit(1);
	}

	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = chid;
	event.sigev_priority = getprio(0);
	event.sigev_code = TIME_PULSE_CODE;
	timer_create(CLOCK_REALTIME, &event, &timer_id);

	itime.it_value.tv_sec = 1;
	/* 500 million nsecs = .5 secs */
	itime.it_value.tv_nsec = 0; 
	itime.it_interval.tv_sec = 1;
	/* 500 million nsecs = .5 secs */
	itime.it_interval.tv_nsec = 0; 
	timer_settime(timer_id, 0, &itime, NULL);
	printf("SERVER: TIMER INIT\n");
}

void gotAPulse( _struct_client_id ** inf_client)
{

	for(int i = 0; i < AMOUNT_PROC; i++)
	{
		if(inf_client[i]->clientId != -1)
		{
			if(--((_struct_client_data*)inf_client[i]->_struct_client_transmission)->timeOut == 0)
			{
				printf("SERVER: CLIENT TIMEOUT %d %d\n", inf_client[i]->clientId, TIMEOUT);
				((_struct_client_data*)inf_client[i]->_struct_client_transmission)->event.sigev_code = TIMEOUT;
				MsgDeliverEvent(inf_client[i]->clientId, 
						&(((_struct_client_data*)inf_client[i]->_struct_client_transmission)->event));
				inf_client[i]->clientId = -1;
				if(inf_client[i]->procId != -1)
				{
					((_struct_client_data*)inf_client[i]->_struct_proc_transmission)->event.sigev_code = TIMEOUT;
					MsgDeliverEvent(inf_client[i]->procId, 
								&(((_struct_client_data*)inf_client[i]->_struct_proc_transmission)->event));
				}
				inf_client[i]->procId = -1;
			}
		}
	}
}


int findPendingClient( _struct_client_id ** inf_client)
{
	int arrayIndex[2];
	arrayIndex[0] = 1000;
	arrayIndex[1] = -1;
	for(int i = 0; i < AMOUNT_PROC; i++)
	{
		if((inf_client[i]->clientId != -1) && (inf_client[i]->procId == -1))
		{
			//printf("find %d %d\n", inf_client[i]->clientId, inf_client[i]->procId);
			
			if(arrayIndex[0] >= ((_struct_client_data*)inf_client[i]->_struct_client_transmission)->timeOut)
			{
				arrayIndex[0] = ((_struct_client_data*)inf_client[i]->_struct_client_transmission)->timeOut;
				arrayIndex[1] = i;  
			}
		}
	}
	return arrayIndex[1];
}
