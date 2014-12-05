#include "IMSL.h"

extrapolation::extrapolation()
{

}

float extrapolation::calcExtrapolationXY(float *x, float *y, int n, float x1)
{
	float L = 0;
	for(int i = 0; i < n; i++)
	{
		//printf("L %f %f\n", L, y[i]);
		L += y[i] * calcL(x, n, x1, i);
	}
	return L;
}

float extrapolation::calcExtrapolationXY(float *x, float *y, int n, float x_min, float x_max, float step, float *data)
{
	if (step == 0)
	{
		printf("IMSL: step = 0\n");
		return -1;
	}
	float iter = (x_max - x_min) / step + 1;
	if (x_max < x_min)
	{
		printf("IMSL: x_max < x_min \n");
		return -1;
	}
	
	for(int j = 0; j < iter; j++)
	{
		double L = 0;
		for(int i = 0; i < n; i++)
		{
			L += y[i] * calcL(x, n, x_min + j * step, i);
		}
		//printf("IMSL: data: %f\n", L);
		data[j] = L;
	}
	
	return 0;
}

float extrapolation::calcL(float *x, int n, float x1, int num)
{
	double xCurrent = 0;
	int j = 2;
	if(num == 1)
		xCurrent = (x1 - x[0]) / ( x[num] - x[0]);
	if(num == 0)
		xCurrent = (x1 - x[1]) / ( x[num] - x[1]);
	if((num != 0) && (num != 1))
	{
		xCurrent = (x1 - x[0]) / ( x[num] - x[0]);
		j = 1;
	}
	//printf("xCurrent1 %d %f\n", num, xCurrent );
	for(;j < n; j++)
	{
		//printf("xj: %f\n", x[j] );
		if (j != num)
			xCurrent *= (x1 - x[j]) / ( x[num] - x[j]);
	}
	//printf("xCurrent2 %d %f\n", num, xCurrent );
	return xCurrent;
}
