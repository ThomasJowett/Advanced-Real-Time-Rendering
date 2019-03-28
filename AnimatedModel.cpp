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
	if (!_currentAnimation)
	{
		return;
	}

	IncreaseAnimationTime(deltaTime);

	std::map<std::string, XMMATRIX> currentPos = CalculateCurrentAnimationPose();
	ApplyPoseToJoints(currentPos, _rootJoint, XMMATRIX());
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
	_animationTime += deltaTime;

	if (_animationTime > _currentAnimation->GetLength())
	{
		_animationTime = _currentAnimation->GetLength();
	}
}

std::map<std::string, XMMATRIX> AnimatedModel::CalculateCurrentAnimationPose()
{
	KeyFrame previousFrame, nextFrame;

	GetPreviousAndNextFrames(previousFrame, nextFrame);

	float progression = CalculateProgression(previousFrame, nextFrame);

	return InterpolatePoses(previousFrame, nextFrame, progression);
}

void AnimatedModel::ApplyPoseToJoints(std::map<std::string, XMMATRIX> currentPose, Joint* joint, XMMATRIX parentTransform)
{
	XMMATRIX currentLocalTransform = currentPose.at(joint->_name);
	XMMATRIX currentTransform = XMMatrixMultiply(parentTransform, currentLocalTransform);

	for (Joint* childJoint : joint->_children)
	{
		ApplyPoseToJoints(currentPose, childJoint, currentTransform);
	}

	currentTransform = XMMatrixMultiply(currentTransform, XMLoadFloat4x4(&joint->GetInverseBindTransform()));

	XMFLOAT4X4 currentTransformAsFloats;

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

std::map<std::string, XMMATRIX> AnimatedModel::InterpolatePoses(KeyFrame previousFrame, KeyFrame nextFrame, float progression)
{
	std::map<std::string, XMMATRIX> currentPose;
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
	jointMatrices[rootJoint->_index] = _currentAnimation ? XMLoadFloat4x4(&rootJoint->GetAnimatedTransform()) : XMMatrixIdentity();
	for (Joint* childJoint : rootJoint->_children)
	{
		AddJointsToArray(childJoint, jointMatrices);
	}
}

Joint * AnimatedModel::CreateJoints(JointData data)
{
	Joint* joint = new Joint(data.index, data.nameID, data.bindLocalTransform);

	for (JointData* child : data.children)
	{
		joint->AddChild(CreateJoints(*child));
	}
	return joint;
}
