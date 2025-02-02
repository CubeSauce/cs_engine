// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/math/mat4.hpp"
#include "cs/math/vec3.hpp"

#include <cmath>

mat4::mat4()
{
}

mat4::mat4(float v)
{
	columns[0] = vec4(v, 0.0f, 0.0f, 0.0f);
	columns[1] = vec4(0.0f, v, 0.0f, 0.0f);
	columns[2] = vec4(0.0f, 0.0f, v, 0.0f);
	columns[3] = vec4(0.0f, 0.0f, 0.0f, v);
}

mat4::mat4(const mat4& other)
{
	columns[0] = other.columns[0];
	columns[1] = other.columns[1];
	columns[2] = other.columns[2];
	columns[3] = other.columns[3];
}

mat4::mat4(const vec4& col0, const vec4& col1, const vec4& col2, const vec4& col3)
	: columns{ col0, col1, col2, col3 }
{

}

vec4& mat4::operator[](int32 index)
{
	if (index < 0 || index > 3)
	{
		return columns[0];
	}

	return columns[index];
}

const vec4& mat4::operator[](int32 index) const
{
	if (index < 0 || index > 3)
	{
		return columns[0];
	}

	return columns[index];
}

mat4 mat4::operator*(const mat4& other) const
{
	vec4 const SrcA0 = columns[0];
	vec4 const SrcA1 = columns[1];
	vec4 const SrcA2 = columns[2];
	vec4 const SrcA3 = columns[3];
	vec4 const SrcB0 = other[0];
	vec4 const SrcB1 = other[1];
	vec4 const SrcB2 = other[2];
	vec4 const SrcB3 = other[3];

	mat4 result;
	result[0] = SrcA0 * SrcB0[0] + SrcA1 * SrcB0[1] + SrcA2 * SrcB0[2] + SrcA3 * SrcB0[3];
	result[1] = SrcA0 * SrcB1[0] + SrcA1 * SrcB1[1] + SrcA2 * SrcB1[2] + SrcA3 * SrcB1[3];
	result[2] = SrcA0 * SrcB2[0] + SrcA1 * SrcB2[1] + SrcA2 * SrcB2[2] + SrcA3 * SrcB2[3];
	result[3] = SrcA0 * SrcB3[0] + SrcA1 * SrcB3[1] + SrcA2 * SrcB3[2] + SrcA3 * SrcB3[3];
	return result;
}

mat4& mat4::transpose()
{
	mat4 ret(*this);

	columns[0][0] = ret.columns[0][0];
	columns[0][1] = ret.columns[1][0];
	columns[0][2] = ret.columns[2][0];
	columns[0][3] = ret.columns[3][0];
	columns[1][0] = ret.columns[0][1];
	columns[1][1] = ret.columns[1][1];
	columns[1][2] = ret.columns[2][1];
	columns[1][3] = ret.columns[3][1];
	columns[2][0] = ret.columns[0][2];
	columns[2][1] = ret.columns[1][2];
	columns[2][2] = ret.columns[2][2];
	columns[2][3] = ret.columns[3][2];
	columns[3][0] = ret.columns[0][3];
	columns[3][1] = ret.columns[1][3];
	columns[3][2] = ret.columns[2][3];
	columns[3][3] = ret.columns[3][3];

	return *this;
}

mat4 mat4::transposed() const
{
	mat4 ret;

	ret.columns[0][0] = columns[0][0];
	ret.columns[0][1] = columns[1][0];
	ret.columns[0][2] = columns[2][0];
	ret.columns[0][3] = columns[3][0];

	ret.columns[1][0] = columns[0][1];
	ret.columns[1][1] = columns[1][1];
	ret.columns[1][2] = columns[2][1];
	ret.columns[1][3] = columns[3][1];

	ret.columns[2][0] = columns[0][2];
	ret.columns[2][1] = columns[1][2];
	ret.columns[2][2] = columns[2][2];
	ret.columns[2][3] = columns[3][2];

	ret.columns[3][0] = columns[0][3];
	ret.columns[3][1] = columns[1][3];
	ret.columns[3][2] = columns[2][3];
	ret.columns[3][3] = columns[3][3];

	return ret;
}

mat4 mat4::inverse() const
{
	mat4 m(*this);

	float Coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
	float Coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
	float Coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

	float Coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
	float Coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
	float Coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

	float Coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
	float Coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
	float Coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

	float Coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
	float Coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
	float Coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

	float Coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
	float Coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
	float Coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

	float Coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
	float Coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
	float Coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

	vec4 Fac0(Coef00, Coef00, Coef02, Coef03);
	vec4 Fac1(Coef04, Coef04, Coef06, Coef07);
	vec4 Fac2(Coef08, Coef08, Coef10, Coef11);
	vec4 Fac3(Coef12, Coef12, Coef14, Coef15);
	vec4 Fac4(Coef16, Coef16, Coef18, Coef19);
	vec4 Fac5(Coef20, Coef20, Coef22, Coef23);

	vec4 Vec0(m[1][0], m[0][0], m[0][0], m[0][0]);
	vec4 Vec1(m[1][1], m[0][1], m[0][1], m[0][1]);
	vec4 Vec2(m[1][2], m[0][2], m[0][2], m[0][2]);
	vec4 Vec3(m[1][3], m[0][3], m[0][3], m[0][3]);

	vec4 Inv0(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
	vec4 Inv1(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
	vec4 Inv2(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
	vec4 Inv3(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

	vec4 SignA(+1, -1, +1, -1);
	vec4 SignB(-1, +1, -1, +1);
	mat4 Inverse(Inv0 * SignA, Inv1 * SignB, Inv2 * SignA, Inv3 * SignB);

	vec4 Row0(Inverse[0][0], Inverse[1][0], Inverse[2][0], Inverse[3][0]);

	vec4 Dot0(m[0] * Row0);
	float Dot1 = (Dot0.x + Dot0.y) + (Dot0.z + Dot0.w);

	float OneOverDeterminant = static_cast<float>(1) / Dot1;

	return Inverse * OneOverDeterminant;
}

mat4 translate(const mat4& other, const vec3& translation)
{
	mat4 result(other);
	result[3] = other[0] * translation[0] + other[1] * translation[1] + other[2] * translation[2] + other[3];
	return result;
}

mat4 rotate(const mat4& other, float angle, const vec3& rotation_axis)
{
	mat4 Rotate(1.0f);

	const float a = angle;
	const float c = cosf(a);
	const float s = sinf(a);
	const vec3 axis = rotation_axis.normalized();
	const vec3 temp = vec3(axis * (1.0f - c));

	Rotate[0][0] = c + temp[0] * axis[0];
	Rotate[0][1] = temp[0] * axis[1] + s * axis[2];
	Rotate[0][2] = temp[0] * axis[2] - s * axis[1];

	Rotate[1][0] = temp[1] * axis[0] - s * axis[2];
	Rotate[1][1] = c + temp[1] * axis[1];
	Rotate[1][2] = temp[1] * axis[2] + s * axis[0];

	Rotate[2][0] = temp[2] * axis[0] + s * axis[1];
	Rotate[2][1] = temp[2] * axis[1] - s * axis[0];
	Rotate[2][2] = c + temp[2] * axis[2];

	mat4 result(other);
	result.columns[0] = other[0] * Rotate[0][0] + other[1] * Rotate[0][1] + other[2] * Rotate[0][2];
	result.columns[1] = other[0] * Rotate[1][0] + other[1] * Rotate[1][1] + other[2] * Rotate[1][2];
	result.columns[2] = other[0] * Rotate[2][0] + other[1] * Rotate[2][1] + other[2] * Rotate[2][2];
	result.columns[3] = other[3];
	return result;
}

mat4 scale(const mat4& other, const vec3& scaling)
{
	mat4 result;
	result[0] = other[0] * scaling[0];
	result[1] = other[1] * scaling[1];
	result[2] = other[2] * scaling[2];
	result[3] = other[0];
	return result;
}

mat4 orthographic_matrix(const vec4& frustum, float near, float far)
{
	mat4 ret(1.0f);

	const float& left = frustum[0];
	const float& right = frustum[1];
	const float& bottom = frustum[2];
	const float& top = frustum[3];

	ret[0][0] = 2.0f / (right - left);
	ret[1][1] = 2.0f / (top - bottom);
	ret[2][2] = -2.0f / (far - near);
	ret[3][0] = -(right + left) / (right - left);
	ret[3][1] = -(top + bottom) / (top - bottom);
	ret[3][2] = -(far + near) / (far - near);

	return ret;
}

mat4 perspective_matrix(float angle, float aspect_ratio, float near, float far)
{
	mat4 ret(1.0f);

	const float tan_half_fov_y = tanf(angle / 2.0f);

	ret[0][0] = 1.0f / (aspect_ratio * tan_half_fov_y);
	ret[1][1] = 1.0f / (tan_half_fov_y);
	ret[2][2] = -(far + near) / (far - near);
	ret[2][3] = -1.0f;
	ret[3][2] = -(2.0f * far * near) / (far - near);

	return ret;
}

mat4 look_at(vec3 const& eye, vec3 const& center, vec3 const& up)
{
	const vec3 f = (center - eye).normalized();
	vec3 u = up.normalized();
	const vec3 s = f.cross(u).normalized();
	u = s.cross(f);

	mat4 ret(1.0f);

	ret[0][0] = s.x;
	ret[1][0] = s.y;
	ret[2][0] = s.z;
	ret[0][1] = u.x;
	ret[1][1] = u.y;
	ret[2][1] = u.z;
	ret[0][2] = -f.x;
	ret[1][2] = -f.y;
	ret[2][2] = -f.z;
	ret[3][0] = -s.dot(eye);
	ret[3][1] = -u.dot(eye);
	ret[3][2] = f.dot(eye);

	return ret;
}
