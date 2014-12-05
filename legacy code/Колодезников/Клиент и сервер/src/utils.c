#include "includes.h"

static int counter = 0;

void create_msg(const char * progname, char * buffer) {
	pid_t pid = getpid();
	char pid_s[20];
	strcpy(buffer, "USRPROGRAM ");
	strcat(buffer, progname);
	strcat(buffer, " is ");
	itoa(pid, pid_s, 10);
	strcat(buffer, pid_s);
}

void trace_config() {
	TraceEvent(_NTO_TRACE_SETALLCLASSESFAST);
	TraceEvent(_NTO_TRACE_ADDALLCLASSES);
}

void trace_msg_start() {
	TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, counter++, "Start");
}

void trace_msg_stop() {
	TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, counter++, "Stop");
}

void trace_msg(const char * msg) {
	TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, counter++, msg);
}

void trace_start() {
	TraceEvent(_NTO_TRACE_START);
}

void trace_stop() {
	TraceEvent(_NTO_TRACE_STOP);
}
