#include "Utilities.h"

void Util::ExtractFrustumPlanes(XMFLOAT4 planes[6], XMFLOAT4X4 matrix)
{
	//
	// Left
	//
	planes[0].x = matrix(0, 3) + matrix(0, 0);
	planes[0].y = matrix(1, 3) + matrix(1, 0);
	planes[0].z = matrix(2, 3) + matrix(2, 0);
	planes[0].w = matrix(3, 3) + matrix(3, 0);

	//
	// Right
	//
	planes[1].x = matrix(0, 3) - matrix(0, 0);
	planes[1].y = matrix(1, 3) - matrix(1, 0);
	planes[1].z = matrix(2, 3) - matrix(2, 0);
	planes[1].w = matrix(3, 3) - matrix(3, 0);

	//
	// Bottom
	//
	planes[2].x = matrix(0, 3) + matrix(0, 1);
	planes[2].y = matrix(1, 3) + matrix(1, 1);
	planes[2].z = matrix(2, 3) + matrix(2, 1);
	planes[2].w = matrix(3, 3) + matrix(3, 1);

	//
	// Top
	//
	planes[3].x = matrix(0, 3) - matrix(0, 1);
	planes[3].y = matrix(1, 3) - matrix(1, 1);
	planes[3].z = matrix(2, 3) - matrix(2, 1);
	planes[3].w = matrix(3, 3) - matrix(3, 1);

	//
	// Near
	//
	planes[4].x = matrix(0, 2);
	planes[4].y = matrix(1, 2);
	planes[4].z = matrix(2, 2);
	planes[4].w = matrix(3, 2);

	//
	// Far
	//
	planes[5].x = matrix(0, 3) - matrix(0, 2);
	planes[5].y = matrix(1, 3) - matrix(1, 2);
	planes[5].z = matrix(2, 3) - matrix(2, 2);
	planes[5].w = matrix(3, 3) - matrix(3, 2);

	// Normalize the plane equations.
	for (int i = 0; i < 6; ++i)
	{
		XMVECTOR v = XMPlaneNormalize(XMLoadFloat4(&planes[i]));
		XMStoreFloat4(&planes[i], v);
	}
}
