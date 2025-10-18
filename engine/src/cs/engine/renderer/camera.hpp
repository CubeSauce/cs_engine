// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/math.hpp"

class Camera
{
public:
	vec3 position { vec3::zero_vector };
	quat orientation { quat::zero_quat };
	float aspect_ratio { 0.0f };

public:
	Camera(const mat4& projection = mat4(1.0f), const mat4& view = mat4(1.0f));
	virtual ~Camera() = default;

	virtual void calculate_projection() = 0;
	virtual void calculate_view() = 0;

	mat4 get_projection();
	mat4 get_view();
	mat4 get_world();

protected:
	mat4 _projection { mat4(1.0f) };
	mat4 _view { mat4(1.0f) };
	mat4 _world { mat4(1.0f) };
};

class Orthographic_Camera : public Camera
{
public:
	float left { 0.0f };
	float right { 0.0f };
	float bottom { 0.0f };
	float top { 0.0f };
	float near_d { 0.0f };
	float far_d { 0.0f };

public:
	virtual void calculate_projection() override;
	virtual void calculate_view() override;
};

class Perspective_Camera : public Camera
{
public:
	float FOV { 45_deg };
	float near_d { 0.1f };
	float far_d { 1000.0f };

public:
	virtual void calculate_projection() override;
	virtual void calculate_view() override;
};