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
	float GetTimeStamp() { return _timeStamp; }

	std::map<std::string, JointTransform> GetJointKeyFrames() 
	{
		return _pose;
	}
};

