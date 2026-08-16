#ifndef CAMERA_H
#define CAMERA_H

#include "color.h"
#include "material.h"
#include "object.h"
#include "ray.h"
#include "types.h"

#include <pthread.h>
#include <unistd.h>

#define DEFAULT_BUF_SIZE		  256				 // Arbitrary size of buffer for string manipulation
#define DEFAULT_ASPECT_RATIO	  (16.0f / 9.0f)	 // Ratio of image with over height
#define DEFAULT_IMG_W			  800				 // Rendered image width in pixels
#define DEFAULT_SAMPLES_PER_PIXEL 800				 // Count of random samples for each pixel
#define DEFAULT_MAX_DEPTH		  40				 // Maximum number of ray bounces into scene
#define DEFAULT_VERTICAL_FOV	  20				 // Vertical field of view
#define DEFAULT_LOOKFROM		  point3_t{13, 2, 3} // Point camera is looking from
#define DEFAULT_LOOKAT			  point3_t{0, 0, 0}	 // Point camera is looking at
#define DEFAULT_VUP				  vec3_t{0, 1, 0}	 // Camera-relative "up" direction
#define DEFAULT_DEFOCUS_ANGLE	  0.6f				 // Variation angle of rays through each pixel
#define DEFAULT_FOCUS_DIST		  10.0f				 // Distance from camera lookfrom to plane of perfect focus

#define RENDER_SEED 0x0ull

typedef struct camera_t
{
	point3_t center;
	point3_t pixel00_loc;
	vec3_t	 pixel_delta_u;
	vec3_t	 pixel_delta_v;
	vec3_t	 u, v, w;
	vec3_t	 defocus_disk_u;
	vec3_t	 defocus_disk_v;
	f32		 pixel_samples_scale;
	i32		 img_h;
	i32		 max_depth;
} camera_t;

internal vec3_t sample_square(pcg32_random_t* rng)
{
	return {random_f32(rng) - 0.5f, random_f32(rng) - 0.5f, 0.0f};
}

internal point3_t defocus_disk_sample(camera_t* cam, pcg32_random_t* rng)
{
	vec3_t p = vec3_random_in_unit_disk(rng);
	return cam->center + (p.x * cam->defocus_disk_u) + (p.y * cam->defocus_disk_v);
}

internal ray_t get_ray(camera_t* cam, i32 i, i32 j, pcg32_random_t* rng)
{
	vec3_t offset		= sample_square(rng);
	vec3_t pixel_sample = cam->pixel00_loc								//
						  + (((f32)i + offset.x) * cam->pixel_delta_u)	//
						  + (((f32)j + offset.y) * cam->pixel_delta_v); //

	point3_t ray_origin	   = (DEFAULT_DEFOCUS_ANGLE <= 0) ? cam->center : defocus_disk_sample(cam, rng);
	vec3_t	 ray_direction = pixel_sample - ray_origin;

	return {ray_origin, ray_direction};
}

internal void camera_initialize(camera_t* camera)
{
	ASSERT(camera && "Trying to initialize a NULL camera");

	i32 img_h = (i32)(DEFAULT_IMG_W / DEFAULT_ASPECT_RATIO);
	img_h	  = (img_h < 1) ? 1 : img_h;

	f32		 theta		   = degrees_to_radians(DEFAULT_VERTICAL_FOV);
	f32		 h			   = tanf(theta / 2);
	f32		 viewport_h	   = 2.0f * h * DEFAULT_FOCUS_DIST;
	f32		 viewport_w	   = viewport_h * ((f32)DEFAULT_IMG_W / (f32)img_h);
	point3_t camera_center = DEFAULT_LOOKFROM;

	camera->w = vec3_unit_vector(DEFAULT_LOOKFROM - DEFAULT_LOOKAT);
	camera->u = vec3_unit_vector(vec3_cross(DEFAULT_VUP, camera->w));
	camera->v = vec3_cross(camera->w, camera->u);

	vec3_t viewport_u = viewport_w * camera->u;
	vec3_t viewport_v = viewport_h * -camera->v;

	vec3_t pixel_delta_u = viewport_u / (f32)DEFAULT_IMG_W;
	vec3_t pixel_delta_v = viewport_v / (f32)img_h;

	point3_t viewport_upper_left =
		camera_center - (DEFAULT_FOCUS_DIST * camera->w) - viewport_u / 2.0f - viewport_v / 2.0f;
	point3_t pixel00_loc = viewport_upper_left + 0.5f * (pixel_delta_u + pixel_delta_v);

	f32 defocus_radius	   = DEFAULT_FOCUS_DIST * tanf(degrees_to_radians((f32)DEFAULT_DEFOCUS_ANGLE / 2.0f));
	camera->defocus_disk_u = camera->u * defocus_radius;
	camera->defocus_disk_v = camera->v * defocus_radius;

	camera->img_h				= img_h;
	camera->center				= DEFAULT_LOOKFROM;
	camera->pixel00_loc			= pixel00_loc;
	camera->pixel_delta_u		= pixel_delta_u;
	camera->pixel_delta_v		= pixel_delta_v;
	camera->pixel_samples_scale = 1.0f / DEFAULT_SAMPLES_PER_PIXEL;
	camera->max_depth			= DEFAULT_MAX_DEPTH;
}

internal color ray_color(ray_t* r, i32 depth, objects_t* world, pcg32_random_t* rng)
{
	if (depth <= 0)
	{
		return {0, 0, 0};
	}

	hit_record_t rec;

	if (evaluate_all_object_hits(world, r, interval_t{0.001f, INFINITY}, &rec))
	{
		ray_t scattered;
		color attenuation;

		if (scatter(rec.mat, r, &rec, &attenuation, &scattered, rng))
		{
			return (attenuation * ray_color(&scattered, depth - 1, world, rng));
		}

		return {0, 0, 0};
	}

	vec3_t unit_direction = vec3_unit_vector(r->direction);
	f32	   a			  = 0.5f * (unit_direction.y + 1.0f);
	return (1.0f - a) * color{1.0f, 1.0f, 1.0f} + a * color{0.5f, 0.7f, 1.0f};
}

typedef struct render_work_t
{
	camera_t*  camera;
	objects_t* world;
	color*	   fb;
	i32		   width;
	i32		   height;
	i32*	   next_row;
} render_work_t;

internal void* render_worker(void* r)
{
	render_work_t* work = (render_work_t*)r;
	pcg32_random_t rng;

	i32 j;
	while ((j = __atomic_fetch_add(work->next_row, 1, __ATOMIC_RELAXED)) < work->height)
	{
		pcg32_srandom_r(&rng, RENDER_SEED, (u64)j);
		for (i32 i = 0; i < work->width; ++i)
		{
			color pixel_color = {0, 0, 0};
			for (i32 sample = 0; sample < DEFAULT_SAMPLES_PER_PIXEL; ++sample)
			{
				ray_t r = get_ray(work->camera, i, j, &rng);
				pixel_color += ray_color(&r, work->camera->max_depth, work->world, &rng);
			}

			work->fb[(size_t)(j * work->width + i)] = pixel_color * work->camera->pixel_samples_scale;
		}
	}

	return NULL;
}

internal void render(camera_t* camera, objects_t* world)
{
	const i32 w = DEFAULT_IMG_W;
	const i32 h = camera->img_h;

	color* fb = (color*)malloc(size_t(w * h) * sizeof(color));
	ASSERT(fb && "Failed to allocate a framebuffer.");

	i32 thread_count = (i32)sysconf(_SC_NPROCESSORS_ONLN);
	if (thread_count < 1)
	{
		thread_count = 1;
	}

	i32 next_row = 0;

	render_work_t work = {
		.camera	  = camera,
		.world	  = world,
		.fb		  = fb,
		.width	  = w,
		.height	  = h,
		.next_row = &next_row,
	};

	pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * (size_t)thread_count);

	for (i32 t = 0; t < thread_count; ++t)
	{
		pthread_create(&threads[t], NULL, render_worker, &work);
	}

	for (i32 t = 0; t < thread_count; ++t)
	{
		pthread_join(threads[t], NULL);
	}

	free(threads);

	FILE* img_file;
	img_file = fopen("image.ppm", "wb");
	ASSERT(img_file && "Could not write to image.ppm.");

	char buf[DEFAULT_BUF_SIZE];
	i32	 header_length = snprintf(buf, DEFAULT_BUF_SIZE, "P3\n%d %d\n255\n", DEFAULT_IMG_W, camera->img_h);
	fwrite(buf, sizeof(u8), (size_t)header_length, img_file);

	for (i32 idx = 0; idx < w * h; ++idx)
	{
		vec3i byterange = byterange_from_color(fb[idx]);
		i32	  data_length =
			snprintf(buf, DEFAULT_BUF_SIZE * sizeof(char), "%d %d %d\n", byterange[0], byterange[1], byterange[2]);
		fwrite(buf, sizeof(u8), (size_t)data_length, img_file);
	}

	fclose(img_file);
	free(fb);
}

#endif
