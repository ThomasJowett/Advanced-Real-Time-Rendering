#pragma once
#include "Commons.h"

namespace GeometryGenerator
{
	IndexedModel CreateCube(float width, float height, float depth);

	IndexedModel CreateSphere(float radius, unsigned int longitudeLines, unsigned int latitudeLines);

	IndexedModel CreateGrid(float width, float length, unsigned int widthLines, unsigned int lengthLines, float tileU, float tileV);

	IndexedModel CreateFullScreenQuad();

	IndexedModel CreateCylinder(float bottomRadius, float topRadius, float height, int sliceCount, int stackCount);

	IndexedModel CreateTorus(float diameter, float thickness, int segments);
}
