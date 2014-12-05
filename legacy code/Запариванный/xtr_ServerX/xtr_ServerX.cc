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

#define SIZE_MAX_RECEIVE_BUFF 1000000

typedef struct _message_recieve
{
	io_write_t whdr;
	char buff[1000];
};

struct _info_client
{
	int clientId;
	int procId;
	iov_t *iov;
	pthread_t proc;
};

struct info_client
{
private:
	struct _info_client info_client[AMOUNT_CLIENT];
	int findFreeIndex(int id);
public:
	void init();
	void addThread(int id, pthread_t proc_t, iov_t *iov);
};

void createNewThread(iov_t *iov_client, int rcvid);
void *server(void*  arg);

name_attach_t *attach;
struct info_client s_info_client;

int main( int argc, char **argv)
{
	int rcvid = -1;
	int errvalue;	
	
	struct _message_recieve msg;
	struct _msg_info info;
	typedef struct _pulse msg_header_t;
	struct _client_data_to_calc client_data_to_calc;
	
	s_info_client.init();
/************************************************/
	iov_t iov[3];
	io_write_t whdr;
	char receive_buf[25];
	struct sigevent event;
	SETIOV(iov + 0, &whdr, sizeof(whdr));
	SETIOV(iov + 1, &event, sizeof(event));
/***********************************************/	
	
	/* attach the name the client will use to find us */
	/* our channel will be in the attach structure */
	if ( (attach = name_attach( NULL, MY_SERV, 0 )) == NULL)
	{
		printf("[SERVER] failed to attach name, errno %d\n", errno );
		exit(1);
	}

	/* wait for the message from the client */
	while(1)
	{
		rcvid = MsgReceive( attach->chid, &msg, SIZE_MAX_RECEIVE_BUFF, &info );
		printf("[SERVER] Connected: %d scoid: %d\n", rcvid, ((_pulse*)&msg)->scoid);
		if (rcvid == -1) 
		{/* Error condition, exit */
			errvalue = errno;
			printf( "[SERVER] The error generated was %d\n", errvalue );
			printf( "[SERVER] That means: %s\n", strerror( errvalue ) );
			continue;
		}
		if (rcvid == 0) 	// обрабатываем отсоеденение клиента и возможные ложные срабатывания
		{/* Pulse received */
			printf( "[SERVER] Receive pulse \n" );
			
			switch (((_pulse*)&msg)->code) 
			{
				case _PULSE_CODE_DISCONNECT:
				{
					/*
					* A client disconnected all its connections (called
					* name_close() for each name_open() of our name) or
					* terminated
					*/
					printf("[SERVER] Disconnected: %d\n", ((_pulse*)&msg)->scoid);
					ConnectDetach(((_pulse*)&msg)->scoid);
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

		
		switch(msg.whdr.i.type)
		{
			case ENUM_CLIENT_TRANSMISSION_DATA:
			{
				printf("[SERVER] size:%d\n", msg.whdr.i.nbytes);
				struct _client_data_to_calc *msg_data = new _client_data_to_calc;	//выделяем место под принятой сообщение
				iov_t *iov_client = new iov_t;
				SETIOV(iov + 2, msg_data, sizeof(client_data_to_calc));
				MsgReadv(rcvid, iov + 2, 2,  sizeof(whdr) + sizeof(event));
				MsgReadv(rcvid, iov + 1, 1,  sizeof(whdr));	//копируем сообщение
				
				//printf( "[SERVER] ERROR %f\n", ((_client_data_to_calc*)GETIOVBASE(iov + 2))->step);
				
				whdr.i.type = ENUM_CLIENT_TRANSMISSION_DATA;
				whdr.i.nbytes = sizeof(client_data_to_calc) + sizeof(event);
				MsgReply(rcvid, 0, NULL, 0);
				
				createNewThread(iov_client, rcvid);
				
				MsgDeliverEvent(rcvid, (sigevent*)GETIOVBASE(iov + 1));

				delete msg_data;
				break;
			}
			
			case ENUM_CLIENT_REQUEST_DATA:
			{

				break;
			}

			case ENUM_PROC_REQUEST_DATA:
			{

				break;
			}
			
			default:
			{
				printf("[SERVER] DONT KNOWN TYPE %d\n", msg.whdr.i.type);
				continue;
				break;
			}
		
		}
	}
		
	return 0;
}

void createNewThread(iov_t *iov_client, int rcvid)
{
	pthread_t server_tid;
	s_info_client.addThread(rcvid, server_tid, iov_client);
	pthread_create(&server_tid, NULL, &server, NULL);
}

void *server(void*  arg)
{
	sleep(1);
	
	MsgSendPulse(attach->chid, 0, ENUM_PROC_REQUEST_DATA, 0);
	
	pthread_exit(NULL);
}



/************************************************/
void info_client::init()
{
	for(int i = 0; i < AMOUNT_CLIENT; i++)
	{
		info_client[i].clientId = -1;
		info_client[i].procId = -1;
	}
}

void info_client::addThread(int id, pthread_t proc_t, iov_t *iov)
{
	int index = findFreeIndex(id);
	info_client[index].clientId = id;
	info_client[index].proc = proc_t;
	info_client[index].iov = iov;
}

int info_client::findFreeIndex(int id)
{
	int i = 0;
	for(;i < AMOUNT_CLIENT; i++)
	{
		if(info_client[i].clientId != -1)
			break;
	}
	if( i == AMOUNT_CLIENT)
		return -1;
	return i;
}
