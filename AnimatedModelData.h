#pragma once
#include <string>
#include <DirectXMath.h>
#include <vector>
#include "Vector.h"

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
	JointData rootJoint;

public:
	SkeletonData(int jointCount, JointData rootJoint)
		:jointCount(jointCount), rootJoint(rootJoint) {}
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

	SkeletalMeshData() = default;

	SkeletalMeshData(std::vector<float> vertices, std::vector<float> textureCoords, std::vector<float> normals, std::vector<int> indices, std::vector<int> jointIDs, std::vector<float> vertexWeights)
		:vertices(vertices), textureCoords(textureCoords), normals(normals), indices(indices), jointIDs(jointIDs), vertexWeights(vertexWeights) {}
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

	XMFLOAT4 GetWeights()
	{
		XMFLOAT4 returnWeights = { 0.0f,0.0f,0.0f,0.0f };

		for (int i = 0; i < weights.size(); i++)
		{
			if (i == 0)
				returnWeights.x = weights[i];
			if (i == 1)
				returnWeights.y = weights[i];
			if (i == 2)
				returnWeights.z = weights[i];
			if (i == 3)
				returnWeights.w = weights[i];
		}

		return returnWeights;
	}

	XMFLOAT4 GetBoneIndices()
	{
		XMFLOAT4 returnIndices = { 0.0f,0.0f,0.0f,0.0f };

		for (int i = 0; i < jointIds.size(); i++)
		{
			if (i == 0)
				returnIndices.x = jointIds[i];
			if (i == 1)
				returnIndices.y = jointIds[i];
			if (i == 2)
				returnIndices.z = jointIds[i];
			if (i == 3)
				returnIndices.w = jointIds[i];
		}

		return returnIndices;
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
	VertexData *duplicateVertex = nullptr;
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

	VertexData* GetDuplicateVertex()
	{
		return duplicateVertex;
	}

	void AverageTangents()
	{
		if (tangents.size() == 0)
		{
			return;
		}
		for (Vector3D tangent : tangents)
		{
			//TODO generate or load tangents
		}
	}
};

struct AnimatedModelData
{
	SkeletonData joints;
	IndexedSkeletalModel meshData;

	AnimatedModelData(SkeletonData joints, IndexedSkeletalModel meshData)
		:joints(joints), meshData(meshData) {}

	AnimatedModelData()
		:joints(SkeletonData(0, JointData(0, "null", XMMATRIX()))) {};

	IndexedModel ToIndexedModel()
	{
		SimpleVertex* verts = new SimpleVertex[meshData.Vertices.size()];
		for (int i = 0; i < meshData.Vertices.size(); i++)
		{
			verts[i].PosL = meshData.Vertices[i].PosL;

			verts[i].NormL = meshData.Vertices[i].NormL;

			verts[i].Tex = meshData.Vertices[i].Tex;

			verts[i].Tangent = meshData.Vertices[i].Tangent;
		}

		unsigned short* indicesArray = new unsigned short[meshData.Indices.size()];
		for (int i = 0; i < meshData.Indices.size(); i++)
		{
			indicesArray[i] = meshData.Indices[i];
		}

		IndexedModel returnGeometry;

		returnGeometry.Vertices.assign(&verts[0], &verts[meshData.Vertices.size()]);
		returnGeometry.Indices.assign(&indicesArray[0], &indicesArray[meshData.Indices.size()]);

		return returnGeometry;
	}
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

