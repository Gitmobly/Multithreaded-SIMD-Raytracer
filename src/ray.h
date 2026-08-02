#ifndef RAY_H
#define RAY_H

#include "types.h"

typedef struct ray_t
{
	point3_t origin;
	point3_t direction;
} ray_t;

// P(t) = A + tb
internal point3_t ray_point_at(ray_t* r, f64 t)
{
	return r->origin + t * r->direction;
}

#endif
