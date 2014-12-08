#include <math.h>
#include "spline.h"
#include <stdio.h>
#include <stdlib.h>

#define SIZE_SRC 8
#define SIZE_TAR 40
#define STEP_SRC 0.5
#define STEP_TAR 0.1

int main() {

	double_t xs[SIZE_SRC], ys[SIZE_SRC],
			xt[SIZE_TAR], yt[SIZE_TAR], ye[SIZE_TAR],
			v;
	spline_t spline;

	v=0;
	for(size_t i=0; i<SIZE_SRC; ++i, v+=STEP_SRC) {
		xs[i] = v;
		ys[i] = sin(v) + 2;
	}
	spline_init(&spline, xs, ys, SIZE_SRC);

	v=0;
	for(size_t i=0; i<SIZE_TAR; ++i, v+=STEP_TAR) {
		xt[i] = v;
		yt[i] = sin(v) + 2;
		ye[i] = spline_getvalue(&spline, v);
		printf("x %f, target %f, spline %f, error %f\n", xt[i], yt[i], ye[i], yt[i]-ye[i]);
	}
	spline_destroy(&spline);
	return EXIT_SUCCESS;
}
