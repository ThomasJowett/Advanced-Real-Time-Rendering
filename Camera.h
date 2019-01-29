#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>

using namespace DirectX;

class Camera
{
private:
	XMFLOAT3 _eye; 
	XMFLOAT3 _at;
	XMFLOAT3 _up;
	XMFLOAT3 _right;
	XMFLOAT3 _forward;

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

	XMFLOAT3 GetPosition() const { return _eye; }
	XMFLOAT3 GetLookAt() const { return _at; }
	XMFLOAT3 GetUp() const { return _up; }
	
	void SetPosition(XMFLOAT3 position) { _eye = position; }
	void SetLookAt(XMFLOAT3 lookAt) { _at = lookAt; }
	void SetUp(XMFLOAT3 up) { _up = up; }
	void SetRight(XMFLOAT3 right) { _right = right; }

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

