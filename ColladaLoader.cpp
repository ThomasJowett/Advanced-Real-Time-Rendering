#include "ColladaLoader.h"


AnimatedModelData ColladaLoader::LoadModel(const char * filename, int maxWeights)
{
	tinyxml2::XMLDocument doc;

	if (doc.LoadFile(filename) == 0)
	{
		tinyxml2::XMLElement* pRoot;

		pRoot = doc.FirstChildElement("COLLADA");

		tinyxml2::XMLElement* pNode = pRoot->FirstChildElement("library_controllers");
		SkinningData skinningData = LoadSkin(pNode, maxWeights);

		pNode = pRoot->FirstChildElement("library_visual_scenes");
		SkeletonData skeletonData = LoadSkeleton(pNode, skinningData.jointOrder);

		pNode = pRoot->FirstChildElement("library_geometries");
		SkeletalMeshData meshData = LoadGeometry(pNode, skinningData.verticesSkinData);

		return AnimatedModelData(skeletonData, meshData);
	}

	return AnimatedModelData();
}

SkinningData ColladaLoader::LoadSkin(tinyxml2::XMLElement * node, int maxWeights)
{
	//find the skin node
	tinyxml2::XMLElement * pSkinNode = node->FirstChildElement("controller")->FirstChildElement("skin");

	//find the vertex weights node
	tinyxml2::XMLElement * pVertexWeightsNode = pSkinNode->FirstChildElement("vertex_weights");

	std::string jointDataId;
	std::string weightDataId;
	std::vector<std::string> jointNames;
	std::vector<float> weights;
	std::vector<int> effectorJointCounts;
	std::vector<VertexSkinData> vertexWeights;

	tinyxml2::XMLElement * pInputNode = pVertexWeightsNode->FirstChildElement("input");

	//get the data IDs
	while (pInputNode)
	{
		if (strcmp(pInputNode->Attribute("semantic"), "JOINT") == 0)
		{
			jointDataId = pInputNode->Attribute("source");
			jointDataId.erase(jointDataId.begin());
		}
		else if (strcmp(pInputNode->Attribute("semantic"), "WEIGHT") == 0)
		{
			weightDataId = pInputNode->Attribute("source");
			weightDataId.erase(weightDataId.begin());
		}

		pInputNode = pInputNode->NextSiblingElement("input");
	}

	tinyxml2::XMLElement * pSourceNode = pSkinNode->FirstChildElement("source");

	//get the data
	while (pSourceNode)
	{
		if (strcmp(pSourceNode->Attribute("id"), jointDataId.c_str()) == 0)
		{
			jointNames = Util::SplitString(pSourceNode->FirstChildElement("Name_array")->GetText(), ' ');
		}
		else if (strcmp(pSourceNode->Attribute("id"), weightDataId.c_str()) == 0)
		{
			std::vector<std::string> rawData = Util::SplitString(pSourceNode->FirstChildElement("float_array")->GetText(), ' ');

			for (int i = 0; i < rawData.size(); i++)
			{
				weights.push_back(atof(rawData[i].c_str()));
			}
		}
		pSourceNode = pSourceNode->NextSiblingElement("source");
	}

	//get the effective vertex counts data
	std::vector<std::string> rawData = Util::SplitString(pVertexWeightsNode->FirstChildElement("vcount")->GetText(), ' ');

	for (int i = 0; i < rawData.size(); i++)
	{
		effectorJointCounts.push_back(atoi(rawData[i].c_str()));
	}

	//get the vertex weights
	rawData = Util::SplitString(pVertexWeightsNode->FirstChildElement("v")->GetText(), ' ');
	int pointer = 0;
	for (int count : effectorJointCounts)
	{
		VertexSkinData skinData;
		skinData.SetNumberOfEffects(count);
		for (int i = 0; i < count; i++)
		{
			int jointId = atoi(rawData[pointer++].c_str());
			int weightId = atoi(rawData[pointer++].c_str());
			skinData.AddJointEffect(jointId, weights[weightId]);
		}
		skinData.LimitJointNumber(maxWeights);
		vertexWeights.push_back(skinData);
	}

	return SkinningData(jointNames, vertexWeights);
}

SkeletonData ColladaLoader::LoadSkeleton(tinyxml2::XMLElement * node, std::vector<std::string> jointOrder)
{
	tinyxml2::XMLElement * pVisualSceneNode = node->FirstChildElement("visual_scene");

	//find the armature node
	tinyxml2::XMLElement * pArmatureNode = pVisualSceneNode->FirstChildElement("node");
	
	while (pArmatureNode)
	{
		if (strcmp(pArmatureNode->Attribute("id"), "Armature") == 0)
		{
			break;
		}
		pArmatureNode = pArmatureNode->NextSiblingElement("node");
	}
	tinyxml2::XMLElement * pHeadNode = pArmatureNode->FirstChildElement("node");

	JointData headJoint = LoadJointData(pHeadNode, true);

	return SkeletonData(jointOrder.size(), headJoint);
}

JointData ColladaLoader::LoadJointData(tinyxml2::XMLElement * node, bool isRoot)
{
	std::string nameId = node->Attribute("id");
	int index;
	XMMATRIX matrix = XMMATRIX();
	//TODO: load matrix
	XMMatrixTranspose(matrix);
	JointData joint = JointData(index, nameId, matrix);

	return joint;
}