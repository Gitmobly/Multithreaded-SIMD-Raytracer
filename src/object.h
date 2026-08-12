#ifndef OBJECT_H
#define OBJECT_H

#include "hittable.h"
#include "interval.h"
#include "ray.h"
#include "types.h"
#include <cstdio>

#define MAX_WORLD_OBJECTS 1024

typedef enum object_shape_t : u8
{
	NO_SHAPE,
	SPHERE,
} object_shape_t;

typedef struct object_t
{
	point3_t	   center;
	f64			   radius; // For spheres
	material_t*	   mat;
	object_shape_t shape;
} object_t;

typedef struct object_group_t
{
	object_t objects[MAX_WORLD_OBJECTS];
	i32		 count;
} object_group_t;

internal b32 hit(object_t* object, ray_t* r, interval_t ray_t, hit_record_t* rec)
{
	switch (object->shape)
	{
		case SPHERE:
		{
			vec3_t oc			= object->center - r->origin;
			f64	   a			= vec3_length_squared(r->direction);
			f64	   h			= vec3_dot(r->direction, oc);
			f64	   c			= vec3_length_squared(oc) - object->radius * object->radius;
			f64	   discriminant = h * h - a * c;

			if (discriminant < 0)
			{
				return false;
			}

			f64 sqrtd = sqrt(discriminant);

			f64 root = (h - sqrtd) / a;
			if (!interval_surrounds(ray_t, root))
			{
				root = (h + sqrtd) / a;
				if (!interval_surrounds(ray_t, root))
				{
					return false;
				}
			}

			rec->t				  = root;
			rec->p				  = ray_point_at(r, rec->t);
			vec3_t outward_normal = (rec->p - object->center) / object->radius;
			hit_record_set_face_normal(rec, r, outward_normal);
			rec->mat = object->mat;

			return true;
		}
		break;

		default:
		{
			ASSERT(false && "Hit check for unknown shape requested.\n");
			return false;
		}
	}
}

internal b32 evaluate_all_object_hits(object_group_t* o, ray_t* r, interval_t ray_t, hit_record_t* rec)
{
	hit_record_t temp_rec;
	b32			 hit_anything	= false;
	f64			 closest_so_far = ray_t.max;

	for (i32 i = 0; i < o->count; ++i)
	{
		if (hit(&o->objects[i], r, interval_t{ray_t.min, closest_so_far}, &temp_rec))
		{
			hit_anything   = true;
			closest_so_far = temp_rec.t;
			*rec		   = temp_rec;
		}
	}

	return hit_anything;
}

internal b32 object_group_add(object_group_t* g, object_t o)
{
	if (g->count >= MAX_WORLD_OBJECTS - 1)
	{
		printf("Tried to add object to group, but it is full.\n");
		return false;
	}

	g->objects[g->count++] = o;
	return true;
}

#endif
