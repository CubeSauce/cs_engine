// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/renderer/camera.hpp"
#include "cs/math/math.hpp"

Camera::Camera(const mat4& projection, const mat4& view)
	: _projection(projection), _view(view)
{
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
	_view = look_at(position, target, vec3::up_vector);
}

void Perspective_Camera::calculate_projection()
{
	_projection = perspective_matrix(MATH_DEG_TO_RAD(FOV_deg), aspect_ratio, near_d, far_d);
}

void Perspective_Camera::calculate_view()
{
	_view = look_at(position, target, vec3::up_vector);
}

