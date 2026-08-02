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
#define PI 3.14159265358979323846
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

inline f64 degrees_to_radians(f64 degrees)
{
	return degrees * PI / 180.0;
}

inline f64 random_f64()
{
	return (rand() / (RAND_MAX + 1.0));
}

inline f64 random_f64(f64 min, f64 max)
{
	return min + (max - min) * random_f64();
}

/* -------------------------------------
 * (JJB): String
 */

typedef struct str8_t
{
	const char* string;
	u64			length;
} str8_t;

internal str8_t str8_create(const char* string, u64 length)
{
	return {.string = string, .length = length};
}

/* -------------------------------------
 * (JJB): Math Types
 */

typedef struct vec3_t
{
	union
	{
		f64 v[3];
		struct
		{
			f64 x, y, z;
		};
	};

	constexpr f64& operator[](i32 i)
	{
		return v[i];
	}

	constexpr f64 operator[](i32 i) const
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

constexpr vec3_t operator*(vec3_t a, f64 b)
{
	return vec3_t{b * a.x, b * a.y, b * a.z};
}

constexpr vec3_t operator*(vec3_t a, vec3_t b)
{
	return {a.x * b.x, a.y * b.y, a.z * b.z};
}

constexpr vec3_t operator*(f64 a, vec3_t b)
{
	return b * a;
}

constexpr vec3_t operator/(vec3_t a, f64 b)
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

internal f64 vec3_length_squared(vec3_t a)
{
	return a.x * a.x + a.y * a.y + a.z * a.z;
}

internal b32 vec3_near_zero(vec3_t a)
{
	f64 s = 1e-8;
	return (fabs(a.x) < s) && (fabs(a.y) < s) && (fabs(a.z) < s);
}

internal f64 vec3_length(vec3_t a)
{
	return sqrt(vec3_length_squared(a));
}

internal vec3_t vec3_random()
{
	return {random_f64(), random_f64(), random_f64()};
}

internal vec3_t vec3_random(f64 min, f64 max)
{
	return {random_f64(min, max), random_f64(min, max), random_f64(min, max)};
}

internal f64 vec3_dot(vec3_t a, vec3_t b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
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

internal vec3_t vec3_random_in_unit_disk()
{
	while (true)
	{
		vec3_t p = {random_f64(-1, 1), random_f64(-1, 1), 0};
		if (vec3_length_squared(p) < 1)
		{
			return p;
		}
	}
}

internal vec3_t vec3_random_unit_vector()
{
	while (true)
	{
		vec3_t p	 = vec3_random(-1, 1);
		f64	   lensq = vec3_length_squared(p);
		if (1e-160 < lensq && lensq <= 1)
		{
			return p / sqrt(lensq);
		}
	}
}

internal vec3_t vec3_random_on_hemisphere(vec3_t normal)
{
	vec3_t on_unit_sphere = vec3_random_unit_vector();
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

internal vec3_t vec3_refract(vec3_t uv, vec3_t n, f64 etai_over_etat)
{
	f64	   cos_theta	  = fmin(vec3_dot(vec3_t{0, 0, 0} - uv, n), 1.0);
	vec3_t r_out_perp	  = etai_over_etat * (uv + cos_theta * n);
	vec3_t r_out_parallel = -sqrt(fabs(1.0 - vec3_length_squared(r_out_perp))) * n;
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
