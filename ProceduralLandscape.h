#pragma once

//#include "noiseutils.h"
#include <vector>
#include "Terrain.h"



namespace ProceduralLandscape
{
	std::vector<float> PerlinNoise();
	std::vector<float> DiamondSquare(Terrain::InitInfo tii);
	std::vector<float> FaultLine(Terrain::InitInfo tii);
	std::vector<float> LoadHeightMap(Terrain::InitInfo tii);
}