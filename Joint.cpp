#include "Joint.h"

void Joint::CalculateInverseBindTransform(XMFLOAT4X4 parentBindTransform)
{
	XMMATRIX bindTransform = XMMatrixMultiply(XMLoadFloat4x4(&parentBindTransform), XMLoadFloat4x4(&_localBindTransform));

	XMStoreFloat4x4(&_inverseBindTransform, XMMatrixInverse(nullptr, bindTransform));

	for (Joint* child : _children)
	{
		XMFLOAT4X4 nextparentBindTransform;
		XMStoreFloat4x4(&nextparentBindTransform, bindTransform);
		child->CalculateInverseBindTransform(nextparentBindTransform);
	}
}
