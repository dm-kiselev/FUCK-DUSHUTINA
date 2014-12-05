#include "spline.h"
#include <malloc.h>
// описание алгоритма приведено в отчёте, пункт 1.
void spline_init(spline_t *spline, const double_t *x,
					const double_t *y, size_t nb) {
	if(spline->n != 0) spline_destroy(spline);

	spline->n = nb;
	spline->segment = malloc(sizeof(segment_t) * spline->n);

	size_t i = 0;
	while(i < spline->n) {
		spline->segment[i].x = x[i];
		// формула 6
		spline->segment[i].a = y[i];
		++i;
	}
	spline->segment[0].c = 0.;

	// Метод прогонки: прямой ход
	double_t *alpha = malloc(sizeof(double_t) * spline->n);
	double_t *beta = malloc(sizeof(double_t) * spline->n);
	double_t A, B, C, F, h_i, h_i1, z;
	alpha[0] = beta[0] = 0.;

	// Вычисление коэффициента c по формуле 7
	i = 1;
	while(i < spline->n-1) {
		h_i = x[i] - x[i-1];
		h_i1 = x[i+1] - x[i];
		A = h_i;
		C = 2. * (h_i + h_i1);
		B = h_i1;
		F = 6. * ((y[i+1] - y[i])/h_i1 - (y[i] - y[i-1])/h_i);
		z = (A * alpha[i-1] + C);
		alpha[i] = -B / z;
		beta[i] = (F - A * beta[i-1]) / z;
		++i;
	}

	spline->segment[spline->n - 1].c = (F - A * beta[spline->n-2])
										/ (C + A * alpha[spline->n-2]);

	// Обратный ход метода прогонки
	i = spline->n-2;
	while(i > 0) {
		spline->segment[i].c = alpha[i] * spline->segment[i+1].c + beta[i];
		--i;
	}

	free(beta);
	free(alpha);

	i = spline->n-1;
	while(i > 0) {
		h_i = x[i] - x[i-1];
		// Формула 8
		spline->segment[i].d = (spline->segment[i].c
								- spline->segment[i-1].c) / h_i;
		// Формула 9
		spline->segment[i].b = h_i * (2. * spline->segment[i].c
								+ spline->segment[i-1].c) / 6.
								+ (y[i] - y[i-1]) / h_i;
		--i;
	}
}

void spline_destroy(spline_t *spline) {
	if(spline->segment != NULL) {
		free(spline->segment);
		spline->segment = NULL;
	}
	spline->n = 0;
}

double_t spline_getvalue(spline_t *spline, double_t x) {
	if(spline == NULL) {
		return -1;
	}
	segment_t *ps;
	if (x <= spline->segment[0].x)
		ps = &spline->segment[0];
	else if (x >= spline->segment[spline->n-1].x) {
		ps = &spline->segment[spline->n-1];
	} else {
		size_t i = 0, j = spline->n-1;
		while (i+1 < j) {
			size_t k = i + (j-i)/2;
			if (x <= spline->segment[k].x)
				j = k;
			else
				i = k;
		}
		ps = &spline->segment[j];
	}

	double_t dx = (x - ps->x);
	return ps->a + (ps->b + (ps->c/2. + ps->d*dx/6.) * dx) * dx;
}

int spline_getvaluev(spline_t *pspline, double_t *px, double_t *py, size_t n) {
	for(size_t i=0; i < n; ++i) {
		py[i] = spline_getvalue(pspline, px[i]);
	}
	return 0;
}
