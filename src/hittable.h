#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"
#include "types.h"

typedef struct material_t material_t;

typedef struct hit_record_t
{
	point3_t	p;
	vec3_t		normal;
	material_t* mat;
	f32			t;
	b32			front_face;
} hit_record_t;

internal void hit_record_set_face_normal(hit_record_t* hr, ray_t* r, vec3_t outward_normal)
{
	hr->front_face = vec3_dot(r->direction, outward_normal) < 0;
	hr->normal	   = hr->front_face ? outward_normal : -outward_normal;
}

#endif
