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

#include <float.h>			//for FLT_EPSILON
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
		r = cos_z_2 * cos_y_2 * cos_x_2 + sin_z_2 * sin_y_2 * sin_x_2;
		i = cos_z_2 * cos_y_2 * sin_x_2 - sin_z_2 * sin_y_2 * cos_x_2;
		j = cos_z_2 * sin_y_2 * cos_x_2 + sin_z_2 * cos_y_2 * sin_x_2;
		k = sin_z_2 * cos_y_2 * cos_x_2 - cos_z_2 * sin_y_2 * sin_x_2;
	}

	//from 3 euler angles in radians
	Quaternion(const float theta_Roll, float theta_Pitch, float theta_Yaw)
	{
		(*this) = Quaternion(Vector3D(theta_Roll, theta_Pitch, theta_Yaw));
	}

	//from matrix
	Quaternion(XMFLOAT4X4 matrix)
	{
		float diagonal = matrix(0, 0) + matrix(1, 1) + matrix(2, 2);
		if (diagonal > 0)
		{
			float r4 = (float)(sqrt(diagonal + 1.0f) * 2.0f);
			r = r4 / 4.0f;
			i = matrix(2, 1) - matrix(1, 2) / r4;
			j = matrix(0, 2) - matrix(2, 0) / r4;
			k = matrix(1, 0) - matrix(0, 1) / r4;
		}
		else if (matrix(0, 0) > matrix(1, 1) && matrix(0, 0) > matrix(2, 2))
		{
			float i4 = (float)sqrt(1.0f + matrix(0, 0) - matrix(1, 1) - matrix(2, 2) * 2.0f);
			r = (matrix(2, 1) - matrix(1, 2)) / i4;
			i = i4 / 4.0f;
			j = (matrix(0,1) + matrix(1,0)) / i4;
			k = (matrix(0,2) + matrix(2,0)) / i4;
		}
		else if (matrix(1,1) > matrix(2,2)) {
			float j4 = (float)(sqrt(1.0f + matrix(1,1) - matrix(0,0) - matrix(2,2)) * 2.0f);
			r = (matrix(0,2) - matrix(2,0)) / j4;
			i = (matrix(0,1) + matrix(1,0)) / j4;
			j = j4 / 4.0f;					
			k = (matrix(1,2) + matrix(2,1)) / j4;
		}
		else {
			float k4 = (float)(sqrt(1.0f + matrix(2,2) - matrix(0,0) - matrix(1,1)) * 2.0f);
			r = (matrix(1,0) - matrix(0,1)) / k4;
			i = (matrix(0,2) + matrix(2,0)) / k4;
			j = (matrix(1,2) + matrix(2,1)) / k4;
			k = k4 / 4.0f;
		}

		//Normalize();
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

	//Static------------------------------------------------------------------------------------

	//sums the product of q1 and q2
	float static Dot(Quaternion q1, Quaternion q2)
	{
		return q1.r * q2.r + q1.i * q2.i + q1.j * q2.j + q1.k * q2.k;
	}

	//Spherically interpolates between a and b
	Quaternion static Slerp(Quaternion a, Quaternion b, float t)
	{
		Quaternion result = Quaternion();

		//calculate angle between them
		float cosHalfTheta = Dot(a, b);

		//if a==b or a==-b then theta = 0 and we can return a
		if (abs(cosHalfTheta) >= 1.0)
		{
			result = a;
			return result;
		}
		
		//calculate temporary values
		float halfTheta = acos(cosHalfTheta);
		float sinHalfTheta = (float)sqrt(1.0 - cosHalfTheta * cosHalfTheta);
		
		//if theta = 180 degrees then result is not fully defined
		//we could rotate around any axis normal to a or b
		
		if (fabs(sinHalfTheta) < 0.001)
		{
			result.r = (a.r * 0.5f + b.r * 0.5f);
			result.i = (a.i * 0.5f + b.i * 0.5f);
			result.j = (a.j * 0.5f + b.j * 0.5f);
			result.k = (a.k * 0.5f + b.k * 0.5f);
			return result;
		}
		
		float ratioA = (float)sin((1 - t)*halfTheta) / sinHalfTheta;
		float ratioB = (float)sin(t *halfTheta) / sinHalfTheta;
		
		//calculate Quaternion
		result.r = (a.r * ratioA + b.r * ratioB);
		result.i = (a.i * ratioA + b.i * ratioB);
		result.j = (a.j * ratioA + b.j * ratioB);
		result.k = (a.k * ratioA + b.k * ratioB);

		return result;
	}

	//Normally interpolates between a and b
	Quaternion static Nlerp(Quaternion a, Quaternion b, float alpha)
	{
		Quaternion result = Quaternion();

		float dot = Dot(a, b);

		float oneMinusAlpha = 1.0f - alpha;

		if (dot < 0) {
			result.r = oneMinusAlpha * a.r + alpha * -b.r;
			result.i = oneMinusAlpha * a.i + alpha * -b.i;
			result.j = oneMinusAlpha * a.j + alpha * -b.j;
			result.k = oneMinusAlpha * a.k + alpha * -b.k;
		}
		else {
			result.r = oneMinusAlpha * a.r + alpha * b.r;
			result.i = oneMinusAlpha * a.i + alpha * b.i;
			result.j = oneMinusAlpha * a.j + alpha * b.j;
			result.k = oneMinusAlpha * a.k + alpha * b.k;
		}
		result.Normalize();
		return result;
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

		vector = (u * 2.0f * Vector3D::Dot(u, vector)) + (vector * (s*s - Vector3D::Dot(u, vector))) + (Vector3D::Cross(u, vector) * 2.0f * s);
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