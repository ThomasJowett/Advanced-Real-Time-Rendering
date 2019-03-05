#include "ColladaLoader.h"


AnimatedModelData ColladaLoader::Load(const char * filename, int maxWeights)
{
	AnimatedModelData model;

	tinyxml2::XMLDocument doc;

	if (doc.LoadFile(filename) == 0)
	{
		tinyxml2::XMLElement* pRoot;

		pRoot = doc.FirstChildElement("COLLADA");

		tinyxml2::XMLElement* pNode = pRoot->FirstChildElement("library_controllers");

		SkinLoader skinLoader = SkinLoader(pNode);
		SkinningData SkinData = skinLoader.ExtractSkinData();

		pNode = pRoot->FirstChildElement("library_visual_scenes");

		SkeletonLoader skeletonLoader = SkeletonLoader(pNode);
		SkeletonData jointsData = skeletonLoader.ExtractSkeletonData();

		pNode = pRoot->FirstChildElement("library_geometries");

		GeometryLoader geometry = GeometryLoader(pNode);
		SkeletalMeshData meshData = geometry.ExtractMeshData();

		model 
	}

	return model;
}
