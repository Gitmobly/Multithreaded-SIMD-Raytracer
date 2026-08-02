#include "camera.h"
#include "hittable.h"
#include "material.h"
#include "object.h"
#include "types.h"

#include <cstdlib>
#include <stdio.h>

int main(void)
{
	// World
	object_group_t* world = (object_group_t*)malloc(sizeof(object_group_t));
	world->count		  = 0;

	camera_t* cam = (camera_t*)malloc(sizeof(camera_t));
	camera_initialize(cam);

	material_t* ground_material = (material_t*)malloc(sizeof(material_t));
	*ground_material			= material_t{
				   .type			 = LAMBERTIAN,
				   .albedo			 = {0.5, 0.5, 0.5},
				   .fuzz			 = 0.0,
				   .refraction_index = 0.0,
	   };

	b32 success;
	success = object_group_add(world, object_t{SPHERE, {0, -1000, 0}, 1000.0, ground_material});
	if (!success)
	{
		exit(1);
	}

	for (i32 a = -11; a < 11; ++a)
	{
		for (i32 b = -11; b < 11; ++b)
		{
			ASSERT(success && "Added too many objects to the world.");
			f64		 choose_mat = random_f64();
			point3_t center		= {a + 0.9 * random_f64(), 0.2, b + 0.9 * random_f64()};

			if (vec3_length(center - point3_t{4, 0.2, 0}) > 0.9)
			{
				// These will be leaked, it is ok given the context
				material_t* sphere_material = (material_t*)malloc(sizeof(material_t));

				if (choose_mat < 0.8)
				{
					// diffuse
					color albedo = vec3_random() * vec3_random();

					*sphere_material = material_t{
						.type			  = LAMBERTIAN,
						.albedo			  = albedo,
						.fuzz			  = 0.0,
						.refraction_index = 0.0,
					};
				}
				else if (choose_mat < 0.95)
				{
					// metal
					color albedo	 = vec3_random(0.5, 1);
					f64	  fuzz		 = random_f64(0, 0.5);
					*sphere_material = material_t{
						.type			  = METAL,
						.albedo			  = albedo,
						.fuzz			  = fuzz,
						.refraction_index = 0.0,
					};
				}
				else
				{
					// glass
					*sphere_material = material_t{
						.type			  = DIELECTRIC,
						.albedo			  = {},
						.fuzz			  = 0.0,
						.refraction_index = 1.5,
					};
				}

				success = object_group_add(world, object_t{SPHERE, center, 0.2, sphere_material});
			}
		}
	}

	material_t* mat1 = (material_t*)malloc(sizeof(material_t));
	material_t* mat2 = (material_t*)malloc(sizeof(material_t));
	material_t* mat3 = (material_t*)malloc(sizeof(material_t));

	*mat1 = material_t{
		.type			  = DIELECTRIC,
		.albedo			  = {},
		.fuzz			  = 0.0,
		.refraction_index = 1.5,
	};

	success = object_group_add(world, object_t{SPHERE, {0, 1, 0}, 1.0, mat1});
	ASSERT(success && "Added too many objects to the world.");
	if (!success)
	{
		exit(1);
	}

	*mat2 = material_t{
		.type			  = LAMBERTIAN,
		.albedo			  = {0.4, 0.2, 0.1},
		.fuzz			  = 0.0,
		.refraction_index = 0.0,
	};

	success = object_group_add(world, object_t{SPHERE, {-4, 1, 0}, 1.0, mat2});
	ASSERT(success && "Added too many objects to the world.");
	if (!success)
	{
		exit(1);
	}

	*mat3 = material_t{
		.type			  = METAL,
		.albedo			  = {0.7, 0.6, 0.5},
		.fuzz			  = 0.0,
		.refraction_index = 0.0,
	};

	success = object_group_add(world, object_t{SPHERE, {4, 1, 0}, 1.0, mat3});
	ASSERT(success && "Added too many objects to the world.");
	if (!success)
	{
		exit(1);
	}

	render(cam, world);

	free(mat3);
	free(mat2);
	free(mat1);
	free(cam);
	free(world);
	printf("\rDone.                          \n");

	return 0;
}
