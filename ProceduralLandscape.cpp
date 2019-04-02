#include "ProceduralLandscape.h"
#include "noise/noise.h"
#include <fstream>

using namespace noise;

std::vector<float> ProceduralLandscape::GenerateHeightMap()
{
	//module::Perlin myModule;

	//utils::NoiseMap heightMap;
	//utils::NoiseMapBuilderPlane heightMapBuilder;
	//heightMapBuilder.SetSourceModule(myModule);
	//heightMapBuilder.SetDestNoiseMap(heightMap);
	//heightMapBuilder.SetDestSize(256, 256);

	return std::vector<float>();
}

std::vector<float> ProceduralLandscape::LoadHeightMap(const char * filename, unsigned int width, unsigned int height, float HeightScale)
{
	std::vector<float> heightMap;

	//A height for each vertex
	std::vector<unsigned char> in(width * height);

	//open the file
	std::ifstream inFile;
	inFile.open(filename, std::ios_base::binary);

	if (inFile)
	{
		//Read the Raw bytes
		inFile.read((char*)&in[0], (std::streamsize)in.size());

		inFile.close();
	}

	//Copy the array data into a float array and scale it.
	heightMap.resize(height * width, 0);

	for (unsigned int i = 0; i < height * width; ++i)
	{
		heightMap[i] = (in[i] / 255.0f) * HeightScale;
	}

	return heightMap;
}
