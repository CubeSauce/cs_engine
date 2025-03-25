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
	quat inverseRotation = orientation.conjugate();
    mat4 R = inverseRotation.to_mat4();

    vec3 right = R[0].xyz;
    vec3 up = R[2].xyz; 
    vec3 forward = -R[1].xyz;

    _view = mat4(
        {right.x, up.x, forward.x, 0.0f},
        {right.y, up.y, forward.y, 0.0f},
        {right.z, up.z, forward.z, 0.0f},
        {-right.dot(position), -up.dot(position), -forward.dot(position), 1.0f}
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
	quat inverseRotation = orientation.conjugate();
    mat4 R = inverseRotation.to_mat4();

    vec3 right = R[0].xyz;
    vec3 up = R[2].xyz; 
    vec3 forward = -R[1].xyz;

    _view = mat4(
        {right.x, up.x, forward.x, 0.0f},
        {right.y, up.y, forward.y, 0.0f},
        {right.z, up.z, forward.z, 0.0f},
        {-right.dot(position), -up.dot(position), -forward.dot(position), 1.0f}
    );
}

