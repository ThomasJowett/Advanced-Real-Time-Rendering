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
	SkeletalMeshData meshData;

	AnimatedModelData(SkeletonData joints, SkeletalMeshData meshData)
		:joints(joints), meshData(meshData) {}

	AnimatedModelData()
		:joints(SkeletonData(0, JointData(0, "null", XMMATRIX()))) {};

	IndexedModel ToIndexedModel()
	{
		SimpleVertex* verts = new SimpleVertex[meshData.vertices.size()/3];
		for (int i = 0; i < meshData.vertices.size()/3; i++)
		{
			verts[i].PosL.x = meshData.vertices[i * 3];
			verts[i].PosL.y = meshData.vertices[i * 3 + 1];
			verts[i].PosL.z = meshData.vertices[i * 3 + 2];

			verts[i].NormL.x = meshData.normals[i * 3];
			verts[i].NormL.y = meshData.normals[i * 3 + 1];
			verts[i].NormL.z = meshData.normals[i * 3 + 2];

			verts[i].Tangent.x = 0.0f;
			verts[i].Tangent.y = 1.0f;
			verts[i].Tangent.z = 0.0f;
		}

		for (int i = 0; i < meshData.textureCoords.size() / 2; i++)
		{
			verts[i].Tex.x = meshData.textureCoords[i / 2];
			verts[i].Tex.y = meshData.textureCoords[i / 2 + 1];
		}

		unsigned short* indicesArray = new unsigned short[meshData.indices.size()];
		for (int i = 0; i < meshData.indices.size(); i++)
		{
			indicesArray[i] = meshData.indices[i];
		}

		IndexedModel returnGeometry;

		returnGeometry.Vertices.assign(&verts[0], &verts[meshData.vertices.size() / 3]);
		returnGeometry.Indices.assign(&indicesArray[0], &indicesArray[meshData.indices.size()]);

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

