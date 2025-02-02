// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/mat4.hpp"

class Camera
{
public:
	vec3 position { vec3(0.0f) };
	vec3 target { vec3(0.0f) };
	float aspect_ratio { 0.0f };

public:
	Camera(const mat4& projection = mat4(), const mat4& view = mat4());
	virtual ~Camera() = default;

	virtual void calculate_projection() = 0;
	virtual void calculate_view() = 0;

	mat4 get_projection();
	mat4 get_view();

protected:
	mat4 _projection { mat4(1.0f) };
	mat4 _view { mat4(1.0f) };
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
	float FOV_deg { 0.0f };
	float near_d { 0.0f };
	float far_d { 0.0f };

public:
	virtual void calculate_projection() override;
	virtual void calculate_view() override;
};