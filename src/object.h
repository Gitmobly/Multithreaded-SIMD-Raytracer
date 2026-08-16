#ifndef OBJECT_H
#define OBJECT_H

#include "hittable.h"
#include "interval.h"
#include "ray.h"
#include "types.h"
#include <cstdio>
#include <immintrin.h>

#define MAX_WORLD_OBJECTS 1024

typedef enum object_shape_t : u8
{
	NO_SHAPE,
	SPHERE,
} object_shape_t;

typedef struct object_t
{
	point3_t	   center;
	f32			   radius;
	material_t*	   mat;
	object_shape_t shape;
} object_t;

typedef struct objects_t
{
	f32			   center_x[MAX_WORLD_OBJECTS];
	f32			   center_y[MAX_WORLD_OBJECTS];
	f32			   center_z[MAX_WORLD_OBJECTS];
	f32			   radius[MAX_WORLD_OBJECTS]; // For spheres
	material_t*	   mat[MAX_WORLD_OBJECTS];
	object_shape_t shape[MAX_WORLD_OBJECTS];
	i32			   count;
} objects_t;

internal void hit_block(objects_t* o, i32 i, __m256 ray_ox, __m256 ray_oy, __m256 ray_oz, __m256 ray_dx, __m256 ray_dy,
						__m256 ray_dz, __m256 inv_a, __m256 a, __m256 ray_i_min, __m256 ray_i_max, __m256* best_t,
						__m256i* best_idx)
{
	__m256 center_x = _mm256_loadu_ps(&o->center_x[i]);
	__m256 center_y = _mm256_loadu_ps(&o->center_y[i]);
	__m256 center_z = _mm256_loadu_ps(&o->center_z[i]);
	__m256 radius	= _mm256_loadu_ps(&o->radius[i]);

	__m256 oc_x = _mm256_sub_ps(center_x, ray_ox);
	__m256 oc_y = _mm256_sub_ps(center_y, ray_oy);
	__m256 oc_z = _mm256_sub_ps(center_z, ray_oz);

	__m256 h = _mm256_fmadd_ps(ray_dx, oc_x, _mm256_fmadd_ps(ray_dy, oc_y, _mm256_mul_ps(ray_dz, oc_z)));

	__m256 oc_l2 = _mm256_fmadd_ps(oc_x, oc_x, _mm256_fmadd_ps(oc_y, oc_y, _mm256_mul_ps(oc_z, oc_z)));
	__m256 r2	 = _mm256_mul_ps(radius, radius);
	__m256 c	 = _mm256_sub_ps(oc_l2, r2);

	__m256 discriminant = _mm256_sub_ps(_mm256_mul_ps(h, h), _mm256_mul_ps(a, c));
	__m256 hit_mask		= _mm256_cmp_ps(discriminant, _mm256_setzero_ps(), _CMP_GE_OQ);
	__m256 sqrtd		= _mm256_sqrt_ps(discriminant);

	__m256 near_root = _mm256_mul_ps(_mm256_sub_ps(h, sqrtd), inv_a);
	__m256 far_root	 = _mm256_mul_ps(_mm256_add_ps(h, sqrtd), inv_a);

	__m256 near_combined_mask =
		_mm256_and_ps(_mm256_cmp_ps(near_root, ray_i_min, _CMP_GT_OQ), _mm256_cmp_ps(ray_i_max, near_root, _CMP_GT_OQ));
	__m256 far_combined_mask =
		_mm256_and_ps(_mm256_cmp_ps(ray_i_max, far_root, _CMP_GT_OQ), _mm256_cmp_ps(far_root, ray_i_min, _CMP_GT_OQ));

	__m256 nearest_root = _mm256_blendv_ps(far_root, near_root, near_combined_mask);
	__m256 valid		= _mm256_and_ps(hit_mask, _mm256_or_ps(near_combined_mask, far_combined_mask));
	__m256 t			= _mm256_blendv_ps(_mm256_set1_ps(INFINITY), nearest_root, valid);

	__m256i lane_idx = _mm256_add_epi32(_mm256_set1_epi32(i), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
	__m256	closest	 = _mm256_cmp_ps(t, *best_t, _CMP_LT_OQ);
	*best_t			 = _mm256_min_ps(*best_t, t);
	*best_idx =
		_mm256_castps_si256(_mm256_blendv_ps(_mm256_castsi256_ps(*best_idx), _mm256_castsi256_ps(lane_idx), closest));
}

internal b32 evaluate_all_object_hits(objects_t* o, ray_t* r, interval_t ray_t, hit_record_t* rec)
{
	__m256 ray_ox = _mm256_set1_ps(r->origin.x);
	__m256 ray_oy = _mm256_set1_ps(r->origin.y);
	__m256 ray_oz = _mm256_set1_ps(r->origin.z);
	__m256 ray_dx = _mm256_set1_ps(r->direction.x);
	__m256 ray_dy = _mm256_set1_ps(r->direction.y);
	__m256 ray_dz = _mm256_set1_ps(r->direction.z);
	f32	   a_sc	  = vec3_length_squared(r->direction);

	__m256 a	 = _mm256_set1_ps(a_sc);
	__m256 inv_a = _mm256_set1_ps(1.0f / a_sc);

	__m256 ray_i_min = _mm256_set1_ps(ray_t.min);
	__m256 ray_i_max = _mm256_set1_ps(ray_t.max);

	__m256	best_t	 = _mm256_set1_ps(INFINITY);
	__m256i best_idx = _mm256_set1_epi32(-1);

	for (i32 i = 0; i < o->count; i += 8)
	{
		hit_block(o, i, ray_ox, ray_oy, ray_oz, ray_dx, ray_dy, ray_dz, inv_a, a, ray_i_min, ray_i_max, &best_t,
				  &best_idx);
	}

	f32 t_arr[8];
	i32 idx_arr[8];
	_mm256_storeu_ps(t_arr, best_t);
	_mm256_storeu_si256((__m256i*)idx_arr, best_idx);

	f32 winner_t   = INFINITY;
	i32 winner_idx = -1;
	for (i32 k = 0; k < 8; ++k)
	{
		if (t_arr[k] < winner_t)
		{
			winner_t   = t_arr[k];
			winner_idx = idx_arr[k];
		}
	}

	if (winner_idx < 0)
	{
		return false;
	}

	vec3_t center		  = {o->center_x[winner_idx], o->center_y[winner_idx], o->center_z[winner_idx]};
	rec->t				  = winner_t;
	rec->p				  = ray_point_at(r, rec->t);
	vec3_t outward_normal = (rec->p - center) / o->radius[winner_idx];
	hit_record_set_face_normal(rec, r, outward_normal);
	rec->mat = o->mat[winner_idx];

	return true;
}

internal b32 object_group_add(objects_t* g, object_t o)
{
	if (g->count >= MAX_WORLD_OBJECTS - 1)
	{
		printf("Tried to add object to group, but it is full.\n");
		return false;
	}

	i32 i		   = g->count++;
	g->center_x[i] = o.center.x;
	g->center_y[i] = o.center.y;
	g->center_z[i] = o.center.z;
	g->radius[i]   = o.radius;
	g->mat[i]	   = o.mat;
	g->shape[i]	   = o.shape;

	return true;
}

#endif
