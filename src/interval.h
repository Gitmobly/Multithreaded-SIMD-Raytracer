#ifndef INTERVAL_H
#define INTERVAL_H

#include "types.h"

typedef struct interval_t
{
	f64 min;
	f64 max;
} interval_t;

internal f64 interval_size(interval_t i)
{
	return i.max - i.min;
}

internal b32 interval_contains(interval_t i, f64 x)
{
	return i.min <= x && x <= i.max;
}

internal b32 interval_surrounds(interval_t i, f64 x)
{
	return i.min < x && x < i.max;
}

internal f64 interval_clamp(interval_t i, f64 x)
{
	if (x < i.min)
	{
		return i.min;
	}

	if (x > i.max)
	{
		return i.max;
	}

	return x;
}

const global_variable interval_t interval_empty	   = {+INFINITY, -INFINITY};
const global_variable interval_t interval_universe = {-INFINITY, +INFINITY};

#endif
