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
	XMMATRIX _animatedTransform;

	XMMATRIX _localBindTransform;
	XMMATRIX _inverseBindTransform;

public:
	Joint(int index, std::string name, XMMATRIX bindLocalTransform)
		:_index(index), _name(name), _localBindTransform(bindLocalTransform)
	{
		_animatedTransform = XMMatrixIdentity();
	}

	void AddChild(Joint* child)
	{
		_children.push_back(child);
	}

	XMMATRIX GetAnimatedTransform() const
	{
		return _animatedTransform;
	}

	void SetAnimationTransform(XMMATRIX animationTransform)
	{
		_animatedTransform = animationTransform;
	}

	XMMATRIX GetInverseBindTransform() const
	{
		return _inverseBindTransform;
	}

	void CalculateInverseBindTransform(XMMATRIX parentBindTransform);
};
