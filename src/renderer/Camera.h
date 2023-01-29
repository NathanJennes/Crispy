//
// Created by nathan on 1/29/23.
//

#ifndef CAMERA_H
#define CAMERA_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Window.h"

namespace Vulkan {

class Camera
{
public:		// Methods
	//----
	// Initialization
	//----
	static void				initialize(const glm::vec3& pos, const glm::vec3& rot = {0.0f, 90.0f, 0.0f});
	static void				update();

	//----
	// Getters
	//----
	static const	glm::mat4&	get_view_matrix() 		{ return view_matrix; };
	static const	glm::mat4&	get_projection_matrix()	{ return projection_matrix; }
	static const	glm::mat4&	get_camera_matrix()		{ return camera_matrix; }

private:	// Methods
	static void	recalculate_forward_vector();
	static void recalculate_view_matrix();
	static void	recalculate_projection_matrix();
	static void	recalculate_camera_matrix();

private:	// Members
	static glm::vec3	position;
	static glm::vec3	rotation;
	static glm::vec3	up;
	static glm::vec3	forward;

	static glm::mat4	view_matrix;
	static glm::mat4	projection_matrix;
	static glm::mat4	camera_matrix;

	static float		fov;
	static float		near_plane;
	static float		far_plane;
	static float		base_speed;
	static float 		sensitivity;
};

} // Vulkan

#endif //CAMERA_H
