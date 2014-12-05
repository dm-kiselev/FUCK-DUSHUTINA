#ifndef XTR_STRUCT_H_
#define XTR_STRUCT_H_

#define MY_PULSE_CODE _PULSE_CODE_MINAVAIL + 5

#define CLIENT_TRANSMISSION_DATA 	_IO_MAX + 4
#define CLIENT_REQUEST_DATA 		_IO_MAX + 5
#define PROC_TRANSMISSION_DATA 		_IO_MAX + 6
#define PROC_REQUEST_DATA 			_IO_MAX + 7
#define PROC_READY		 			_IO_MAX + 8
#define MONITOR_REQUEST_DATA		_IO_MAX + 9

#define TIMEOUT			 			_PULSE_CODE_MINAVAIL + 2
#define EXIT_SERV			 		_PULSE_CODE_MINAVAIL + 4

#define MY_SERV "IMSL"

#define AMOUNT_PROC 20

typedef struct _pulse msg_header_t;

#define AMOUNT_POINT 4
struct data_calc_struct	//данные для расчетов
{
	float x[AMOUNT_POINT];
	float y[AMOUNT_POINT];
	float x_min;
	float x_max;
	float step;
};

struct data_processed
{
	float f[100];
};

struct receive_msg
{
	msg_header_t hdr;
	short type;
	struct sigevent event;
	short timeOut;
	char data[100000];
};

//	PROC READY
struct _struct_proc_ready
{
	msg_header_t hdr;
	short type;
	struct sigevent event;
};

//CLIENT DATA
struct _struct_client_data
{
	msg_header_t hdr;
	short type;
	struct sigevent event;
	short timeOut;
	struct data_calc_struct data;
};
//CLIENT СDATA
struct _struct_client_cdata
{
	msg_header_t hdr;
	short type;
	struct sigevent event;
	short timeOut;
	struct data_processed data;
};

//PROC SEND DATA
struct _struct_proc_cdata
{
	msg_header_t hdr;
	short type;
	struct sigevent event;
	struct data_processed data;
};

struct _struct_monitor_data
{
	int procState[AMOUNT_PROC][3];
};

struct _struct_monitor_request
{
	msg_header_t hdr;
	short type;
	struct sigevent event;
	struct _struct_monitor_data data;
};

#endif /*XTR_STRUCT_H_*/
