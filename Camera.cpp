#include "Camera.h"

Camera::Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
	: _eye(position), _at(at), _up(up), _windowWidth(windowWidth), _windowHeight(windowHeight), _nearDepth(nearDepth), _farDepth(farDepth)
{
	SetLook((_at - _eye).GetNormalized());
	SetRight((Vector3D::Cross(_up, _look)).GetNormalized());

	Update();

	_fovY = 0.25f * XM_PI;	
}

Camera::~Camera()
{
}

void Camera::Update()
{
	_up = Vector3D::Cross(_look, _right).GetNormalized();
	_right = Vector3D::Cross(_up, _look).GetNormalized();

    // Initialize the view matrix

	XMFLOAT4 eye = XMFLOAT4(_eye.x, _eye.y, _eye.z, 1.0f);
	XMFLOAT4 up = XMFLOAT4(_up.x, _up.y, _up.z, 0.0f);
	XMFLOAT4 look = XMFLOAT4(_look.x, _look.y, _look.z, 0.0f);
	XMFLOAT4 at = XMFLOAT4(eye.x + look.x, eye.y + look.y, eye.z + look.z, 0.0f);


	XMVECTOR EyeVector = XMLoadFloat4(&eye);
	XMVECTOR AtVector = XMLoadFloat4(&at);
	XMVECTOR UpVector = XMLoadFloat4(&up);

	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(EyeVector, AtVector, UpVector));

    // Initialize the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(_fovY, _windowWidth / _windowHeight, _nearDepth, _farDepth));
}

void Camera::Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
{
	_windowWidth = windowWidth;
	_windowHeight = windowHeight;
	_nearDepth = nearDepth;
	_farDepth = farDepth;
}

void Camera::MoveForward(float scale)
{
	/*Position += scale * look*/
	Vector3D s = Vector3D(scale, scale, scale);
	SetPosition((s * _look) + _eye);
}

void Camera::MoveRight(float scale)
{
	/*Position += scale * right*/
	Vector3D s = Vector3D(scale, scale, scale);
	SetPosition((s * _right) + _eye);
}

void Camera::MoveUp(float scale)
{
	/*Position += scale * right*/
	Vector3D s = Vector3D(scale, scale, scale);
	SetPosition((s*_up) + _eye);
}

void Camera::Pitch(float angle)
{
	XMVECTOR right = { _right.x, _right.y, _right.z };
	XMMATRIX R = XMMatrixRotationAxis(right, angle);

	XMFLOAT3 up = { GetUp().x, GetUp().y, GetUp().z };
	XMFLOAT3 look = { _look.x, _look.y, _look.z };

	XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), R));
	XMStoreFloat3(&look, XMVector3TransformNormal(XMLoadFloat3(&look), R));

	SetUp(Vector3D( up.x, up.y, up.z ));
	SetLook(Vector3D(look.x, look.y, look.z ));
}

void Camera::Yaw(float angle)
{
	XMMATRIX R = XMMatrixRotationY(angle);

	XMFLOAT3 right = { _right.x, _right.y, _right.z };
	XMFLOAT3 look = { _look.x, _look.y, _look.z };

	XMStoreFloat3(&right, XMVector3TransformNormal(XMLoadFloat3(&right), R));
	XMStoreFloat3(&look, XMVector3TransformNormal(XMLoadFloat3(&look), R));

	SetLook(Vector3D( look.x, look.y, look.z ));
	SetRight(Vector3D( right.x, right.y, right.z ));
}

XMFLOAT4X4 Camera::GetViewProjection() const 
{ 
	XMMATRIX view = XMLoadFloat4x4(&_view);
	XMMATRIX projection = XMLoadFloat4x4(&_projection);

	XMFLOAT4X4 viewProj;

	XMStoreFloat4x4(&viewProj, view * projection);

	return viewProj;
}