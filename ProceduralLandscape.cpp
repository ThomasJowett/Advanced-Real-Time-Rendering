#include "ProceduralLandscape.h"

std::vector<float> ProceduralLandscape::GenerateHeightMap()
{
	module::Perlin myModule;

	utils::NoiseMap heightMap;
	utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(myModule);
	heightMapBuilder.SetDestNoiseMap(heightMap);
	heightMapBuilder.SetDestSize(256, 256);

	return std::vector<float>();
}
