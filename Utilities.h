#pragma once

#include <directxmath.h>

using namespace DirectX;

namespace Util
{
	void ExtractFrustumPlanes(XMFLOAT4 planes[6], XMFLOAT4X4 matrix);
}