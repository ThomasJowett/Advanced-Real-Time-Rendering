#pragma once

#include "Quaternion.h"
#include "Vector.h"

class JointTransform
{
public:
	Vector3D _position;
	Quaternion _rotation;

public:
	JointTransform(Vector3D position, Quaternion rotation)
		:_position(position), _rotation(rotation) {}

	JointTransform()
	{
		_position = Vector3D(0.0f, 0.0f, 0.0f);
		_rotation = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
	}

	XMMATRIX GetLocalTransform()
	{
		XMMATRIX transform = XMMatrixIdentity();
		XMMATRIX translation = XMMatrixTranslation(_position.x, _position.y, _position.z);
		transform = translation * CalculateTransformMatrix(_rotation);

		XMStoreFloat4x4(&_localTransform, transform);
		return transform;
	}

	static JointTransform interpolate(JointTransform frameA, JointTransform frameB, float alpha)
	{
		Vector3D pos = Vector3D::Lerp(frameA._position, frameB._position, alpha);
		Quaternion rot = Quaternion::Slerp(frameA._rotation, frameB._rotation, alpha);

		return JointTransform(pos, rot);
	}
private:
	XMFLOAT4X4 _localTransform;
};