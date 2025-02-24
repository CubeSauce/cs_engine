// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/renderer/camera.hpp"
#include "cs/math/math.hpp"

Camera::Camera(const mat4& projection, const mat4& view)
	: _projection(projection), _view(view)
{
}

mat4 Camera::get_world()
{
	return _world;
}

mat4 Camera::get_view()
{
	return _view;
}

mat4 Camera::get_projection()
{ 
	return _projection; 
}

void Orthographic_Camera::calculate_projection()
{
	_projection = orthographic_matrix(vec4(left, right, bottom, top), near_d, far_d);
}

void Orthographic_Camera::calculate_view()
{
	const mat4 r = orientation.to_mat4();;
	vec3 right = r[0].xyz;
	vec3 up = r[1].xyz;
	vec3 forward = vec3::zero_vector - r[2].xyz; //negate for right handed

	_view = mat4(
		vec4(right.x, up.x, forward.x, 0.0f),
		vec4(right.y, up.y, forward.y, 0.0f),
		vec4(right.z, up.z, forward.z, 0.0f),
		vec4(-right.dot(position), -up.dot(position), -forward.dot(position), 1.0f)
	);
}

void Perspective_Camera::calculate_projection()
{
	_projection = perspective_matrix(MATH_DEG_TO_RAD(FOV_deg), aspect_ratio, near_d, far_d);
}

void Perspective_Camera::calculate_view()
{
	const mat4 r = orientation.to_mat4();
	vec3 right = r[0].xyz;
	vec3 up = r[1].xyz;
	vec3 forward = vec3::zero_vector - r[2].xyz; //negate for right handed

	_view = mat4(
		vec4(right.x, up.x, forward.x, 0.0f),
		vec4(right.y, up.y, forward.y, 0.0f),
		vec4(right.z, up.z, forward.z, 0.0f),
		vec4(-right.dot(position), -up.dot(position), -forward.dot(position), 1.0f)
	);
}

