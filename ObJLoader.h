#pragma once
#include <string>

#include "Commons.h"
#include "Vector.h"

struct OBJIndex
{
	unsigned int vertexIndex;
	unsigned int uvIndex;
	unsigned int normalIndex;

	bool operator<(const OBJIndex& r) const { return vertexIndex < r.vertexIndex; }
};

struct OBJModel
{
	std::vector<OBJIndex> OBJIndices;
	std::vector<Vector3D> vertices;
	std::vector<Vector2D> uvs;
	std::vector<Vector3D> normals;
	bool hasUVs;
	bool hasNormals;
};

class OBJLoader
{
public:
	static IndexedModel Load(const char* filename, bool invertCoordinates);
private:
	static IndexedModel ToIndexedModel(OBJModel model);

	unsigned int FindLastVertexIndex(const std::vector<OBJIndex*>& indexLookup, const OBJIndex* currentIndex, const IndexedModel& result);
	void CreateOBJFace(const std::string& line);

	Vector2D ParseOBJVec2(const std::string& line);
	Vector3D ParseOBJVec3(const std::string& line);
	OBJIndex ParseOBJIndex(const std::string& token, bool* hasUVs, bool* hasNormals);
};