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

#include "IMSL.h"

#define SIZE_DATA_INPUT 5

typedef struct _pulse msg_header_t;

struct my_msg
{
	msg_header_t hdr;
	short type;
	struct sigevent event;
	int data;
	float x[SIZE_DATA_INPUT];
	float y[SIZE_DATA_INPUT];
	float min;
	float max;
	float step;
};

struct _result_proc
{
   int data;
};

#define MY_PULSE_CODE 	_PULSE_CODE_MINAVAIL+5
#define MSG_GIVE_PULSE 	_IO_MAX + 4
#define MSG_GIVE_RESULT _IO_MAX + 5
#define PROC_GIVE_PULSE _IO_MAX + 6
#define PROC_GIVE_RESULT _IO_MAX + 7

#define MY_SERV "my_server_name"

#define AMOUNT_THREAD 10

void *proc(void*  arg);
void handler(int signo, siginfo_t *info, void *other);
int find_rcvid(int id);

char 	chid[AMOUNT_THREAD];
int 	rcvid[AMOUNT_THREAD];		//id клиентов
int 	flagProc[AMOUNT_THREAD];	// масив флагов 0 - поток свободен, 1 - поток занят. Индекс номер потока
struct my_msg msg;					// данные от клиента
struct _result_proc *addresRes[AMOUNT_THREAD];
struct _input_proc *addresInp[AMOUNT_THREAD];

int main( int argc, char **argv)
{
	pthread_t 			proc_tid[AMOUNT_THREAD];	// массив tid потоков
	int 				errvalue;
	struct _result_proc resultData;				//данные полученные в результате вычислений на сервере
	name_attach_t 		*attach;
	static int numProc = 0;
	int tmp_rcvid = 0;
	
	// init signal
    struct sigaction	act;
    sigset_t 			set;
    sigemptyset( &set );
    sigaddset( &set, SIGUSR1 );
    act.sa_flags = 0;
    act.sa_mask = set;
    act.sa_sigaction = &handler;
    sigaction( SIGUSR1, &act, NULL );
    
    for(int i = 0; i < AMOUNT_THREAD; i++)	// init
    {
    	flagProc[i] = 0;
    	rcvid[i] = -1;
    }
	/* attach the name the client will use to find us */
	/* our channel will be in the attach structure */
	if ( (attach = name_attach( NULL, MY_SERV, 0 )) == NULL)	//объявляем имя сервера
	{
		printf("SERVER:failed to attach name, errno %d\n", errno );
		exit(1);
	}
	
	while(1)
	{
		/*
		 * ждем пока не прийдет сообщение от клиента
		 */
		tmp_rcvid = MsgReceive( attach->chid, &msg, sizeof( msg ), NULL );
		/*
		 * В то время пока сервер находится в recieve блокировке,
		 *  может закончить работу поток и отправить клиенту пульс.
		 * Блокировка сервера снимается с кодом ошибки 4, tmp_rcvid == -1
		 */
		if (tmp_rcvid == -1) 
		{/* Error condition, exit */
			errvalue = errno;
			printf( "SERVER: The error generated was %d\n", errvalue );
			printf( "SERVER: That means: %s\n", strerror( errvalue ) );
			continue;
		}
		if (rcvid == 0) 	// обрабатываем отсоеденение клиента и возможные ложные срабатывания
		{/* Pulse received */
			switch (msg.hdr.code) 
			{
				case _PULSE_CODE_DISCONNECT:
					/*
					* A client disconnected all its connections (called
					* name_close() for each name_open() of our name) or
					* terminated
					*/
					ConnectDetach(msg.hdr.scoid);
				break;
				case _PULSE_CODE_UNBLOCK:
					/*
					* REPLY blocked client wants to unblock (was hit by
					* a signal or timed out).  It's up to you if you
					* reply now or later.
					*/
				break;
				default:
					/*
					* A pulse sent by one of your processes or a
					* _PULSE_CODE_COIDDEATH or _PULSE_CODE_THREADDEATH
					* from the kernel?
					*/
				break;
			}
			continue;
		}
		printf("SERVER: RECEIVE tmp_rcvid: %d\n", tmp_rcvid);
		numProc = find_rcvid(tmp_rcvid);
		rcvid[numProc] = tmp_rcvid;
		printf("SERVER:numProc: %d\n", numProc);
		/* wait for the message from the client */
		switch(msg.type)
		{
			case MSG_GIVE_PULSE: // прием данных от клиента
				MsgReply(rcvid[numProc], 0, NULL, 0);
				addresRes[numProc] = new struct _result_proc;	// выделяем память
				pthread_create(&proc_tid[numProc], NULL, &proc, (void *) &numProc); // запускаем поток
				printf("SERVER: delivered event\n");
				break;
			
			case MSG_GIVE_RESULT: // отправляем клиенту данные

				MsgReply(rcvid[numProc], EOK, addresRes[numProc], sizeof(resultData));
				rcvid[numProc] = -1;
				
				printf("SERVER: reply message \n");
				if(addresRes[numProc] != NULL)
				{
					delete addresRes[numProc];
					addresRes[numProc] = NULL;
				}
				break;

			default:
				printf("SERVER: unexpected message \n");
				break;
		}
		
	}

	return 0;
}

void *proc(void*  arg)	// полезная нагрузка
{
	int x; 
	x = *((int *) arg);
	flagProc[x] = 1;
	extrapolation line;
	printf("PROC: create %d\n", x);
	// init signsl
    struct sigaction act;
    sigset_t set;
    sigemptyset( &set );
    sigaddset( &set, SIGUSR1 );
    act.sa_flags = 0;
    act.sa_mask = set;
    act.sa_sigaction = &handler;
    sigaction( SIGUSR1, &act, NULL );

    sleep(3);
    addresRes[x]->data = 17 * (x + 1);
    
	SignalKill(0, getpid(), NULL, SIGUSR1, -x , 0 );	// по готовности вызываем обработчик, а из него посылаем пульс клиенту
	flagProc[x] = 0;	// поток свободен
	pthread_exit(NULL);
}

void handler(int signo, siginfo_t *info, void *other)	// обработчик
{
	//printf("SERVER: RECEIVE si_signo: %d\n", info->si_signo );
	printf("SERVER: RECEIVE si_code: %d\n",  info->si_code );
	printf("HANDLER: RECEIVE si_value: %d\n", info->si_value.sival_int );
	printf("HANDLER:  rcvid: %d\n", rcvid[-info->si_code]);
	
	printf("HANDLER: flagProc ");
	for(int i = 0; i < AMOUNT_THREAD; i++ )
		printf("%d ", flagProc[i]);
	printf("\n");
	
	MsgDeliverEvent(rcvid[-info->si_code ], &msg.event );	//отправляем пульс о готовности данных клиенту
	
}

int find_rcvid(int id)	// поиск свободных потоков и сравнение id с базой клиентов
{
	int i = 0;
	for(;i < AMOUNT_THREAD; i++ )
	{
		if(rcvid[i] == id)
			break;
	}
	if(i == AMOUNT_THREAD)
	{
		i = 0;
		while(flagProc[i])
			i++;
	}
	
	return i;
}
