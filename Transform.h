#pragma once
#include "Vector.h"
#include "Quaternion.h"

class Transform
{
public:
	Transform()
	{
		_position = Vector3D(0.0f, 0.0f, 0.0f);
		_rotation = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
		_scale = Vector3D(1.0f, 1.0f, 1.0f);
	}
	Transform(Vector3D position, Quaternion rotation, Vector3D scale)
		:_position(position), _rotation(rotation), _scale(scale) {}
	~Transform() = default;

	Vector3D _position;
	Quaternion _rotation;
	Vector3D _scale;

	void SetWorldMatrix4x4(XMFLOAT4X4 world) { _world = world; }
	XMMATRIX GetWorldMatrix() const { return XMLoadFloat4x4(&_world); }
	XMFLOAT4X4 GetWorldMatrix4x4() const { return _world; }

	void UpdateWorldMatrix()
	{
		XMMATRIX scale = XMMatrixScaling(_scale.x, _scale.y, _scale.z);
		XMMATRIX translation = XMMatrixTranslation(_position.x, _position.y, _position.z);
		XMStoreFloat4x4(&_world, scale * CalculateTransformMatrix(_rotation) * translation);
	}

private:
	XMFLOAT4X4 _world;
};