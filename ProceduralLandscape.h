#pragma once

//#include "noiseutils.h"
#include <vector>



namespace ProceduralLandscape
{
	std::vector<float> GenerateHeightMap();
	std::vector<float> LoadHeightMap(const char* filename, unsigned int width, unsigned int height, float HeightScale);
}