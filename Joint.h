#pragma once

#include <string>
#include <vector>
#include <directxmath.h>

using namespace DirectX;

class Joint
{
public:
	int _index;
	std::string _name;
	std::vector<Joint*> _children;

private:
	XMFLOAT4X4 _animatedTransform;

	XMFLOAT4X4 _localBindTransform;
	XMFLOAT4X4 _inverseBindTransform;

public:
	Joint(int index, std::string name, XMFLOAT4X4 bindLocalTransform)
		:_index(index), _name(name), _localBindTransform(bindLocalTransform)
	{
		//_localBindTransform = XMMatrixIdentity();
		//_animatedTransform = XMMatrixIdentity();
	}

	~Joint()
	{
		for (Joint* child : _children)
		{
			delete child;
		}
	}

	void AddChild(Joint* child)
	{
		_children.push_back(child);
	}

	XMFLOAT4X4 GetAnimatedTransform() const
	{
		return _animatedTransform;
	}

	void SetAnimationTransform(XMFLOAT4X4 animationTransform)
	{
		_animatedTransform = animationTransform;
	}

	XMFLOAT4X4 GetInverseBindTransform() const
	{
		return _inverseBindTransform;
	}

	void CalculateInverseBindTransform(XMFLOAT4X4 parentBindTransform);
};
