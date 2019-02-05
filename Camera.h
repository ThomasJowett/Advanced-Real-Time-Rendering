#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include "Vector.h"

using namespace DirectX;

class Camera
{
private:
	Vector3D _eye; 
	Vector3D _at;
	Vector3D _up;
	Vector3D _right;
	Vector3D _look;

	FLOAT _windowWidth;
	FLOAT _windowHeight;
	FLOAT _nearDepth;
	FLOAT _farDepth;

	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;

	FLOAT _fovY;

public:
	Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
	~Camera();

	void Update();

	XMFLOAT4X4 GetView() const { return _view; }
	XMFLOAT4X4 GetProjection() const { return _projection; }

	XMFLOAT4X4 GetViewProjection() const;

	XMFLOAT3 GetPosition() const { return XMFLOAT3(_eye.x,_eye.y,_eye.z); }
	XMFLOAT3 GetLookAt() const { return XMFLOAT3(_at.x, _at.y, _at.z); }
	XMFLOAT3 GetUp() const { return XMFLOAT3(_up.x, _up.y, _up.z); }
	
	void SetPosition(XMFLOAT3 position) { _eye = position; }
	void SetLookAt(XMFLOAT3 lookAt) { _at = lookAt; }
	void SetUp(XMFLOAT3 up) { _up = up; }
	void SetLook(XMFLOAT3 look) { _look = look; }
	void SetRight(XMFLOAT3 right) { _right = right; }

	void SetPosition(Vector3D position) { _eye = position; }
	void SetLookAt(Vector3D lookAt) { _at = lookAt; }
	void SetUp(Vector3D up) { _up = up.GetNormalized(); }
	void SetLook(Vector3D look) { _look = look.GetNormalized(); }
	void SetRight(Vector3D right) { _right = right.GetNormalized(); }

	void Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);

	float GetFovY() { return _fovY; }
	float GetFarDepth() { return _farDepth; }
	float GetNearDepth() { return _nearDepth; }

	void MoveForward(float scale);
	void MoveRight(float scale);
	void MoveUp(float scale);

	void Pitch(float angle);
	void Yaw(float angle);
};

