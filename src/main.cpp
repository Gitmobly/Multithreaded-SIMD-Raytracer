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
	objects_t* world = (objects_t*)malloc(sizeof(objects_t));
	world->count	 = 0;

	camera_t* cam = (camera_t*)malloc(sizeof(camera_t));
	camera_initialize(cam);

	material_t* ground_material = (material_t*)malloc(sizeof(material_t));
	*ground_material			= material_t{
		.albedo			  = {.r = 0.5f, .g = 0.5f, .b = 0.5f},
		.fuzz			  = 0.0,
		.refraction_index = 0.0,
		.type			  = LAMBERTIAN,
	};

	b32 success;
	success = object_group_add(world, object_t{{0, -1000, 0}, 1000.0f, ground_material, SPHERE});
	if (!success)
	{
		exit(1);
	}

	for (i32 a = -11; a < 11; ++a)
	{
		for (i32 b = -11; b < 11; ++b)
		{
			ASSERT(success && "Added too many objects to the world.");
			f32		 choose_mat = random_f32();
			point3_t center		= {a + 0.9f * random_f32(), 0.2f, b + 0.9f * random_f32()};

			if (vec3_length(center - point3_t{4, 0.2f, 0}) > 0.9f)
			{
				// These will be leaked, it is ok given the context
				material_t* sphere_material = (material_t*)malloc(sizeof(material_t));

				if (choose_mat < 0.8)
				{
					// diffuse
					color albedo = vec3_random() * vec3_random();

					*sphere_material = material_t{
						.albedo			  = albedo,
						.fuzz			  = 0.0f,
						.refraction_index = 0.0f,
						.type			  = LAMBERTIAN,
					};
				}
				else if (choose_mat < 0.95)
				{
					// metal
					color albedo	 = vec3_random(0.5f, 1);
					f32	  fuzz		 = random_f32(0, 0.5f);
					*sphere_material = material_t{
						.albedo			  = albedo,
						.fuzz			  = fuzz,
						.refraction_index = 0.0f,
						.type			  = METAL,
					};
				}
				else
				{
					// glass
					*sphere_material = material_t{
						.albedo			  = {},
						.fuzz			  = 0.0f,
						.refraction_index = 1.5f,
						.type			  = DIELECTRIC,
					};
				}

				success = object_group_add(world, object_t{center, 0.2f, sphere_material, SPHERE});
			}
		}
	}

	material_t* mat1 = (material_t*)malloc(sizeof(material_t));
	material_t* mat2 = (material_t*)malloc(sizeof(material_t));
	material_t* mat3 = (material_t*)malloc(sizeof(material_t));

	*mat1 = material_t{
		.albedo			  = {},
		.fuzz			  = 0.0f,
		.refraction_index = 1.5f,
		.type			  = DIELECTRIC,
	};

	success = object_group_add(world, object_t{{0, 1, 0}, 1.0f, mat1, SPHERE});
	ASSERT(success && "Added too many objects to the world.");
	if (!success)
	{
		exit(1);
	}

	*mat2 = material_t{
		.albedo			  = {0.4f, 0.2f, 0.1f},
		.fuzz			  = 0.0f,
		.refraction_index = 0.0f,
		.type			  = LAMBERTIAN,
	};

	success = object_group_add(world, object_t{{-4, 1, 0}, 1.0f, mat2, SPHERE});
	ASSERT(success && "Added too many objects to the world.");
	if (!success)
	{
		exit(1);
	}

	*mat3 = material_t{
		.albedo			  = {0.7f, 0.6f, 0.5f},
		.fuzz			  = 0.0f,
		.refraction_index = 0.0f,
		.type			  = METAL,
	};

	success = object_group_add(world, object_t{{4, 1, 0}, 1.0, mat3, SPHERE});
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
