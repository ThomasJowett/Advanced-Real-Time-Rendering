#pragma once

#include "Mesh.h"
#include "Joint.h"
#include "Animation.h"

#include<map>
#include<string>

#include "GameObject.h"//just for material

class AnimatedModel
{
private:
	Mesh _geometry; //Posibily need to be a skeletal mesh class

	ID3D11ShaderResourceView * _textureRV;

	Material _material;

	Joint* _rootJoint = nullptr;
	int _jointCount;

	Animation* _currentAnimation = nullptr;
	float _animationTime = 0;

public:
	AnimatedModel(AnimatedModelData modelData, ID3D11ShaderResourceView* textureRV, ID3D11Device * d3dDevice);
	~AnimatedModel();

	void DoAnimation(Animation* animation);

	void Update(float deltaTime);

	XMMATRIX* GetJointTransforms();

	void Draw(ID3D11DeviceContext* pImmediateContext);

	ID3D11ShaderResourceView *GetTextureRV() { return _textureRV; }

	Material GetMaterial() { return _material; }
private:

	void IncreaseAnimationTime(float deltaTime);

	std::map<std::string, XMMATRIX> CalculateCurrentAnimationPose();

	void ApplyPoseToJoints(std::map<std::string, XMMATRIX> currentPose, Joint* joint, XMMATRIX parentTransform);

	void GetPreviousAndNextFrames(KeyFrame &previousFrame, KeyFrame &nextFrame);

	float CalculateProgression(KeyFrame previousFrame, KeyFrame nextFrame);

	std::map<std::string, XMMATRIX> InterpolatePoses(KeyFrame previousFrame, KeyFrame NextFrame, float progression);

	void AddJointsToArray(Joint* rootJoint, XMMATRIX* jointMatirces);

	Joint* CreateJoints(JointData data);
};