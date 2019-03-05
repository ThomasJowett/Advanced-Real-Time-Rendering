#include "Joint.h"

void Joint::CalculateInverseBindTransform(XMMATRIX parentBindTransform)
{
	XMMATRIX bindTransform = XMMatrixMultiply(parentBindTransform, _localBindTransform);

	_inverseBindTransform = XMMatrixInverse(nullptr, bindTransform);
	for (Joint* child : _children)
	{
		child->CalculateInverseBindTransform(bindTransform);
	}
}
