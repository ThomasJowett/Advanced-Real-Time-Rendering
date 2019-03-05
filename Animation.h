#pragma once

#include <vector>
#include "KeyFrame.h"

class Animation
{
private:
	float _length;
	std::vector<KeyFrame> _keyFrames;
	

public:
	Animation(float lengthInSeconds, std::vector<KeyFrame> frames)
		:_keyFrames(frames), _length(lengthInSeconds) {}

	float GetLength() const { return _length; }

	std::vector<KeyFrame> GetKeyFrames() { return _keyFrames; }
};