#ifndef MATERIAL_TYPE_H
#define MATERIAL_TYPE_H

#include "color.h"

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

#endif
