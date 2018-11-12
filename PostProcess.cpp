#include "PostProcess.h"

PostProcessConstantBuffer PostProcess::GaussianBlur(float multiplier, int screenHeight, int screenWidth)
{
	float tu = 1.0f / float(screenWidth);
	float tv = 1.0f / float(screenHeight);

	float totalWeight = 0.0f;

	size_t index = 0;

	PostProcessConstantBuffer cb;
	auto offsets = reinterpret_cast<XMFLOAT4*>(cb.sampleOffsets);
	auto weights = cb.sampleWeights;

	for (int x = -2; x <= 2; ++x)
	{
		for (int y = -2; y <= 2; ++y)
		{
			// Exclude pixels with a block distance greater than 2. This will
			// create a kernel which approximates a 5x5 kernel using only 13
			// sample points instead of 25; this is necessary since 2.0 shaders
			// only support 16 texture grabs.
			if (fabs(float(x)) + fabs(float(y)) > 2.0f)
				continue;

			// Get the unscaled Gaussian intensity for this offset
			
			offsets[index].x = float(x) * tu;
			offsets[index].y = float(y) * tv;
			offsets[index].z = 0.0f;
			offsets[index].w = 0.0f;

			float g = GaussianDistribution(float(x), float(y), 1.0f);
			weights[index] = XMVectorReplicate(g);

			totalWeight += XMVectorGetX(weights[index]);

			++index;
		}
	}

	// Divide the current weight by the total weight of all the samples; Gaussian
	// blur kernels add to 1.0f to ensure that the intensity of the image isn't
	// changed when the blur occurs. An optional multiplier variable is used to
	// add or remove image intensity during the blur.
	XMVECTOR vtw = XMVectorReplicate(totalWeight);
	XMVECTOR vm = XMVectorReplicate(multiplier);
	for (size_t i = 0; i < index; ++i)
	{
		weights[i] = XMVectorDivide(weights[i], vtw);
		weights[i] = XMVectorMultiply(weights[i], vm);
	}

	return cb;
}

PostProcessConstantBuffer PostProcess::Bloom(bool horizontal, float size, float brightness, int screenHeight, int screenWidth)
{
	PostProcessConstantBuffer cb;

	float tu = 0.f;
	float tv = 0.f;
	if (horizontal)
	{
		tu = 1.f / float(screenWidth);
	}
	else
	{
		tv = 1.f / float(screenHeight);
	}

	auto weights = reinterpret_cast<XMFLOAT4*>(cb.sampleWeights);
	auto offsets = reinterpret_cast<XMFLOAT4*>(cb.sampleOffsets);

	// Fill the center texel
	float weight = brightness * GaussianDistribution(0, 0, size);
	weights[0] = XMFLOAT4(weight, weight, weight, 1.0f);
	offsets[0].x = offsets[0].y = offsets[0].z = offsets[0].w = 0.f;

	// Fill the first half
	for (int i = 1; i < 8; ++i)
	{
		// Get the Gaussian intensity for this offset
		weight = brightness * GaussianDistribution(float(i), 0, size);
		weights[i] = XMFLOAT4(weight, weight, weight, 1.0f);
		offsets[i] = XMFLOAT4(float(i) * tu, float(i) * tv, 0.f, 0.f);
	}

	// Mirror to the second half
	for (int i = 8; i < 15; i++)
	{
		weights[i] = weights[i - 7];
		offsets[i] = XMFLOAT4(-offsets[i - 7].x, -offsets[i - 7].y, 0.f, 0.f);
	}

	return cb;
}
