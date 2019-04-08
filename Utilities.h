#pragma once
#include <cstdlib>

#include <directxmath.h>

using namespace DirectX;

namespace Util
{
	void ExtractFrustumPlanes(XMFLOAT4 planes[6], XMFLOAT4X4 matrix);
}

namespace Random
{
	static int IntInRange2D(int x, int y, int seed, int min, int max)
	{
		unsigned int hash = (unsigned int)seed;
		hash ^= (unsigned int)x;
		hash *= 0x51d7348d;
		hash ^= 0x85dbdda2;
		hash = (hash << 16) ^ (hash >> 16);
		hash *= 0x7588f287;
		hash ^= (unsigned int)y;
		hash *= 0x487a5559;
		hash ^= 0x64887219;
		hash = (hash << 16) ^ (hash >> 16);
		hash *= 0x63288691;
		return (int)((hash % (max - min)) + min);
	}

	static float FloatInRange(float min, float max)
	{
		return (max - min) * (float)rand() / (RAND_MAX)+min;
	}

	static int IntInRange(int min, int max)
	{
		return (max - min) * rand() / (RAND_MAX)+min;
	}
}