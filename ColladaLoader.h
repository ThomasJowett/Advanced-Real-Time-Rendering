#pragma once

#include "Commons.h"
#include "Vector.h"
#include "AnimatedModelData.h"
#include "TinyXML2.h"

namespace ColladaLoader
{
	AnimatedModelData Load(const char* filename, int maxWeights);

	struct SkinLoader
	{
		SkinLoader(tinyxml2::XMLElement* node);
		SkinningData ExtractSkinData();
	};

	struct SkeletonLoader
	{
		SkeletonLoader(tinyxml2::XMLElement* node);
		SkeletonData ExtractSkeletonData();
	};

	struct GeometryLoader
	{
		GeometryLoader(tinyxml2::XMLElement* node);
		SkeletalMeshData ExtractMeshData();
	};
}


