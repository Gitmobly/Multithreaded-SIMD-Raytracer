#ifndef MATERIAL_H
#define MATERIAL_H

#include "color.h"
#include "hittable.h"
#include "ray.h"
#include "types.h"
#include <cmath>
#include <cstddef>

typedef enum material_type_t : u8
{
	NO_MATERIAL,
	METAL,
	LAMBERTIAN,
	DIELECTRIC,
} material_type_t;

typedef struct material_t
{
	color			albedo;
	f32				fuzz;
	f32				refraction_index;
	material_type_t type;
} material_t;

internal f32 reflectance(f32 cosine, f32 refraction_index)
{
	f32 r0 = (1 - refraction_index) / (1 + refraction_index);
	r0	   = r0 * r0;
	return r0 + (1 - r0) * pow5((1 - cosine));
}

internal b32 scatter(material_t* mat, ray_t* r_in, hit_record_t* rec, color* attenuation, ray_t* scattered)
{
	switch (mat->type)
	{
		case LAMBERTIAN:
		{
			vec3_t scatter_direction = rec->normal + vec3_random_unit_vector();

			if (vec3_near_zero(scatter_direction))
			{
				scatter_direction = rec->normal;
			}

			*scattered	 = {rec->p, scatter_direction};
			*attenuation = mat->albedo;
			return true;
		}

		case METAL:
		{
			vec3_t reflected = vec3_reflect(r_in->direction, rec->normal);
			reflected		 = vec3_unit_vector(reflected) + (mat->fuzz * vec3_random_unit_vector());
			*scattered		 = {rec->p, reflected};
			*attenuation	 = mat->albedo;
			return (vec3_dot(scattered->direction, rec->normal) > 0);
		}

		case DIELECTRIC:
		{
			*attenuation = color{1, 1, 1};
			f32 ri		 = rec->front_face ? (1.0f / mat->refraction_index) : mat->refraction_index;

			vec3_t unit_direction = vec3_unit_vector(r_in->direction);
			f32	   cos_theta	  = fminf(vec3_dot(-unit_direction, rec->normal), 1.0f);
			f32	   sin_theta	  = sqrtf(1.0f - cos_theta * cos_theta);

			b32	   cannot_refract = ri * sin_theta > 1.0f;
			vec3_t direction;

			if (cannot_refract || reflectance(cos_theta, ri) > random_f32())
			{
				direction = vec3_reflect(unit_direction, rec->normal);
			}
			else
			{
				direction = vec3_refract(unit_direction, rec->normal, ri);
			}

			*scattered = {rec->p, direction};
			return true;
		}

		default:
		{
			ASSERT(false && "Hit check for unknown reflectance requested.\n");
			return false;
		}
	}
	return false;
}

#endif
