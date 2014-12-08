#ifndef UTILS_H_
#define UTILS_H_

void trace_config();
void trace_msg(const char * msg);
void trace_start();
void trace_stop();
void trace_msg_start();
void trace_msg_stop();
void create_msg(const char * progname, char * buffer);

# define __ERROR_EXIT(__t, __v, __m)				\
if (__t == __v) {									\
	perror(__m); 									\
	return EXIT_FAILURE;							\
}

# define __ERROR_CONT(__t, __v, __m)				\
if (__t == __v) {									\
	perror(__m); 									\
	continue;										\
}

# define __ERROR_CHECK(__t, __v, __m)				\
if (__t == __v) {									\
	perror(__m); 									\
}

# define SIGEV_GET_VALUE(__e)					\
	((__e)->__data.__proc.__pdata.__kill.__value.sival_int)

#endif /* UTILS_H_ */
