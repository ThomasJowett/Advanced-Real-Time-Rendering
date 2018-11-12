#include "GameObject.h"

GameObject::GameObject(string type, Transform* transform, Mesh geometry, Material material) 
	: _transform (transform), _geometry(geometry), _type(type), _material(material)
{
	_parent = nullptr;
	
	_textureRV[0] = { nullptr};
	_textureRV[1] = { nullptr };
	_textureRV[2] = { nullptr };
}

GameObject::~GameObject()
{
	delete _transform;
}

void GameObject::Update(float deltaTime)
{
	_transform->UpdateWorldMatrix();

	if (_parent != nullptr)
	{
		XMStoreFloat4x4(&_transform->GetWorldMatrix4x4(), this->_transform->GetWorldMatrix() * _parent->_transform->GetWorldMatrix());
	}
}

void GameObject::Draw(ID3D11DeviceContext * pImmediateContext)
{
	// NOTE: We are assuming that the constant buffers and all other draw setup has already taken place

	// Set vertex and index buffers
	pImmediateContext->IASetVertexBuffers(0, 1, &_geometry._vertexBuffer, &_geometry._vertexBufferStride, &_geometry._vertexBufferOffset);
	pImmediateContext->IASetIndexBuffer(_geometry._indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	pImmediateContext->DrawIndexed(_geometry._numberOfIndices, 0, 0);
}
