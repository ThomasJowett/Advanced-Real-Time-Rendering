#pragma once

#include "Mesh.h"
#include "Joint.h"
#include "Animation.h"

#include<map>
#include<string>

class AnimatedModel
{
private:
	Mesh _geometry; //Posibily need to be a skeletal mesh class

	ID3D11ShaderResourceView * _textureRV;

	Joint* _rootJoint;
	int _jointCount;

	Animation* _currentAnimation;
	float _animationTime = 0;

public:
	AnimatedModel(Mesh mesh, ID3D11ShaderResourceView* textureRV, Joint* rootJoint, int jointCount);
	~AnimatedModel();

	void DoAnimation(Animation* animation);

	void Update(float deltaTime);

	XMMATRIX GetJointTransforms();

private:

	void IncreaseAnimationTime(float deltaTime);

	std::map<std::string, XMMATRIX> CalculateCurrentAnimationPose();

	void ApplyPoseToJoints(std::map<std::string, XMMATRIX> currentPose, Joint* joint, XMMATRIX parentTransform);

	void GetPreviousAndNextFrames(KeyFrame &previousFrame, KeyFrame &nextFrame);

	float CalculateProgression(KeyFrame previousFrame, KeyFrame nextFrame);

	std::map<std::string, XMMATRIX> InterpolatePoses(KeyFrame previousFrame, KeyFrame NextFrame, float progression);

	void AddJointsToArray(Joint* headJoint, std::vector<XMMATRIX> jointMatirces);
};