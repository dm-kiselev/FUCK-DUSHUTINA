#ifndef IMSL_H_
#define IMSL_H_

#include <stdio.h>	/*printf*/

class extrapolation
{
	public:
		extrapolation();
		float calcExtrapolationXY(float *x, float *f, int n, float x1);
		float calcExtrapolationXY(float *x, float *y, int n, float x_min, float x_max, float step, float *data);
	private:
		float calcL(float *x, int n, float x1, int num);
};

#endif /*IMSL_H_*/
