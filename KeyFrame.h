#pragma once
#include <map>
#include <string>

#include "JointTransform.h"

class KeyFrame
{
private:
	float _timeStamp;
	std::map<std::string, JointTransform> _pose;

public:
	KeyFrame()
	{
		_timeStamp = 0.0f;
		_pose = std::map<std::string, JointTransform>();
	}
	KeyFrame(float time, std::map<std::string, JointTransform> map)
		:_timeStamp(time), _pose(map) {}

	float GetTimeStamp() { return _timeStamp; }

	std::map<std::string, JointTransform> GetJointKeyFrames() 
	{
		return _pose;
	}
};

