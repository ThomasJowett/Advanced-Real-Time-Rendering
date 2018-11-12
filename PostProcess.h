#pragma once
#include "Commons.h"

namespace PostProcess
{
	PostProcessConstantBuffer GaussianBlur(float multiplier, int screenHeight, int screenWidth);

	PostProcessConstantBuffer Bloom(bool horizontal, float size, float brightness, int screenHeight, int screenWidth);

	inline float GaussianDistribution(float x, float y, float rho)
	{
		return expf(-(x * x + y * y) / (2 * rho * rho)) / sqrtf(2 * XM_PI * rho * rho);
	}
}