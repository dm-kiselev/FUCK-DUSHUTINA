#include <cstdlib>
#include <iostream>

/* open file*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
/*end open file*/
#include <unistd.h> /*read*/

#include <stdarg.h>			/*tracelogger*/
#include <sys/trace.h>		/*tracelogger*/
#include <string.h>
#include <string>

#include <stdio.h>	/*printf*/

#include <errno.h>

#include "main.h"
#include "IMSL.h"

void trace_vnlogfr(int eventNum, std::string const str, std::string const format, ...);

int main(int argc, char *argv[]) 
{

	float x[50];
	float y[50];
	int n = 5;
	for(int i = 0; i < n; i++)
	{
		x[i] = i;
		y[i] = i * i;
	}
	for(int i = 0; i < n; i++)
		printf("input x: %f y: %f\n", x[i],  y[i] );
	
	float data[200];
	extrapolation line;
	float x_min = 20, x_max = 30, step = 0.5;
	line.calcExtrapolationXY(x, y, n, x_min, x_max, step, data );
	
	for(int i = 0; i < (x_max - x_min) / step + 1; i++)
		printf("result: %f error: %f\n", data[i],  data[i] - (i * step + x_min) * (i * step + x_min) );
	
	return EXIT_SUCCESS;
}

void trace_vnlogfr(int eventNum, std::string const str, std::string const format, ...)
{
	va_list ap;
	va_start(ap, format);
	trace_vnlogf(eventNum, 50, str.c_str() , ap);
	va_end(ap);
}
