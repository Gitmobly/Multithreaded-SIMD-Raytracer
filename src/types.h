#ifndef TYPES_H
#define TYPES_H

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <random>

/* -------------------------------------
 * (JJB): Simple Aliases & Macros
 */

#define ARRAYCOUNT(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

#ifndef PI
#define PI 3.14159265358979323846f
#endif

#define BYTES(size)		((size_t)(size))
#define KILOBYTES(size) ((size_t)(size) * 1024ULL)
#define MEGABYTES(size) ((size_t)(size) * 1024ULL * 1024ULL)
#define GIGABYTES(size) ((size_t)(size) * 1024ULL * 1024ULL * 1024ULL)

#ifdef DEBUG_BUILD
#define ASSERT(expr)                                                                                                   \
	if (!(expr))                                                                                                       \
	{                                                                                                                  \
		*(volatile int*)0 = 0;                                                                                         \
	}
#else
#define ASSERT(expr) {};
#endif

#define global_variable static
#define internal		static
#define local_persist	static

typedef uint8_t	 u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t	i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint32_t b32;

typedef float  f32;
typedef double f64;

inline f32 degrees_to_radians(f32 degrees)
{
	return degrees * PI / 180.0f;
}

struct pcg_state_setseq_64
{
	u64 state;
	u64 inc;
};
typedef struct pcg_state_setseq_64 pcg32_random_t;

#define PCG32_INITIALIZER {0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL}

inline u32 pcg32_random_r(pcg32_random_t* rng)
{
	u64 oldstate   = rng->state;
	rng->state	   = oldstate * 6364136223846793005ULL + rng->inc;
	u32 xorshifted = (u32)(((oldstate >> 18u) ^ oldstate) >> 27u);
	u32 rot		   = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

inline void pcg32_srandom_r(pcg32_random_t* rng, uint64_t initstate, uint64_t initseq)
{
	rng->state = 0U;
	rng->inc   = (initseq << 1u) | 1u;
	pcg32_random_r(rng);
	rng->state += initstate;
	pcg32_random_r(rng);
}

inline f32 random_f32(pcg32_random_t* rng)
{
	const u32 max = 1 << 24;
	u32		  r	  = pcg32_random_r(rng);
	r &= max - 1;
	return (f32)(r) * 1.0f / max;
}

inline f32 random_f32(f32 min, f32 max, pcg32_random_t* rng)
{
	return min + (max - min) * random_f32(rng);
}

inline f32 pow5(f32 x)
{
	f32 x2 = x * x;
	return x2 * x2 * x;
}

/* -------------------------------------
 * (JJB): Math Types
 */

typedef struct vec3_t
{
	union
	{
		f32 v[3];
		struct
		{
			f32 x, y, z;
		};
		struct
		{
			f32 r, g, b;
		};
	};

	constexpr f32& operator[](i32 i)
	{
		return v[i];
	}

	constexpr f32 operator[](i32 i) const
	{
		return v[i];
	}
} vec3_t;

#define point3_t vec3_t

constexpr vec3_t operator-(vec3_t a, vec3_t b)
{
	return vec3_t{a.x - b.x, a.y - b.y, a.z - b.z};
}

constexpr vec3_t operator-(vec3_t a)
{
	return vec3_t{-a.x, -a.y, -a.z};
}

constexpr vec3_t operator+(vec3_t a, vec3_t b)
{
	return vec3_t{a.x + b.x, a.y + b.y, a.z + b.z};
}

constexpr vec3_t operator*(vec3_t a, f32 b)
{
	return vec3_t{b * a.x, b * a.y, b * a.z};
}

constexpr vec3_t operator*(vec3_t a, vec3_t b)
{
	return {a.x * b.x, a.y * b.y, a.z * b.z};
}

constexpr vec3_t operator*(f32 a, vec3_t b)
{
	return b * a;
}

constexpr vec3_t operator/(vec3_t a, f32 b)
{
	return (1 / b) * a;
}

constexpr vec3_t& operator+=(vec3_t& a, vec3_t b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

constexpr vec3_t& operator*=(vec3_t& a, vec3_t b)
{
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	return a;
}

constexpr vec3_t& operator/=(vec3_t& a, vec3_t b)
{
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	return a;
}

internal f32 vec3_length_squared(vec3_t a)
{
	return a.x * a.x + a.y * a.y + a.z * a.z;
}

internal b32 vec3_near_zero(vec3_t a)
{
	f32 s = 1e-8f;
	return (fabsf(a.x) < s) && (fabsf(a.y) < s) && (fabsf(a.z) < s);
}

internal f32 vec3_length(vec3_t a)
{
	return sqrtf(vec3_length_squared(a));
}

internal vec3_t vec3_random(pcg32_random_t* rng)
{
	return {random_f32(rng), random_f32(rng), random_f32(rng)};
}

internal vec3_t vec3_random(f32 min, f32 max, pcg32_random_t* rng)
{
	return {random_f32(min, max, rng), random_f32(min, max, rng), random_f32(min, max, rng)};
}

internal f32 vec3_dot(vec3_t a, vec3_t b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

internal vec3_t vec3_cross(vec3_t a, vec3_t b)
{
	return vec3_t{
		a[1] * b[2] - a[2] * b[1],
		a[2] * b[0] - a[0] * b[2],
		a[0] * b[1] - a[1] * b[0],
	};
}

internal vec3_t vec3_unit_vector(vec3_t a)
{
	return a / vec3_length(a);
}

internal vec3_t vec3_random_in_unit_disk(pcg32_random_t* rng)
{
	while (true)
	{
		vec3_t p = {random_f32(-1, 1, rng), random_f32(-1, 1, rng), 0};
		if (vec3_length_squared(p) < 1)
		{
			return p;
		}
	}
}

internal vec3_t vec3_random_unit_vector(pcg32_random_t* rng)
{
	while (true)
	{
		vec3_t p	 = vec3_random(-1, 1, rng);
		f32	   lensq = vec3_length_squared(p);
		if (1e-160 < lensq && lensq <= 1)
		{
			return p / sqrtf(lensq);
		}
	}
}

internal vec3_t vec3_random_on_hemisphere(vec3_t normal, pcg32_random_t* rng)
{
	vec3_t on_unit_sphere = vec3_random_unit_vector(rng);
	if (vec3_dot(on_unit_sphere, normal) > 0.0)
	{
		return on_unit_sphere;
	}
	else
	{
		return vec3_t{0, 0, 0} - on_unit_sphere;
	}
}

internal vec3_t vec3_reflect(vec3_t v, vec3_t n)
{
	return v - 2 * vec3_dot(v, n) * n;
}

internal vec3_t vec3_refract(vec3_t uv, vec3_t n, f32 etai_over_etat)
{
	f32	   cos_theta	  = fminf(vec3_dot(vec3_t{0, 0, 0} - uv, n), 1.0);
	vec3_t r_out_perp	  = etai_over_etat * (uv + cos_theta * n);
	vec3_t r_out_parallel = -sqrtf(fabsf(1.0f - vec3_length_squared(r_out_perp))) * n;
	return r_out_perp + r_out_parallel;
}

typedef struct vec3i_t
{
	union
	{
		i32 vi[3];
		struct
		{
			i32 x, y, z;
		};
	};

	constexpr i32& operator[](i32 i)
	{
		return vi[i];
	}

	constexpr i32 operator[](i32 i) const
	{
		return vi[i];
	}
} vec3i;

#endif
