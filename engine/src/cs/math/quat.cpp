// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/math/quat.hpp"

#include <cmath>

quat quat::zero_quat = quat(0.0f, 0.0f, 0.0f, 1.0f);

quat::quat()
	: x(0.0f), y(0.0f), z(0.0f), w(1.0f)
{
}

quat::quat(float x, float y, float z, float w)
	: x(x), y(y), z(z), w(w)
{
}

quat::quat(const quat& other)
	: x(other.x), y(other.y), z(other.z), w(other.w)
{
}

mat4 quat::to_mat4() const
{
	float qxx(x * x);
	float qyy(y * y);
	float qzz(z * z);
	float qxz(x * z);
	float qxy(x * y);
	float qyz(y * z);
	float qwx(w * x);
	float qwy(w * y);
	float qwz(w * z);

	mat4 res(1.0f);
	res[0][0] = float(1) - float(2) * (qyy + qzz);
	res[1][0] = float(2) * (qxy + qwz);
	res[2][0] = float(2) * (qxz - qwy);

	res[0][1] = float(2) * (qxy - qwz);
	res[1][1] = float(1) - float(2) * (qxx + qzz);
	res[2][1] = float(2) * (qyz + qwx);

	res[0][2] = float(2) * (qxz + qwy);
	res[1][2] = float(2) * (qyz - qwx);
	res[2][2] = float(1) - float(2) * (qxx + qyy);

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
	float mult = static_cast<float>(0.25) / biggestVal;

	switch (biggestIndex)
	{
	case 0: return quat(biggestVal, (m[1][2] - m[2][1]) * mult, (m[2][0] - m[0][2]) * mult, (m[0][1] - m[1][0]) * mult);
	case 1: return quat((m[1][2] - m[2][1]) * mult, biggestVal, (m[0][1] + m[1][0]) * mult, (m[2][0] + m[0][2]) * mult);
	case 2: return quat((m[2][0] - m[0][2]) * mult, (m[0][1] + m[1][0]) * mult, biggestVal, (m[1][2] + m[2][1]) * mult);
	case 3: return quat((m[0][1] - m[1][0]) * mult, (m[2][0] + m[0][2]) * mult, (m[1][2] + m[2][1]) * mult, biggestVal);
	}

	assert(false);
	return quat(1, 0, 0, 0);
}

quat quat::from_euler_angles(const vec3& euler)
{
    vec3 c = cos(euler * float(0.5));
    vec3 s = sin(euler * float(0.5));

	quat ret;
	ret.w = c.x * c.y * c.z + s.x * s.y * s.z;
    ret.x = s.x * c.y * c.z - c.x * s.y * s.z;
    ret.y = c.x * s.y * c.z + s.x * c.y * s.z;
    ret.z = c.x * c.y * s.z - s.x * s.y * c.z;

	return ret;
}

mat4 rotation(const quat& rotation)
{
	return rotation.to_mat4();
}

quat slerp(quat qa, quat qb, float t)
{
	quat qm;

	float cosHalfTheta = qa.w * qb.w + qa.x * qb.x + qa.y * qb.y + qa.z * qb.z;

	if (abs(cosHalfTheta) >= 1.0) 
	{
		qm.w = qa.w; qm.x = qa.x; qm.y = qa.y; qm.z = qa.z;
		return qm;
	}

	float halfTheta = acos(cosHalfTheta);
	float sinHalfTheta = sqrt(1.0 - cosHalfTheta * cosHalfTheta);

	if (fabs(sinHalfTheta) < 0.001) 
	{
		qm.w = (qa.w * 0.5 + qb.w * 0.5);
		qm.x = (qa.x * 0.5 + qb.x * 0.5);
		qm.y = (qa.y * 0.5 + qb.y * 0.5);
		qm.z = (qa.z * 0.5 + qb.z * 0.5);
		return qm;
	}

	float ratioA = sin((1 - t) * halfTheta) / sinHalfTheta;
	float ratioB = sin(t * halfTheta) / sinHalfTheta;

	qm.w = (qa.w * ratioA + qb.w * ratioB);
	qm.x = (qa.x * ratioA + qb.x * ratioB);
	qm.y = (qa.y * ratioA + qb.y * ratioB);
	qm.z = (qa.z * ratioA + qb.z * ratioB);

	return qm;
}
