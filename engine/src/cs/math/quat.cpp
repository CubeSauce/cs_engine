// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/math/quat.hpp"

#include <cmath>

quat quat::zero_quat = quat(0.0f, 1.0f);

quat::quat()
	: v(0.0f), w(1.0f)
{
}

quat::quat(const vec3& v, float w)
	: v(v), w(w)
{
}

quat::quat(const quat& other)
	: v(other.v), w(other.w)
{
}

mat4 quat::to_mat4() const
{
	float qxx(v.x * v.x);
	float qyy(v.y * v.y);
	float qzz(v.z * v.z);
	float qxz(v.x * v.z);
	float qxy(v.x * v.y);
	float qyz(v.y * v.z);
	float qwx(w * v.x);
	float qwy(w * v.y);
	float qwz(w * v.z);

	mat4 res(1.0f);
	res[0][0] = 1.0f - 2.0f * (qyy + qzz);
	res[1][0] = 2.0f * (qxy + qwz);
	res[2][0] = 2.0f * (qxz - qwy);

	res[0][1] = 2.0f * (qxy - qwz);
	res[1][1] = 1.0f - 2.0f * (qxx + qzz);
	res[2][1] = 2.0f * (qyz + qwx);

	res[0][2] = 2.0f * (qxz + qwy);
	res[1][2] = 2.0f * (qyz - qwx);
	res[2][2] = 1.0f - 2.0f * (qxx + qyy);

	return res;
}

quat quat::from_mat4(const mat4& m)
{
	float fourXSquaredMinus1 = m[0][0] - m[1][1] - m[2][2];
	float fourYSquaredMinus1 = m[1][1] - m[0][0] - m[2][2];
	float fourZSquaredMinus1 = m[2][2] - m[0][0] - m[1][1];
	float fourWSquaredMinus1 = m[0][0] + m[1][1] + m[2][2];

	int biggestIndex = 0;
	float fourBiggestSquaredMinus1 = fourWSquaredMinus1;
	if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourXSquaredMinus1;
		biggestIndex = 1;
	}
	if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourYSquaredMinus1;
		biggestIndex = 2;
	}
	if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourZSquaredMinus1;
		biggestIndex = 3;
	}

	float biggestVal = sqrt(fourBiggestSquaredMinus1 + static_cast<float>(1)) * static_cast<float>(0.5);
	float mult = -static_cast<float>(0.25) / biggestVal;

	switch (biggestIndex)
	{
	case 0: return quat(vec3(biggestVal, (m[1][2] - m[2][1]) * mult, (m[2][0] - m[0][2]) * mult), (m[0][1] - m[1][0]) * mult);
	case 1: return quat(vec3((m[1][2] - m[2][1]) * mult, biggestVal, (m[0][1] + m[1][0]) * mult), (m[2][0] + m[0][2]) * mult);
	case 2: return quat(vec3((m[2][0] - m[0][2]) * mult, (m[0][1] + m[1][0]) * mult, biggestVal), (m[1][2] + m[2][1]) * mult);
	case 3: return quat(vec3((m[0][1] - m[1][0]) * mult, (m[2][0] + m[0][2]) * mult, (m[1][2] + m[2][1]) * mult), biggestVal);
	}

	assert(false);
	return zero_quat;
}

quat quat::from_direction(const vec3& direction)
{
	const vec3 forward = direction.normalized();

	float dot = vec3::forward_vector.dot(forward);
	if (dot > 0.9999f)
	{
		return quat::zero_quat;
	}

	if (dot < -0.9999f)
	{
		return quat::from_rotation_axis(vec3::up_vector, MATH_DEG_TO_RAD(180.0f));
	}

	const vec3 axis = vec3::forward_vector.cross(forward).normalize();
	const float angle = acos(dot);

	return quat::from_rotation_axis(axis, angle);
}

quat quat::from_euler_angles(const vec3& euler)
{
    vec3 c = cos(euler * 0.5f);
    vec3 s = sin(euler * 0.5f);

	quat ret;
	ret.w = c.x * c.y * c.z + s.x * s.y * s.z;
    ret.v.x = s.x * c.y * c.z - c.x * s.y * s.z;
    ret.v.y = c.x * s.y * c.z + s.x * c.y * s.z;
    ret.v.z = c.x * c.y * s.z - s.x * s.y * c.z;

	return ret;
}

quat quat::from_rotation_axis(const vec3& axis, float angle)
{
	return quat(axis * sin(angle * 0.5f), cos(angle * 0.5f));
}

quat quat::mul(const quat& other) const
{
	return quat(
		other.v * w + v * other.w + v.cross(other.v), 
		w * other.w - v.dot(other.v)
	).normalized();
}

vec3 quat::mul(const vec3& other) const
{
	const vec3 t = v.cross(other) * 2.0f;
	return other + t * w + v.cross(t);
}
    
void quat::normalize()
{
	const float length = sqrt(v.dot(v) + w * w);
	v /= length;
	w /= length;
}

quat quat::normalized() const
{
	quat result(*this);
	result.normalize();
	return result;
}

quat quat::conjugate() const
{
	return quat(vec3::zero_vector - v, w);
}

vec3 quat::get_direction(const vec3& forward_axis)
{
	return forward_axis + (v.cross(forward_axis) + forward_axis * w).cross(v) * 2.0f;
}

mat4 rotation(const quat& rotation)
{
	return rotation.to_mat4();
}

quat slerp(quat qa, quat qb, float t)
{
	quat qm;

	float cosHalfTheta = qa.w * qb.w + qa.v.x * qb.v.x + qa.v.y * qb.v.y + qa.v.z * qb.v.z;

	if (abs(cosHalfTheta) >= 1.0f) 
	{
		qm.w = qa.w; qm.v.x = qa.v.x; qm.v.y = qa.v.y; qm.v.z = qa.v.z;
		return qm;
	}

	float halfTheta = acos(cosHalfTheta);
	float sinHalfTheta = sqrt(1.0f - cosHalfTheta * cosHalfTheta);

	if (fabs(sinHalfTheta) < 0.001f) 
	{
		qm.w = (qa.w * 0.5f + qb.w * 0.5f);
		qm.v.x = (qa.v.x * 0.5f + qb.v.x * 0.5f);
		qm.v.y = (qa.v.y * 0.5f + qb.v.y * 0.5f);
		qm.v.z = (qa.v.z * 0.5f + qb.v.z * 0.5f);
		return qm;
	}

	float ratioA = sin((1.0f - t) * halfTheta) / sinHalfTheta;
	float ratioB = sin(t * halfTheta) / sinHalfTheta;

	qm.w = (qa.w * ratioA + qb.w * ratioB);
	qm.v.x = (qa.v.x * ratioA + qb.v.x * ratioB);
	qm.v.y = (qa.v.y * ratioA + qb.v.y * ratioB);
	qm.v.z = (qa.v.z * ratioA + qb.v.z * ratioB);

	return qm;
}
