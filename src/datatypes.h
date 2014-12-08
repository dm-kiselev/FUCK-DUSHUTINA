#ifndef DATATYPES_H_
#define DATATYPES_H_
// Includes
#include <inttypes.h>
#include <math.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdlib.h>
#include "spline.h"
#include <sys/neutrino.h>
// Defines
#define PULSE_CODE_DATA_READY _PULSE_CODE_MINAVAIL+5
#define PULSE_CODE_DATA_ERROR PULSE_CODE_DATA_READY+1
#define PULSE_CODE_TIME_ERROR PULSE_CODE_DATA_ERROR+1

// Data types
typedef struct _pulse header_t;

typedef enum {
	SPLINE_INIT,
	SPLINE_DESTROY,
	SPLINE_GETVAL,
	TASK_GETVAL,
	CLIENT_UNBLOCK,
	IMG_CLEAR,
	IMG_DRAW
} cmd_t;

typedef enum {
	DOWN,
	READY,
	WORK,
	FULL,
	SEND,
	ERR,
	EMPTY
} status_t;

typedef struct {
	cmd_t cmd;
	size_t n;
	void *px;
} task_t;

typedef struct {
	header_t header;			// Structure that describes a pulse
	uint32_t cid;
	uint32_t fid;
	struct timespec timeout;
	size_t size;
	task_t *ptask;
} frame_t;

typedef struct {
	frame_t framerep;
	spline_t spline;
} cash_t;		// structure that describes a client's data

typedef struct {
	int rcvid;
	status_t status;
} syncsig_t;

typedef struct {
	cash_t *pcash;
	size_t id;
	uint32_t client_amount;
	uint32_t wrk_amount;
	syncsig_t* psync;
	int chid;
} wrk_info_t;			// structure that describes a works
// Public functions

#endif /* DATATYPES_H_ */
