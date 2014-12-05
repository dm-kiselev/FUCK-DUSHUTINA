#ifndef XTR_STRUCT_H_
#define XTR_STRUCT_H_

#define MY_SERV "IMSL"

#define IOV_data 2

typedef enum
{
	ENUM_CLIENT_TRANSMISSION_DATA	= 33,
	ENUM_CLIENT_REQUEST_DATA 		= 34,
	ENUM_PROC_REQUEST_DATA			= 35,
};
#define AMOUNT_CLIENT 20

#define AMOUNT_POINT 4
typedef struct _client_data_to_calc
{
	float x[AMOUNT_POINT];
	float y[AMOUNT_POINT];
	float x_min;
	float x_max;
	float step;
};


#endif /*XTR_STRUCT_H_*/
