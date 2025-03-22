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
	const mat4 r = orientation.to_mat4();
	vec3 right = r[0].xyz;
	vec3 up = r[2].xyz;
	vec3 forward = vec3::zero_vector - r[1].xyz;  //negate for right handed

	_view = mat4(
		vec4(right.x, up.x, forward.x, 0.0f),
		vec4(right.y, up.y, forward.y, 0.0f),
		vec4(right.z, up.z, forward.z, 0.0f),
		vec4(-right.dot(position), -up.dot(position), -forward.dot(position), 1.0f)
	);
}

void Perspective_Camera::calculate_projection()
{
	_projection = perspective_matrix(FOV, aspect_ratio, near_d, far_d);
}

const mat4 correction = {
	vec4(1, 0,  0, 0),
	vec4(0, 0, -1, 0),
	vec4(0, 1,  0, 0),
	vec4(0, 0,  0, 1)
};

void Perspective_Camera::calculate_view()
{
	//_view = correction * orientation.conjugate().to_mat4();
	//_view = _view * translate(mat4(1.0f), vec3::zero_vector - position);
	quat inverseRotation = orientation.conjugate();
    mat4 R = inverseRotation.to_mat4();

    vec3 right = -R[0].xyz;
    vec3 up = R[2].xyz; 
    vec3 forward = -R[1].xyz; // +Z is forward in DirectX

    _view = mat4(
        {right.x, up.x, forward.x, 0.0f},
        {right.y, up.y, forward.y, 0.0f},
        {right.z, up.z, forward.z, 0.0f},
        {-right.dot(position), -up.dot(position), -forward.dot(position), 1.0f}
    );
}

