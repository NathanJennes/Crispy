//
// Created by nathan on 1/29/23.
//

#include <iostream>
#include "Camera.h"
#include "input_codes.h"
#include "input.h"

namespace Vulkan {

glm::vec3	Camera::position;
glm::vec3	Camera::rotation;
glm::vec3	Camera::up;
glm::vec3	Camera::forward;

glm::mat4	Camera::view_matrix;
glm::mat4	Camera::projection_matrix;
glm::mat4	Camera::camera_matrix;

float		Camera::fov = 0.0f;
float		Camera::near_plane = 0.0f;
float		Camera::far_plane = 0.0f;
float		Camera::base_speed = 0.0f;
float		Camera::sensitivity = 0.1f;

void Camera::initialize(const glm::vec3 &pos, const glm::vec3& rot)
{
	position = pos;
	rotation = rot;
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	near_plane = 0.1f;
	far_plane = 100.0f;
	fov = 45.0f;
	base_speed = 0.25f;

	recalculate_forward_vector();
	recalculate_view_matrix();
	recalculate_projection_matrix();
	recalculate_camera_matrix();
}

void Camera::update()
{
	if (Window::did_resize()) {
		recalculate_projection_matrix();
		recalculate_camera_matrix();
	}

	glm::vec3 movement{};
	if (is_key_down(Keys::W))
        movement += base_speed * forward;
	if (is_key_down(Keys::S))
        movement -= base_speed * forward;
	if (is_key_down(Keys::A))
		movement -= base_speed * glm::normalize(glm::cross(forward, up));
	if (is_key_down(Keys::D))
        movement += base_speed * glm::normalize(glm::cross(forward, up));
	if (is_key_down(Keys::LSHIFT))
        movement -= base_speed * up;
	if (is_key_down(Keys::SPACE))
	    movement += base_speed * up;
	position += movement;

	glm::vec2 mouse_movement = (Window::get_mouse_pos() - Window::get_last_mouse_pos());
	mouse_movement *= sensitivity;
	std::cout << "Mouse movement: " << mouse_movement.x << ", " << mouse_movement.y << std::endl;
	rotation.y += mouse_movement.x;
	rotation.x -= mouse_movement.y;
	if (rotation.x > 80.0f)
		rotation.x = 80.0f;
	if (rotation.x < -80.0f)
		rotation.x = 80.0f;
	std::cout << "Rotation: " << rotation.x << ", " << rotation.y << std::endl;

	bool need_to_recalculate_camera_matrix = false;
	if (movement.x != 0.0f || movement.y != 0.0f || movement.z != 0.0f) {
		recalculate_view_matrix();
		need_to_recalculate_camera_matrix = true;
	}
	if (mouse_movement.x != 0.0f || mouse_movement.y != 0.0f) {
		recalculate_forward_vector();
		recalculate_view_matrix();
		need_to_recalculate_camera_matrix = true;
	}
	if (need_to_recalculate_camera_matrix)
		recalculate_camera_matrix();
}

void Camera::recalculate_forward_vector()
{
	glm::vec3 direction;
	direction.x = cosf(glm::radians(rotation.y)) * cosf(glm::radians(rotation.x));
	direction.y = sinf(glm::radians(rotation.x));
	direction.z = sinf(glm::radians(rotation.y)) * cosf(glm::radians(rotation.x));
	forward = glm::normalize(direction);
}

void Camera::recalculate_view_matrix()
{
	view_matrix = glm::lookAt(position, position + forward, up);
}

void Camera::recalculate_projection_matrix()
{
	projection_matrix = glm::perspective(glm::radians(fov), Window::get_aspect_ratio(), near_plane, far_plane);
	projection_matrix[1][1] *= -1;
}

void Camera::recalculate_camera_matrix()
{
	camera_matrix = projection_matrix * view_matrix;

}
} // Vulkan