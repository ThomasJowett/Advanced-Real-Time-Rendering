#pragma once
#include <string>
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

struct JointData
{
	int index;
	std::string nameID;
	XMMATRIX bindLocalTransform;

	std::vector<JointData*> children;

public:
	JointData(int index, std::string nameID, XMMATRIX bindLocalTransform)
		:index(index), nameID(nameID), bindLocalTransform(bindLocalTransform) {}

	void AddChild(JointData* child)
	{
		children.push_back(child);
	}

};

struct SkeletonData
{
	int jointCount;
	JointData headJoint;

public:
	SkeletonData(int jointCount, JointData headJoint)
		:jointCount(jointCount), headJoint(headJoint) {}
};

struct SkeletalMeshData
{
	const static int DIMENSIONS = 3;

	std::vector<float> vertices;
	std::vector<float> textureCoords;
	std::vector<float> normals;
	std::vector<int> indices;
	std::vector<int> jointIDs;
	std::vector<float> vertexWeights;

	int GetVertexCount() { return vertices.size() / DIMENSIONS; }
};

struct SkinningData
{
	std::vector<std::string> jointOrder;

};

struct AnimatedModelData
{
	SkeletonData joints;
	SkeletalMeshData meshData;

	AnimatedModelData(SkeletonData joints, SkeletalMeshData meshData)
		:joints(joints), meshData(meshData) {}
};