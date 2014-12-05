#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <process.h>
#include <sys/neutrino.h>
#include <sys/procfs.h>
#include <sys/procmgr.h>
#include <ha/ham.h>
#include <fstream>
#include <sys/dispatch.h>
#include <math.h>
#define ATTACH_POINT "inter_server"
typedef struct {
	double x;
	double y;
} point;
typedef struct
{
    // 0 - успешно
    int messageType;
    // данные
    point data;
} ClientMessageT;
using namespace std;
int ham = 1;
int main(int argc, char *argv[])
{
	char *inetdpath;
	int step = 0;
	ham_entity_t *ehdl;
	ham_condition_t *chdl;
	ham_action_t *ahdl;
	ClientMessageT mdata;
	printf("started\n");
	point data={0,0};
	point cdata ={0,0};
	point ndata={0,0};
	mdata.messageType=2;
	mdata.data.x=ndata.x;
	mdata.data.y=ndata.y;
	char* last_data="icc_log";
	char* all_data="ic_log";

	FILE* fd;
	int  connect_id, flag;
	inetdpath = strdup("/home/gr10/inter/inter_cont -l");
	if(ham)
	{
	/* attach to ham */
	//inter_proc- имя, 2000000000 - 2c, ожидаемая частота heart_break,
	// 5 - количество пропущенный сигналов до сробатывания условия, т.о - 10с
	ehdl = ham_attach_self("intet-proc",2000000000,5 ,5, 0);
	chdl = ham_condition(ehdl, CONDHBEATMISSEDHIGH, "heartbeat-missed-high", 0);
	ahdl = ham_action_execute(chdl, "Restart_proc",
			"slay -f inter_cont", 0);//убить

	}
	while(1)
	{
		switch (step)
		{
		case 0:
			//загрузить из сохранения последние данные, если есть файл
			fd = fopen(last_data, "r");
			if (fd)
			{
				fread(&data, sizeof(data), 1, fd);
				fclose(fd);
			}
			else
			{
				printf("Cont: File open fail\n");
			}
			//открыть заново, но уже с перезаписью
			step=1;
		case 1:
			if (strcmp(argv[1],"-l") == 0)		flag = 0;
				else if (strcmp(argv[1],"-g") == 0)	flag = NAME_FLAG_ATTACH_GLOBAL;
				else {
					cerr << "Cont: cmd args error!" << endl;
					exit(1);
				}
			printf("Cont: Try to connect\n");
				// Открытие канала //
				connect_id = name_open( ATTACH_POINT, flag );
				if (connect_id == -1) {
					step = 4;//немного подождать
					break;
				}
			//подключение к серверу
			step=2;	//успешно, начать работу
		case 2:
			//убедить, что жив вообще
			if (ham)ham_heartbeat();
			printf("Cont: Ask for data\n");
			//послать запрос, ждать ответа
			mdata.messageType=2;
			if (MsgSend( connect_id, &mdata, sizeof(mdata), &mdata, sizeof(mdata) )==-1)
			{
				step=4;
				break;
			}
			else{
				printf("Cont:%d\n", mdata.messageType);
			if (mdata.messageType==0){
							//сохранить в файл и сгенерировать новые данные
							step =3;
						}//иначе пытаться снова получить предыдущие данные
			else
			{
				break;
			}
			}
		case 3:
			//расчитать и сохранить в файл
			ndata.x=mdata.data.x;
			ndata.y=mdata.data.y;

			cdata.x=(data.x+ndata.x)/2;
			cdata.y=(data.y+ndata.y)/2;
			data.x=ndata.x;
			data.y=ndata.y;

			printf("x0 = %f, y0= %f, x1 =%f, y1=%f\n", ndata.x,ndata.y, cdata.x, cdata.y);
			fd = fopen(all_data, "a");
			if (fd)
			{
				//fwrite(&cdata, sizeof(cdata), 1, fd);
				//fwrite(&ndata, sizeof(ndata), 1, fd);
				fprintf(fd, "x = %f, y= %f\nx=%f, y=%f\n", cdata.x, cdata.y, ndata.x, ndata.y);
				fclose (fd);
			}
			else
				printf("Cont: File log open fail\n");

			fd = fopen(last_data, "w");
			if (fd)
			{
			fsetpos(fd,0);
			fwrite(&ndata, sizeof(ndata), 1, fd);
			fclose (fd);
			}
			else
			{
				printf("Cont: File log2 open fail\n");
			}
			step=2;
			break;
		case 4:
			//ожидание перезапуска сервера
			if (ham)ham_heartbeat();//еще жив
			sleep(1);
			step=1;//попробовать заново подключиться
			break;
		}
	}

	return 0;
}
