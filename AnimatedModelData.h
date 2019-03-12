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

struct VertexSkinData
{
	std::vector<int> jointIds;
	std::vector<float> weights;

	void SetNumberOfEffects(int numberOfEffects)
	{
		jointIds.resize(numberOfEffects);
		weights.resize(numberOfEffects);
	}

	void AddJointEffect(int jointId, float weight)
	{
		for (int i = 0; i < weights.size(); i++)
		{
			if (weight > weights[i])
			{
				jointIds.emplace(jointIds.begin() + i, jointId);
				weights.emplace(weights.begin() + i, weight);
				break;
			}
		}
	}

	void LimitJointNumber(int max)
	{
		if (jointIds.size() > max)
		{
			jointIds.resize(max);
			weights.resize(max);
		}
	}
};

struct SkinningData
{
	std::vector<std::string> jointOrder;
	std::vector<VertexSkinData> verticesSkinData;

	SkinningData(std::vector<std::string> jointOrder, std::vector<VertexSkinData> verticesSkinData)
		:jointOrder(jointOrder), verticesSkinData(verticesSkinData) {}
};

struct VertexData
{
	Vector3D position;
	int textureIndex = -1;
	int normalIndex = -1;
	VertexData duplicateVertex;
	int index;
	float length;
	std::vector<Vector3D> tangents;
	Vector3D averagedTangent;

	VertexSkinData weightsData;

	VertexData(int index, Vector3D position, VertexSkinData weightsData)
	{
		this->index = index;
		this->weightsData = weightsData;
		this->position = position;
		this->length = position.Magnitude();
	}

	bool IsSet()
	{
		return textureIndex != -1 && normalIndex != -1;
	}

	bool HasSameTextureAndNormal(int textureIndexOther, int normalIndexOther)
	{
		return textureIndexOther == textureIndex && normalIndexOther == normalIndex;
	}
};

struct AnimatedModelData
{
	SkeletonData joints;
	SkeletalMeshData meshData;

	AnimatedModelData(SkeletonData joints, SkeletalMeshData meshData)
		:joints(joints), meshData(meshData) {}

	AnimatedModelData()
		:joints(SkeletonData(0, JointData(0, "null", XMMATRIX()))) {};
};

struct JointTransformData
{
	std::string jointNameId;
	XMMATRIX jointLocalTransform;
};

struct KeyFrameData
{
	float time;
	std::vector<JointTransformData> jointTransforms;
};

struct AnimationData
{
	float lengthSeconds;
	std::vector<KeyFrameData> keyframes;
};