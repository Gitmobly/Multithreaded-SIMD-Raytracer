#ifndef CAMERA_H
#define CAMERA_H

#include "color.h"
#include "material.h"
#include "object.h"
#include "ray.h"
#include "types.h"

#define DEFAULT_BUF_SIZE		  256				 // Arbitrary size of buffer for string manipulation
#define DEFAULT_ASPECT_RATIO	  (16.0 / 9.0)		 // Ratio of image with over height
#define DEFAULT_IMG_W			  1200				 // Rendered image width in pixels
#define DEFAULT_SAMPLES_PER_PIXEL 10				 // Count of random samples for each pixel
#define DEFAULT_MAX_DEPTH		  20				 // Maximum number of ray bounces into scene
#define DEFAULT_VERTICAL_FOV	  20				 // Vertical field of view
#define DEFAULT_LOOKFROM		  point3_t{13, 2, 3} // Point camera is looking from
#define DEFAULT_LOOKAT			  point3_t{0, 0, 0}	 // Point camera is looking at
#define DEFAULT_VUP				  vec3_t{0, 1, 0}	 // Camera-relative "up" direction
#define DEFAULT_DEFOCUS_ANGLE	  0.6				 // Variation angle of rays through each pixel
#define DEFAULT_FOCUS_DIST		  10.0				 // Distance from camera lookfrom to plane of perfect focus

typedef struct camera_t
{
	point3_t center;
	point3_t pixel00_loc;
	vec3_t	 pixel_delta_u;
	vec3_t	 pixel_delta_v;
	vec3_t	 u, v, w;
	vec3_t	 defocus_disk_u;
	vec3_t	 defocus_disk_v;
	f64		 pixel_samples_scale;
	i32		 img_h;
	i32		 max_depth;
} camera_t;

internal vec3_t sample_square()
{
	return {random_f64() - 0.5, random_f64() - 0.5, 0};
}

internal point3_t defocus_disk_sample(camera_t* cam)
{
	vec3_t p = vec3_random_in_unit_disk();
	return cam->center + (p.x * cam->defocus_disk_u) + (p.y * cam->defocus_disk_v);
}

internal ray_t get_ray(camera_t* cam, i32 i, i32 j)
{
	vec3_t offset		= sample_square();
	vec3_t pixel_sample = cam->pixel00_loc						   //
						  + ((i + offset.x) * cam->pixel_delta_u)  //
						  + ((j + offset.y) * cam->pixel_delta_v); //

	point3_t ray_origin	   = (DEFAULT_DEFOCUS_ANGLE <= 0) ? cam->center : defocus_disk_sample(cam);
	vec3_t	 ray_direction = pixel_sample - ray_origin;

	return {ray_origin, ray_direction};
}

internal void camera_initialize(camera_t* camera)
{
	ASSERT(camera && "Trying to initialize a NULL camera");

	i32 img_h = (i32)(DEFAULT_IMG_W / DEFAULT_ASPECT_RATIO);
	img_h	  = (img_h < 1) ? 1 : img_h;

	f64		 theta		   = degrees_to_radians(DEFAULT_VERTICAL_FOV);
	f64		 h			   = tan(theta / 2);
	f64		 viewport_h	   = 2.0 * h * DEFAULT_FOCUS_DIST;
	f64		 viewport_w	   = viewport_h * ((f64)DEFAULT_IMG_W / img_h);
	point3_t camera_center = DEFAULT_LOOKFROM;

	camera->w = vec3_unit_vector(DEFAULT_LOOKFROM - DEFAULT_LOOKAT);
	camera->u = vec3_unit_vector(vec3_cross(DEFAULT_VUP, camera->w));
	camera->v = vec3_cross(camera->w, camera->u);

	vec3_t viewport_u = viewport_w * camera->u;
	vec3_t viewport_v = viewport_h * -camera->v;

	vec3_t pixel_delta_u = viewport_u / DEFAULT_IMG_W;
	vec3_t pixel_delta_v = viewport_v / img_h;

	point3_t viewport_upper_left = camera_center - (DEFAULT_FOCUS_DIST * camera->w) - viewport_u / 2 - viewport_v / 2;
	point3_t pixel00_loc		 = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

	f64 defocus_radius	   = DEFAULT_FOCUS_DIST * tan(degrees_to_radians((f64)DEFAULT_DEFOCUS_ANGLE / 2));
	camera->defocus_disk_u = camera->u * defocus_radius;
	camera->defocus_disk_v = camera->v * defocus_radius;

	camera->img_h				= img_h;
	camera->center				= DEFAULT_LOOKFROM;
	camera->pixel00_loc			= pixel00_loc;
	camera->pixel_delta_u		= pixel_delta_u;
	camera->pixel_delta_v		= pixel_delta_v;
	camera->pixel_samples_scale = 1.0 / DEFAULT_SAMPLES_PER_PIXEL;
	camera->max_depth			= DEFAULT_MAX_DEPTH;
}

internal color ray_color(ray_t* r, i32 depth, object_group_t* world)
{
	if (depth <= 0)
	{
		return {0, 0, 0};
	}

	hit_record_t rec;

	if (evaluate_all_object_hits(world, r, interval_t{0.001, INFINITY}, &rec))
	{
		ray_t scattered;
		color attenuation;

		if (scatter(rec.mat, r, &rec, &attenuation, &scattered))
		{
			return (attenuation * ray_color(&scattered, depth - 1, world));
		}

		return {0, 0, 0};
	}

	vec3_t unit_direction = vec3_unit_vector(r->direction);
	f64	   a			  = 0.5 * (unit_direction.y + 1.0);
	return (1.0 - a) * color{1.0, 1.0, 1.0} + a * color{0.5, 0.7, 1.0};
}

internal void render(camera_t* camera, object_group_t* world)
{
	FILE* img_file;
	char  header_buf[DEFAULT_BUF_SIZE];

	img_file = fopen("image.ppm", "wb");
	ASSERT(img_file && "Could not write to image.ppm.");

	u64 header_length = snprintf(header_buf, DEFAULT_BUF_SIZE, "P3\n%d %d\n255\n", DEFAULT_IMG_W, camera->img_h);
	fwrite(header_buf, sizeof(u8), header_length, img_file);

	char data_buf[DEFAULT_BUF_SIZE];
	for (i32 j = 0; j < camera->img_h; ++j)
	{
		printf("\rScanlines remaining: %d   ", camera->img_h - j);
		fflush(stdout);
		fflush(stdout);
		for (i32 i = 0; i < DEFAULT_IMG_W; ++i)
		{
			color pixel_color = {0, 0, 0};
			for (i32 sample = 0; sample < DEFAULT_SAMPLES_PER_PIXEL; ++sample)
			{
				ray_t r = get_ray(camera, i, j);
				pixel_color += ray_color(&r, camera->max_depth, world);
			}

			vec3i byterange	  = byterange_from_color(pixel_color * camera->pixel_samples_scale);
			u64	  data_length = snprintf(data_buf, DEFAULT_BUF_SIZE * sizeof(char), "%d %d %d\n", byterange[0],
										 byterange[1], byterange[2]);
			fwrite(data_buf, sizeof(u8), data_length, img_file);
		}
	}

	fclose(img_file);
}

#endif
