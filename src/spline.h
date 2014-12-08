#ifndef SPLINE_H_
#define SPLINE_H_
// Includes
#include <math.h>
#include <stddef.h>
// Defines

// Data types
typedef struct {
	double_t a, b, c, d, x;
} segment_t;

typedef struct {
	size_t n;
	segment_t *segment;
} spline_t;

// Public functions
void spline_init(spline_t *pspline,
					const double_t *px,
					const double_t *py,
					size_t n);

void spline_destroy(spline_t *pspline);

double_t spline_getvalue(spline_t *pspline, double_t x);

int spline_getvaluev(spline_t *pspline, double_t *px, double_t *py, size_t n);

#endif /* SPLINE_H_ */
