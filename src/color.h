#ifndef COLOR_H
#define COLOR_H

#include "interval.h"
#include "types.h"

#define color vec3_t

inline internal f64 linear_to_gamma(f64 linear_component)
{
	if (linear_component > 0)
	{
		return sqrt(linear_component);
	}

	return 0;
}

internal vec3i byterange_from_color(color color)
{
	local_persist const interval_t intensity = {0.000, 0.999};

	f64 r = linear_to_gamma(color.x);
	f64 g = linear_to_gamma(color.y);
	f64 b = linear_to_gamma(color.z);

	i32 rbyte = (255.999 * interval_clamp(intensity, r));
	i32 gbyte = (255.999 * interval_clamp(intensity, g));
	i32 bbyte = (255.999 * interval_clamp(intensity, b));

	return {rbyte, gbyte, bbyte};
}

#endif
