#include "Animation.h"

Animation::Animation(AnimationData animationData)
{
	_length = animationData.lengthSeconds;

	
	_keyFrames.resize(animationData.keyframes.size());

	for (int i = 0; i < _keyFrames.size(); i++)
	{
		_keyFrames[i] = CreateKeyFrame(animationData.keyframes[i]);
	}
}

KeyFrame Animation::CreateKeyFrame(KeyFrameData data)
{
	std::map<std::string, JointTransform> map = std::map<std::string, JointTransform>();

	for (auto jointData : data.jointTransforms)
	{
		Vector3D translation = Vector3D(jointData.second._14, jointData.second._24, jointData.second._34);

		//Vector3D translation = Vector3D();

		Quaternion rotation = Quaternion(jointData.second);

		//Quaternion rotation = Quaternion();

		JointTransform jointTransform  = JointTransform(translation, rotation);

		map.insert(std::pair<std::string, JointTransform>(jointData.first, jointTransform));
	}
	
	return KeyFrame(data.time, map);
}
