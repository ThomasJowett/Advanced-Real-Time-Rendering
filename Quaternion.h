#pragma once
/**
* Holds a three degree of freedom orientation.
*
* Quaternions have
* several mathematical properties that make them useful for
* representing orientations, but require four items of data to
* hold the three degrees of freedom. These four items of data can
* be viewed as the coefficients of a complex number with three
* imaginary parts. The mathematics of the quaternion is then
* defined and is roughly correspondent to the math of 3D
* rotations. A quaternion is only a valid rotation if it is
* normalised: i.e. it has a length of 1.
*
* @note Angular velocity and acceleration can be correctly
* represented as vectors. Quaternions are only needed for
* orientation.
*/

#include <float.h>
#include <math.h>
#include <directxmath.h>
#include <d3d11_1.h>
#include "Vector.h"

using namespace DirectX;

class Quaternion
{
public:
	/**
	* Holds the real component of the quaternion.
	*/
	float r;

	/**
	* Holds the first complex component of the
	* quaternion.
	*/
	float i;

	/**
	* Holds the second complex component of the
	* quaternion.
	*/
	float j;

	/**
	* Holds the third complex component of the
	* quaternion.
	*/
	float k;

	/**
	* The default constructor creates a quaternion representing
	* a zero rotation.
	*/
	Quaternion() : r(1), i(0), j(0), k(0) {}

	/**
	* The explicit constructor creates a quaternion with the given
	* components.
	*
	* @param r The real component of the rigid body's orientation
	* quaternion.
	*
	* @param i The first complex component of the rigid body's
	* orientation quaternion.
	*
	* @param j The second complex component of the rigid body's
	* orientation quaternion.
	*
	* @param k The third complex component of the rigid body's
	* orientation quaternion.
	*
	* @note The given orientation does not need to be normalised,
	* and can be zero. This function will not alter the given
	* values, or normalise the quaternion. To normalise the
	* quaternion (and make a zero quaternion a legal rotation),
	* use the normalise function.
	*
	* @see normalise
	*/

	//from 4 floats
	Quaternion(const float r, const float i, const float j, const float k)
		: r(r), i(i), j(j), k(k)
	{
	}

	//from a Quaternion
	Quaternion(const Quaternion& q)
		:r(q.r), i(q.i), j(q.j), k(q.k)
	{
	}

	//from a vector of 3 euler angles in radians
	Quaternion(const Vector3D angles)
	{
		float cos_x_2 = cosf(0.5f*angles.x);
		float cos_y_2 = cosf(0.5f*angles.y);
		float cos_z_2 = cosf(0.5f*angles.z);

		float sin_x_2 = sinf(0.5f*angles.x);
		float sin_y_2 = sinf(0.5f*angles.y);
		float sin_z_2 = sinf(0.5f*angles.z);

		// and now compute quaternion
		r = cos_z_2 * cos_y_2*cos_x_2 + sin_z_2 * sin_y_2*sin_x_2;
		i = cos_z_2 * cos_y_2*sin_x_2 - sin_z_2 * sin_y_2*cos_x_2;
		j = cos_z_2 * sin_y_2*cos_x_2 + sin_z_2 * cos_y_2*sin_x_2;
		k = sin_z_2 * cos_y_2*cos_x_2 - cos_z_2 * sin_y_2*sin_x_2;
	}

	//from 3 euler angles in radians
	Quaternion(const float theta_Roll, float theta_Pitch, float theta_Yaw)
	{
		(*this) = Quaternion(Vector3D(theta_Roll, theta_Pitch, theta_Yaw));
	}

	~Quaternion() {}

	/**
	* Normalises the quaternion to unit length, making it a valid
	* orientation quaternion.
	*/

	float GetSqrMagnitude()
	{
		return r * r + i * i + j * j + k * k;
	}

	float GetMagnitude()
	{
		return sqrtf(GetSqrMagnitude());
	}

	void Normalize()
	{
		float d = GetSqrMagnitude();

		// Check for zero length quaternion, and use the no-rotation
		// quaternion in that case.
		if (d < FLT_EPSILON)
		{
			r = 1;
			return;
		}

		d = static_cast<float>(1.0) / sqrtf(d);
		r *= d;
		i *= d;
		j *= d;
		k *= d;
	}

	Quaternion Conjugate()
	{
		return Quaternion(r, -i, -j, -k);
	}

	Quaternion Scale(float scaler)
	{
		return Quaternion(r*scaler, i*scaler, j*scaler, k*scaler);
	}

	Quaternion Inverse()
	{
		return Conjugate().Scale(1 / GetSqrMagnitude());
	}

	Quaternion UnitQuaternion()
	{
		return (*this).Scale(1 / (*this).GetMagnitude());
	}

	//Operators---------------------------------------------------------------------------------

	Quaternion operator = (const Quaternion& q)
	{
		r = q.r;
		i = q.i;
		j = q.j;
		k = q.k;

		return (*this);
	}

	Quaternion operator + (const Quaternion& q)
	{
		return Quaternion(r + q.r, i + q.i, j + q.j, k + q.k);
	}

	Quaternion operator - (const Quaternion& q)
	{
		return Quaternion(r - q.r, i - q.i, j - q.j, k - q.k);
	}

	Quaternion operator * (const Quaternion& q)
	{
		return Quaternion(
			r*q.r - i * q.i - j * q.j - k * q.k,
			r*q.i + i * q.r + j * q.k - k * q.j,
			r*q.j + j * q.r + k * q.i - i * q.k,
			r*q.k + k * q.r + i * q.j - j * q.i
		);
	}

	Vector3D operator*(const Vector3D& multiplier)
	{
		Vector3D V = multiplier;
		return RotateVectorByQuaternion(V);
	}

	/**
	* Multiplies the quaternion by the given quaternion.
	*
	* @param multiplier The quaternion by which to multiply.
	*/
	void operator *=(const Quaternion &multiplier)
	{
		Quaternion q = *this;
		r = q.r*multiplier.r - q.i*multiplier.i -
			q.j*multiplier.j - q.k*multiplier.k;
		i = q.r*multiplier.i + q.i*multiplier.r +
			q.j*multiplier.k - q.k*multiplier.j;
		j = q.r*multiplier.j + q.j*multiplier.r +
			q.k*multiplier.i - q.i*multiplier.k;
		k = q.r*multiplier.k + q.k*multiplier.r +
			q.i*multiplier.j - q.j*multiplier.i;
	}

	/**
	* Adds the given vector to this, scaled by the given amount.
	* This is used to update the orientation quaternion by a rotation
	* and time.
	*
	* @param vector The vector to add.
	*
	* @param scale The amount of the vector to add.
	*/
	//------------------------------------------------------------------------------------------
	void AddScaledVector(const Vector3D& vector, float scale)
	{
		Quaternion q(0,
			vector.x * scale,
			vector.y * scale,
			vector.z * scale);
		q *= *this;
		r += q.r * 0.5f;
		i += q.i * 0.5f;
		j += q.j * 0.5f;
		k += q.k * 0.5f;
	}

	//rotates this quaternion by vector
	void RotateByVector(const Vector3D& vector)
	{
		Quaternion q(0, vector.x, vector.y, vector.z);
		(*this) *= q;
	}

	//rotate vector by this quaternion
	Vector3D RotateVectorByQuaternion(Vector3D& vector)
	{
		Vector3D u = { i, j, k };
		float s = r;

		//vector = (u * 2.0f * Vector3D::Dot(u, vector)) + (vector * (s*s - Vector3D::Dot(u, vector))) + (Vector3D::Cross(u, vector) * 2.0f * s);
		Quaternion V(0, vector.x, vector.y, vector.z);
		V = (*this * V * this->Conjugate());
		vector.x = V.i;
		vector.z = V.j;
		vector.z = V.k;
		return vector;
	}

	//converts the quaternion to an axis and an angle in radians
	void toAxisAngle(Vector3D& axis, float& angle)
	{
		if (r > 1)
			Normalize();
		angle = 2 * acosf(r);
		double s = sqrtf(1 - r * r);

		if (s < 0.001)
		{
			axis.x = i;
			axis.y = j;
			axis.z = k;
		}
		else
		{
			axis.x = i / (float)s;
			axis.y = j / (float)s;
			axis.z = k / (float)s;
		}
	}

	//returns the euler angles of the quaternion
	Vector3D EulerAngles(bool homogenous = true)const
	{
		Vector3D euler;

		if (homogenous)
		{
			euler.x = atan2f(2.f * (i*j + k * i), (i*i) - (j*j) - (k*k) + (r*r));
			euler.y = asinf(-2.f * (i*k - j * r));
			euler.z = atan2f(2.f * (j*k + i * r), -(i*i) - (j*j) + (k*k) + (r*r));
		}
		else
		{
			euler.x = atan2f(2.f * (k*j + i * r), 1 - 2 * ((i*i) + (j*j)));
			euler.y = asinf(-2.f * (i*k - j * r));
			euler.z = atan2f(2.f * (i*j + k * r), 1 - 2 * ((j*j) + (k*k)));
		}
		return euler;
	}
};

/**
* Inline function that creates a transform matrix from an orientation.
*/

static inline XMMATRIX CalculateTransformMatrix(const Quaternion & orientation)
{
	XMMATRIX transformMatrix;
	transformMatrix = XMMatrixIdentity();
	transformMatrix.r[0] = XMVectorSetX(transformMatrix.r[0], 1 - 2 * orientation.j*orientation.j - 2 * orientation.k*orientation.k);
	transformMatrix.r[0] = XMVectorSetY(transformMatrix.r[0], 2 * orientation.i*orientation.j - 2 * orientation.k*orientation.r);
	transformMatrix.r[0] = XMVectorSetZ(transformMatrix.r[0], 2 * orientation.i*orientation.k + 2 * orientation.j*orientation.r);
	transformMatrix.r[0] = XMVectorSetW(transformMatrix.r[0], 0.0f);

	transformMatrix.r[1] = XMVectorSetX(transformMatrix.r[1], 2 * orientation.i*orientation.j + 2 * orientation.r*orientation.k);
	transformMatrix.r[1] = XMVectorSetY(transformMatrix.r[1], 1 - 2 * orientation.i*orientation.i - 2 * orientation.k*orientation.k);
	transformMatrix.r[1] = XMVectorSetZ(transformMatrix.r[1], 2 * orientation.j*orientation.k - 2 * orientation.r*orientation.i);
	transformMatrix.r[1] = XMVectorSetW(transformMatrix.r[1], 0.0f);

	transformMatrix.r[2] = XMVectorSetX(transformMatrix.r[2], 2 * orientation.i*orientation.k - 2 * orientation.r*orientation.j);
	transformMatrix.r[2] = XMVectorSetY(transformMatrix.r[2], 2 * orientation.j*orientation.k + 2 * orientation.r*orientation.i);
	transformMatrix.r[2] = XMVectorSetZ(transformMatrix.r[2], 1 - 2 * orientation.i*orientation.i - 2 * orientation.j*orientation.j);
	transformMatrix.r[2] = XMVectorSetW(transformMatrix.r[2], 0.0f);

	transformMatrix.r[3] = XMVectorSetX(transformMatrix.r[3], 0.0f);
	transformMatrix.r[3] = XMVectorSetY(transformMatrix.r[3], 0.0f);
	transformMatrix.r[3] = XMVectorSetZ(transformMatrix.r[3], 0.0f);
	transformMatrix.r[3] = XMVectorSetW(transformMatrix.r[3], 1.0f);

	return transformMatrix;
}