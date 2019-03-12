#include "ColladaLoader.h"
#include "Quaternion.h"

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
			//remove the # from the start
			jointDataId.erase(jointDataId.begin());
		}
		else if (strcmp(pInputNode->Attribute("semantic"), "WEIGHT") == 0)
		{
			weightDataId = pInputNode->Attribute("source");
			//remove the # from the start
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

	JointData* headJoint = LoadJointData(pHeadNode, true, jointOrder);

	return SkeletonData(jointOrder.size(), *headJoint);
}

JointData* ColladaLoader::LoadJointData(tinyxml2::XMLElement * node, bool isRoot, std::vector<std::string> jointOrder)
{
	std::string nameId = node->Attribute("id");
	auto it = std::find(jointOrder.begin(), jointOrder.end(), nameId);
	int index = std::distance(jointOrder.begin(), it);

	//load the raw data as a list of strings
	std::vector<std::string> matrixRawData = Util::SplitString(node->FirstChildElement("matrix")->GetText(), ' ');

	//convert the raw data into a matrix
	XMMATRIX matrix = XMMATRIX(atof(matrixRawData[0].c_str()), atof(matrixRawData[1].c_str()), atof(matrixRawData[2].c_str()), atof(matrixRawData[3].c_str()),
		atof(matrixRawData[4].c_str()), atof(matrixRawData[5].c_str()), atof(matrixRawData[6].c_str()), atof(matrixRawData[7].c_str()),
		atof(matrixRawData[8].c_str()), atof(matrixRawData[9].c_str()), atof(matrixRawData[10].c_str()), atof(matrixRawData[11].c_str()),
		atof(matrixRawData[12].c_str()), atof(matrixRawData[13].c_str()), atof(matrixRawData[14].c_str()), atof(matrixRawData[15].c_str()));

	XMMatrixTranspose(matrix);
	if (isRoot)
	{
		//rotate the root bone so that it is facing upwards
		matrix = matrix * XMMatrixRotationX(-XM_PIDIV2);
	}
	JointData* joint = new JointData(index, nameId, matrix);


	tinyxml2::XMLElement * childNode = node->FirstChildElement("node");
	while (childNode)
	{
		joint->AddChild(LoadJointData(childNode, false, jointOrder));
		childNode = childNode->NextSiblingElement("node");
	}

	return joint;
}

SkeletalMeshData ColladaLoader::LoadGeometry(tinyxml2::XMLElement * node, std::vector<VertexSkinData> vertexSkinData)
{
	std::vector<float> verticesArray;
	std::vector<float> normalsArray;
	std::vector<float> texturesArray;
	std::vector<int> indicesArray;
	std::vector<int> jointIdsArray;
	std::vector<float> weightsArray;

	std::vector<VertexData> verts;
	std::vector<XMFLOAT3> normals;
	std::vector<XMFLOAT2> TexCoords;
	std::vector<int> indices;


	tinyxml2::XMLElement * pMeshNode = node->FirstChildElement("geometry")->FirstChildElement("mesh");

	//read the raw data ----------------------------------------------------------------------------------------------

	//read in the Ids
	std::string positionsId = pMeshNode->FirstChildElement("vertices")->FirstChildElement("input")->Attribute("source");
	//remove the # from the begining
	positionsId.erase(positionsId.begin());

	std::string normalsId;
	std::string texCoordsId;

	tinyxml2::XMLElement * pPolyNode = pMeshNode->FirstChildElement("polylist");

	tinyxml2::XMLElement * pInputNode = pPolyNode->FirstChildElement("input");

	int typeCount = 0;

	while (pInputNode)
	{
		typeCount++;
		if (strcmp(pInputNode->Attribute("semantic"), "NORMAL") == 0)
		{
			normalsId = pInputNode->Attribute("source");
			normalsId.erase(normalsId.begin());
		}
		else if (strcmp(pInputNode->Attribute("semantic"), "TEXCOORD") == 0)
		{
			texCoordsId = pInputNode->Attribute("source");
			texCoordsId.erase(texCoordsId.begin());
		}
		pInputNode = pInputNode->NextSiblingElement("input");
	}
	
	
	tinyxml2::XMLElement * pSourceNode = pMeshNode->FirstChildElement("source");

	while (pSourceNode)
	{
		//read the positions
		if (strcmp(pSourceNode->Attribute("id"), positionsId.c_str()) == 0)
		{
			tinyxml2::XMLElement * pPositionData = pSourceNode->FirstChildElement("float_array");

			int count = atoi(pPositionData->Attribute("count"));

			std::vector<std::string> positionRawData = Util::SplitString(pPositionData->GetText(), ' ');

			for (int i = 0; i < count / 3; i++)
			{
				float x = atof(positionRawData[i * 3].c_str());
				float y = atof(positionRawData[i * 3 + 1].c_str());
				float z = atof(positionRawData[i * 3 + 2].c_str());

				Vector3D position = { x,y,z };

				Quaternion rotation = Quaternion(-XM_PIDIV2, 0, 0);

				rotation.RotateVectorByQuaternion(position);
				verts.push_back(VertexData(verts.size(), position, vertexSkinData[verts.size()]));
			}
		}
		//read the normals
		else if (strcmp(pSourceNode->Attribute("id"), normalsId.c_str()) == 0)
		{
			tinyxml2::XMLElement * pNormalData = pSourceNode->FirstChildElement("float_array");

			int count = atoi(pNormalData->Attribute("count"));

			std::vector<std::string> normalRawData = Util::SplitString(pNormalData->GetText(), ' ');

			for (int i = 0; i < count / 3; i++)
			{
				float x = atof(normalRawData[i * 3].c_str());
				float y = atof(normalRawData[i * 3 + 1].c_str());
				float z = atof(normalRawData[i * 3 + 2].c_str());
				Vector3D normal = { x,y,z };
				Quaternion rotation = Quaternion(-XM_PIDIV2, 0, 0);
				rotation.RotateVectorByQuaternion(normal);
				normals.push_back(XMFLOAT3(normal.x, normal.y, normal.z));
			}
		}
		//read texture coordinates
		else if (strcmp(pSourceNode->Attribute("id"), texCoordsId.c_str()) == 0)
		{
			tinyxml2::XMLElement * ptexCoordData = pSourceNode->FirstChildElement("float_array");

			int count = atoi(ptexCoordData->Attribute("count"));

			std::vector<std::string> texCoordsRawData = Util::SplitString(ptexCoordData->GetText(), ' ');

			for (int i = 0; i < count / 2; i++)
			{
				float u = atof(texCoordsRawData[i * 2].c_str());
				float v = atof(texCoordsRawData[i * 2 + 1].c_str());
				TexCoords.push_back(XMFLOAT2(u, v));
			}
		}

		pSourceNode = pSourceNode->NextSiblingElement("source");
	}

	//Assemble the vertices--------------------------------------------------------------------------------------------------------

	std::vector<std::string> indexRawData = Util::SplitString(pPolyNode->FirstChildElement("p")->GetText(), ' ');

	for (int i = 0; i < indexRawData.size() / typeCount; i++)
	{
		int positionIndex = atoi(indexRawData[i * typeCount].c_str());
		int normalIndex = atoi(indexRawData[i * typeCount + 1].c_str());
		int texCoordIndex = atoi(indexRawData[i * typeCount + 2].c_str());

		VertexData currentVertex = verts.at(positionIndex);
		if (!currentVertex.IsSet())
		{
			currentVertex.textureIndex = texCoordIndex;
			currentVertex.normalIndex = normalIndex;
			indices.push_back(positionIndex);
		}
		else
		{
			//vertex has already been processed
			if (currentVertex.HasSameTextureAndNormal(texCoordIndex, normalIndex))
			{
				indices.push_back(currentVertex.index);
			}
			else
			{
				VertexData anotherVertex = currentVertex.duplicateVertex;
				if(anotherVertex != )

					//TODO understand duplicate vertex
			}
		}
	}
	return SkeletalMeshData();
}
