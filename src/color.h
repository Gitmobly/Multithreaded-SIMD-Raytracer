#ifndef COLOR_H
#define COLOR_H

#include "interval.h"
#include "types.h"

#define color vec3_t

inline internal f32 linear_to_gamma(f32 linear_component)
{
	if (linear_component > 0)
	{
		return sqrtf(linear_component);
	}

	return 0;
}

internal vec3i byterange_from_color(color color)
{
	local_persist const interval_t intensity = {0.000f, 0.999f};

	f32 r = linear_to_gamma(color.x);
	f32 g = linear_to_gamma(color.y);
	f32 b = linear_to_gamma(color.z);

	i32 rbyte = (i32)(255.999f * interval_clamp(intensity, r));
	i32 gbyte = (i32)(255.999f * interval_clamp(intensity, g));
	i32 bbyte = (i32)(255.999f * interval_clamp(intensity, b));

	return {rbyte, gbyte, bbyte};
}

#endif
