#include "AnimatedModel.h"

AnimatedModel::AnimatedModel(Mesh geometry, ID3D11ShaderResourceView * textureRV, Joint* rootJoint, int jointCount)
	:_geometry(geometry), _textureRV(textureRV), _rootJoint(rootJoint), _jointCount(jointCount)
{
}

AnimatedModel::~AnimatedModel()
{

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

XMMATRIX AnimatedModel::GetJointTransforms()
{
	std::vector<XMMATRIX> jointMatrices;
	jointMatrices.resize(_jointCount);
	AddJointsToArray(_rootJoint, jointMatrices);
	return XMMATRIX();
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

	currentTransform = XMMatrixMultiply(currentTransform, joint->GetInverseBindTransform());

	joint->SetAnimationTransform(currentTransform);
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

void AnimatedModel::AddJointsToArray(Joint* headJoint, std::vector<XMMATRIX> jointMatirces)
{
	jointMatirces[headJoint->_index] = headJoint->GetAnimatedTransform();
	for (Joint* childJoint : headJoint->_children)
	{
		AddJointsToArray(childJoint, jointMatirces);
	}
}
