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
#include <sys/dispatch.h>
#include <math.h>
#define ATTACH_POINT "inter_server"
typedef struct {
	double x;
	double y;
} point;
typedef struct {
	// contains both message to and from client
	int messageType;
	// optional data, depending upon message
	point data;
} ClientMessageT;
using namespace std;
int ham = 1;
int main(int argc, char *argv[]) {
	char *inetdpath;
	printf("Proc start\n");
	int step = 0;
	ham_entity_t *ehdl;
	ham_condition_t *chdl;
	ham_action_t *ahdl;
	point data;
	data.x = 0.0;
	data.y = 0.0;
	ClientMessageT mdata;
	ClientMessageT adata;
	mdata.messageType = 3;
	FILE *fd;
	char* fname = "ip_log";
	int connect_id, flag;
	inetdpath = strdup("/home/gr10/inter/inter_proc -l");
	if (ham) {
		/* attach to ham */
		//inter_proc- имя, 2000000000 - 2c, ожидаемая частота heart_break,
		// 5 - количество пропущенный сигналов до сробатывания условия, т.о - 10с
		ehdl = ham_attach_self("intet-proc", 2000000000, 5, 5, 0);
		chdl = ham_condition(ehdl, CONDHBEATMISSEDHIGH,
				"heartbeat-missed-high", 0);
		ahdl
				= ham_action_execute(chdl, "Restart_proc",
						"slay -f inter_proc", 0);//убить если завис

	}
	while (1) {
		switch (step) {
		case 0:
			//загрузить из сохранения последние данные, если есть файл
			fd = fopen(fname, "r");
			if (fd != NULL) {
				fread(&data, sizeof(data), 1, fd);
				fclose(fd);
			} else {
				printf("Proc: File open fail\n");
			}
			//открыть заново, но уже с перезаписью
			step = 1;
		case 1:

			if (strcmp(argv[1], "-l") == 0)
				flag = 0;
			else if (strcmp(argv[1], "-g") == 0)
				flag = NAME_FLAG_ATTACH_GLOBAL;
			else {
				cerr << "Proc: cmd args error!" << endl;
				exit(1);
			}
			printf("Proc: Try to connect\n");
			// Открытие канала //
			connect_id = name_open(ATTACH_POINT, flag);
			if (connect_id == -1) {
				step = 4;//немного подождать
				break;
			}
			//подключение к серверу
			step = 3; //успешно, начать работу
		case 2:
			//убедить, что жив вообще
			printf("Proc: Generate and send! \n");
			//сгенерировать данные
			data.x = data.x + 0.2;
			if (data.x > 10)
				data.x = 0;
			data.y = data.x * data.x;
			printf("Send: x0 = %f, y0= %f\n", data.x, data.y);

			step = 3;
		case 3:
			if (ham)
				ham_heartbeat();
			//послать, ждать ответа
			mdata.data.x = data.x;
			mdata.data.y = data.y;
			mdata.messageType = 3;
			delay(100);
			printf("Proc: Try to send! \n");
			printf("Send: x0 = %f, y0= %f\n", mdata.data.x, mdata.data.y);
			if (MsgSend(connect_id, &mdata, sizeof(mdata), &adata,
					sizeof(mdata)) == -1) {
				step = 4;
			} else {
				printf("Proc:%d\n", adata.messageType);
				if (adata.messageType == 0) {
					//сохранить в файл и сгенерировать новые данные
					fd = fopen(fname, "w");
					if (fd) {
						fwrite(&data, sizeof(data), 1, fd);
						fclose(fd);
					} else
						printf("Proc: File log open fail\n");
					step = 2;
				} else {
					//иначе пытаться снова отправить предыдущие данные
					sleep(1);
					printf("Proc: Not OK\n");
				}
			}

			break;
		case 4:
			//ожидание перезапуска сервера
			if (ham)
				ham_heartbeat();//еще жив
			sleep(1);
			printf("Proc: No connect, try again\n");
			step = 1;//попробовать заново подключиться
			break;
		}
	}

	return 0;
}
