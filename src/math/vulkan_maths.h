//
// Created by nathan on 1/13/23.
//

#ifndef VULKAN_MATHS_H
#define VULKAN_MATHS_H

namespace Vulkan {

struct vec2
{
	float x;
	float y;

	vec2(float _x, float _y) : x(_x), y(_y) {}
};

struct vec3
{
	float x;
	float y;
	float z;

	vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};



}

#endif //VULKAN_MATHS_H
