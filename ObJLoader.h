#pragma once
#include <string>
#include <map>			//For fast searching when re-creating the index buffer


#include "Commons.h"
#include "Vector.h"

namespace OBJLoader
{
	IndexedModel Load(const char* filename, bool invertCoordinates);

	//searches to see if a similar vertex already exists in the buffer
	bool FindSimilarVertex(const SimpleVertex& vertex, std::map<SimpleVertex, unsigned short>& vertToIndexMap, unsigned short& index);

	//Re-Creates a single index buffer from the 3 given in the OBJ file
	void CreateIndices(const std::vector<XMFLOAT3>& inVertices, 
		const std::vector<XMFLOAT2>& inTexCoords, 
		const std::vector<XMFLOAT3>& inNormals, 
		std::vector<unsigned short>& outIndices, 
		std::vector<XMFLOAT3>& outVertices, 
		std::vector<XMFLOAT2>& outTexCoords, 
		std::vector<XMFLOAT3>& outNormals);

	//Creates the tangents from the normals and texture coordinates
	void InsertTangentsIntoArray(SimpleVertex* vertices, unsigned short* indices, int vertexCount);

	XMFLOAT3 CalculateTangent(SimpleVertex v0, SimpleVertex v1, SimpleVertex v2);
};