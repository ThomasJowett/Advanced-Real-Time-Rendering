#pragma once

#include "Commons.h"
#include "Vector.h"
#include "AnimatedModelData.h"
#include "TinyXML2.h"

namespace ColladaLoader
{
	AnimatedModelData LoadModel(const char* filename, int maxWeights);

	AnimationData LoadAnimation(const char* filename);

	SkinningData LoadSkin(tinyxml2::XMLElement* node, int maxWeights);

	SkeletonData LoadSkeleton(tinyxml2::XMLElement* node, std::vector<std::string> jointOrder, std::vector<XMFLOAT4X4> inverseBindTransforms);

	JointData* LoadJointData(tinyxml2::XMLElement * node, bool isRoot, std::vector<std::string> jointOrder, std::vector<XMFLOAT4X4> inverseBindTransforms);

	IndexedSkeletalModel LoadGeometry(tinyxml2::XMLElement* node, std::vector<VertexSkinData> vertexSkinData);

	void DealWithAlreadyProcessedVertex(VertexData* previousVertex, int newTextureIndex, int newNormalIndex, std::vector<int> &indices, std::vector<VertexData> &verts);

	void InsertTangentsIntoArray(SkeletalVertex* vertices, unsigned short* indices, int vertexCount);

	XMFLOAT3 CalculateTangent(SkeletalVertex v0, SkeletalVertex v1, SkeletalVertex v2);
}