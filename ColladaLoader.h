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

	SkeletonData LoadSkeleton(tinyxml2::XMLElement* node, std::vector<std::string> jointOrder);

	JointData LoadJointData(tinyxml2::XMLElement * node, bool isRoot);

	SkeletalMeshData LoadGeometry(tinyxml2::XMLElement* node, std::vector<VertexSkinData> vertexSkinData);
}