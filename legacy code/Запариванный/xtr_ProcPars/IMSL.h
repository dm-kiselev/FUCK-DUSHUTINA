#ifndef IMSL_H_
#define IMSL_H_

#include <stdio.h>	/*printf*/

class extrapolation
{
	public:
		extrapolation();
		float calcExtrapolationXY(float *x, float *f, int n, float x1);
	private:
		float calcL(float *x, int n, float x1, int num);
};

#endif /*IMSL_H_*/
