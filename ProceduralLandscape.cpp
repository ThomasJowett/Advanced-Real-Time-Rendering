#include "ProceduralLandscape.h"
#include "noise/noise.h"
#include <fstream>
#include "Utilities.h"

using namespace noise;

void DiamondSquareStep(float** array, int chunk_x, int chunk_z, int size);
void SquareStep(float** array, int chunk_x, int chunk_z, int x, int z, int reach);
void DiamondStep(float** array, int chunk_x, int chunk_z, int x, int z, int reach);

std::vector<float> ProceduralLandscape::PerlinNoise()
{
	//module::Perlin myModule;

	//utils::NoiseMap heightMap;
	//utils::NoiseMapBuilderPlane heightMapBuilder;
	//heightMapBuilder.SetSourceModule(myModule);
	//heightMapBuilder.SetDestNoiseMap(heightMap);
	//heightMapBuilder.SetDestSize(256, 256);

	return std::vector<float>();
}


std::vector<float> ProceduralLandscape::DiamondSquare(Terrain::InitInfo tii)
{
	//initialise float array
	float** array = new float*[tii.HeightMapHeight];

	for (int i = 0; i < tii.HeightMapWidth; i++)
	{
		array[i] = new float[tii.HeightMapWidth];

		for (int j = 0; j < tii.HeightMapWidth; j++)
		{
			array[i][j] = Random::FloatInRange(0.0f, tii.HeightScale);
		}
	}

	//define the first four corners based on heightmap scale

	array[0][0] = Random::FloatInRange(0.0f, tii.HeightScale);
	array[0][tii.HeightMapHeight-1] = Random::FloatInRange(0.0f, tii.HeightScale);
	array[tii.HeightMapWidth-1][0] = Random::FloatInRange(0.0f, tii.HeightScale);
	array[tii.HeightMapWidth-1][tii.HeightMapHeight-1] = Random::FloatInRange(0.0f, tii.HeightScale);

	DiamondSquareStep(array, tii.HeightMapWidth, tii.HeightMapHeight, tii.HeightMapHeight / 2);

	//convert float array into float vector

	std::vector<float> returnHeightMap;

	for (int i = 0; i < tii.HeightMapHeight; i++)
	{
		for (int j = 0; j < tii.HeightMapWidth; j++)
		{
			returnHeightMap.push_back(array[i][j]);
		}
	}
	return returnHeightMap;
}

std::vector<float> ProceduralLandscape::FaultLine(Terrain::InitInfo tii)
{
	//initialise float array
	float** array = new float*[tii.HeightMapHeight];

	for (int i = 0; i < tii.HeightMapWidth; i++)
	{
		array[i] = new float[tii.HeightMapWidth];

		for (int j = 0; j < tii.HeightMapWidth; j++)
		{
			array[i][j] = 0.0f;
		}
	}

	for (int k = 0; k < 1000; k++)
	{
		float x1 = Random::FloatInRange(0.0f, tii.HeightMapWidth);
		float z1 = Random::FloatInRange(0.0f, tii.HeightMapHeight);
		float x2 = Random::FloatInRange(0.0f, tii.HeightMapWidth);
		float z2 = Random::FloatInRange(0.0f, tii.HeightMapHeight);

		float displacement = 0.5f + (k / 1000) * (tii.HeightScale - 0.5f);

		for (int i = 0; i < tii.HeightMapHeight; i++)
		{
			for (int j = 0; j < tii.HeightMapWidth; j++)
			{
				float dist = ((x2 - x1) * (i - z1) - (z2 - z1) * (j - x1));
				if (dist > 0 && dist < XM_PIDIV2)
				{
					array[i][j] += displacement * sin(dist);
				}
				else if(dist <0 && dist > - XM_PIDIV2)
				{
					array[i][j] -= displacement *sin(dist);
				}
				else if (dist > 0)
				{
					array[i][j] += displacement;
				}
				else
				{
					array[i][j] -= displacement;
				}
			}
		}
	}

	std::vector<float> returnHeightMap;

	for (int i = 0; i < tii.HeightMapHeight; i++)
	{
		for (int j = 0; j < tii.HeightMapWidth; j++)
		{
			returnHeightMap.push_back(array[i][j]);
		}
	}
	return returnHeightMap;
}

std::vector<float> ProceduralLandscape::LoadHeightMap(Terrain::InitInfo tii)
{
	std::vector<float> heightMap;

	//A height for each vertex
	std::vector<unsigned char> in(tii.HeightMapWidth * tii.HeightMapHeight);

	//open the file
	std::ifstream inFile;
	inFile.open(tii.HeightMapFilename, std::ios_base::binary);

	if (inFile)
	{
		//Read the Raw bytes
		inFile.read((char*)&in[0], (std::streamsize)in.size());

		inFile.close();
	}

	//Copy the array data into a float array and scale it.
	heightMap.resize(tii.HeightMapHeight * tii.HeightMapWidth, 0);

	for (unsigned int i = 0; i < tii.HeightMapHeight * tii.HeightMapWidth; ++i)
	{
		heightMap[i] = (in[i] / 255.0f) * tii.HeightScale;
	}

	return heightMap;
}

void DiamondSquareStep(float ** array, int chunk_x, int chunk_z, int size)
{
	int half = size / 2;

	if (half < 1)
		return;

	//square steps
	for (int z = half; z < chunk_z; z += size)
	{
		for (int x = half; x < chunk_x; x += size)
			SquareStep(array, chunk_x, chunk_z, x % chunk_x, z % chunk_z, half);
	}

	//diamond steps
	int col = 0;
	for (int x = 0; x < chunk_x; x += half)
	{
		col++;
		//if this is an odd column
		if (col % 2 == 1)
		{
			for (int z = half; z < chunk_z; z += size)
			{
				DiamondStep(array, chunk_x, chunk_z, x % chunk_x, z % chunk_z, half);
			}
		}
		else
		{
			for (int z = 0; z < chunk_z; z += size)
			{
				DiamondStep(array, chunk_x, chunk_z, x % chunk_x, z % chunk_z, half);
			}
		}
	}

	DiamondSquareStep(array,chunk_x, chunk_z, size / 2);
}

void SquareStep(float ** array, int chunk_x, int chunk_z, int x, int z, int reach)
{
	int count = 0;
	float avg = 0.0f;
	if (x - reach >= 0 && z - reach >= 0)
	{
		avg += array[x - reach][z - reach];
		count++;
	}
	if (x - reach >= 0 && z + reach < chunk_z)
	{
		avg += array[x - reach][z + reach];
		count++;
	}
	if (x + reach < chunk_x && z - reach >= 0)
	{
		avg += array[x + reach][z - reach];
		count++;
	}
	if (x + reach < chunk_x && z + reach < chunk_z)
	{
		avg += array[x + reach][z + reach];
		count++;
	}

	avg += Random::FloatInRange(-reach, reach);
	avg /= count;
	array[x][z] = avg;
}

void DiamondStep(float ** array, int chunk_x, int chunk_z, int x, int z, int reach)
{
	int count = 0;
	float avg = 0.0f;
	if (x - reach >= 0)
	{
		avg += array[x - reach][z];
		count++;
	}
	if (x + reach < chunk_x)
	{
		avg += array[x + reach][z];
		count++;
	}
	if (z - reach >= 0)
	{
		avg += array[x][z - reach];
		count++;
	}
	if (z + reach < chunk_z)
	{
		avg += array[x][z + reach];
		count++;
	}

	avg += Random::FloatInRange(-reach, reach);
	avg /= count;
	array[x][z] = avg;
}
