#include "logger.h"
#include <stdio.h>
#include <stdlib.h>

FILE *pflog, *pferr;

int log_start(const char* file_name_log, const char* file_name_err) {
	pflog = freopen(file_name_log, "w", stdout);
	pferr = freopen(file_name_err, "w", stderr);
	return EXIT_SUCCESS;
}

int log_update() {
	fflush(stdout);
	fflush(stderr);
}

int log_stop() {
	fclose(pflog);
	fclose(pferr);
	return EXIT_SUCCESS;
}
