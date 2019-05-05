#include "AnimatedModel.h"

AnimatedModel::AnimatedModel(AnimatedModelData modelData, ID3D11ShaderResourceView * textureRV, ID3D11Device * d3dDevice)
	: _textureRV(textureRV)
{
	_geometry = Mesh(modelData.meshData, d3dDevice);

	_rootJoint = CreateJoints(modelData.joints.rootJoint);

	_jointCount = modelData.joints.jointCount;

	_material.ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	_material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	_material.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	_material.specularPower = 0.0f;

	XMFLOAT4X4 identity;
	XMStoreFloat4x4(&identity, XMMatrixIdentity());
	_rootJoint->CalculateInverseBindTransform(identity);

	_transform._rotation = Quaternion(-XM_PIDIV2 - 0.5f, XM_PI, 0.0f);

	_animationPlayRate = 0.8f;
}

AnimatedModel::~AnimatedModel()
{
	if (_rootJoint) delete _rootJoint;
}

void AnimatedModel::DoAnimation(Animation* animation)
{
	_animationTime = 0;

	_currentAnimation = animation;
}

void AnimatedModel::Update(float deltaTime)
{
	_transform.UpdateWorldMatrix();

	if (!_currentAnimation)
	{
		return;
	}

	IncreaseAnimationTime(deltaTime);

	std::map<std::string, XMFLOAT4X4> currentPos = CalculateCurrentAnimationPose();
	XMFLOAT4X4 identity;
	XMStoreFloat4x4(&identity, XMMatrixIdentity());
	ApplyPoseToJoints(currentPos, _rootJoint, identity);
}

void AnimatedModel::GetJointTransforms(XMMATRIX* jointMatrices)
{
	AddJointsToArray(_rootJoint, jointMatrices);
}

void AnimatedModel::Draw(ID3D11DeviceContext * pImmediateContext)
{
	pImmediateContext->IASetVertexBuffers(0, 1, &_geometry._vertexBuffer, &_geometry._vertexBufferStride, &_geometry._vertexBufferOffset);
	pImmediateContext->IASetIndexBuffer(_geometry._indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	pImmediateContext->DrawIndexed(_geometry._numberOfIndices, 0, 0);
}

void AnimatedModel::IncreaseAnimationTime(float deltaTime)
{
	_animationTime += deltaTime * _animationPlayRate;

	if (_animationTime > _currentAnimation->GetLength())
	{
		_animationTime = 0.0f;
	}
}

std::map<std::string, XMFLOAT4X4> AnimatedModel::CalculateCurrentAnimationPose()
{
	KeyFrame previousFrame, nextFrame;

	GetPreviousAndNextFrames(previousFrame, nextFrame);

	float progression = CalculateProgression(previousFrame, nextFrame);

	return InterpolatePoses(previousFrame, nextFrame, progression);
}

void AnimatedModel::ApplyPoseToJoints(std::map<std::string, XMFLOAT4X4> currentPose, Joint* joint, XMFLOAT4X4 parentTransform)
{
	XMFLOAT4X4 currentLocalTransform = currentPose.at(joint->_name);
	XMMATRIX currentTransform = XMMatrixMultiply(XMLoadFloat4x4(&parentTransform), XMLoadFloat4x4(&currentLocalTransform));

	XMFLOAT4X4 currentTransformAsFloats;

	for (Joint* childJoint : joint->_children)
	{
		XMStoreFloat4x4(&currentTransformAsFloats, currentTransform);
		ApplyPoseToJoints(currentPose, childJoint, currentTransformAsFloats);
	}

	currentTransform = XMMatrixMultiply(currentTransform, XMLoadFloat4x4(&joint->GetInverseBindTransform()));

	XMStoreFloat4x4(&currentTransformAsFloats, currentTransform);

	joint->SetAnimationTransform(currentTransformAsFloats);
}

void AnimatedModel::GetPreviousAndNextFrames(KeyFrame & previousFrame, KeyFrame & nextFrame)
{
	previousFrame = _currentAnimation->GetKeyFrames()[0];
	nextFrame = previousFrame;
	for (int i = 1; i < _currentAnimation->GetKeyFrames().size(); i++)
	{
		nextFrame = _currentAnimation->GetKeyFrames()[i];
		if (nextFrame.GetTimeStamp() > _animationTime)
		{
			break;
		}
		previousFrame = _currentAnimation->GetKeyFrames()[i];
	}
}

float AnimatedModel::CalculateProgression(KeyFrame previousFrame, KeyFrame nextFrame)
{
	float totalTime = nextFrame.GetTimeStamp() - previousFrame.GetTimeStamp();
	float currentTime = _animationTime - previousFrame.GetTimeStamp();

	return currentTime / totalTime;
}

std::map<std::string, XMFLOAT4X4> AnimatedModel::InterpolatePoses(KeyFrame previousFrame, KeyFrame nextFrame, float progression)
{
	std::map<std::string, XMFLOAT4X4> currentPose;
	for (auto jointName : previousFrame.GetJointKeyFrames())
	{
		JointTransform previousTransform = previousFrame.GetJointKeyFrames().at(jointName.first);
		JointTransform nextTransform = nextFrame.GetJointKeyFrames().at(jointName.first);
		JointTransform currentTransform = JointTransform::interpolate(previousTransform, nextTransform, progression);
		currentPose[jointName.first] = currentTransform.GetLocalTransform();
	}
	return currentPose;
}

void AnimatedModel::AddJointsToArray(Joint * rootJoint, XMMATRIX * jointMatrices)
{
	jointMatrices[rootJoint->_index] = XMMatrixTranspose(_transform.GetWorldMatrix()) * (_currentAnimation ? XMLoadFloat4x4(&rootJoint->GetAnimatedTransform()) : XMMatrixIdentity());
	for (Joint* childJoint : rootJoint->_children)
	{
		AddJointsToArray(childJoint, jointMatrices);
	}
}

Joint * AnimatedModel::CreateJoints(JointData data)
{
	Joint* joint = new Joint(data.index, data.nameID, data.bindLocalTransform, data.inverseBindTransform);

	for (JointData* child : data.children)
	{
		joint->AddChild(CreateJoints(*child));
	}
	return joint;
}
