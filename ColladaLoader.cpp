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
		if (pNode)
		{
			SkinningData skinningData = LoadSkin(pNode, maxWeights);

			pNode = pRoot->FirstChildElement("library_visual_scenes");
			SkeletonData skeletonData = LoadSkeleton(pNode, skinningData.jointOrder, skinningData.bindMatrices);

			pNode = pRoot->FirstChildElement("library_geometries");
			IndexedSkeletalModel meshData = LoadGeometry(pNode, skinningData.verticesSkinData);

			return AnimatedModelData(skeletonData, meshData);
		}
	}

	return AnimatedModelData();
}

AnimationData ColladaLoader::LoadAnimation(const char * filename)
{
	AnimationData returnAnimationData;

	std::vector<KeyFrameData> keyFramesData;
	std::vector<float> keyFrameTimes;

	tinyxml2::XMLDocument doc;

	if (doc.LoadFile(filename) == 0)
	{
		tinyxml2::XMLElement* pRoot;

		pRoot = doc.FirstChildElement("COLLADA");
		tinyxml2::XMLElement* pNode;

		std::string rootJointName;

		pNode = pRoot->FirstChildElement("library_visual_scenes")->FirstChildElement("visual_scene")->FirstChildElement("node");

		while (pNode)
		{
			if (strcmp(pNode->Attribute("id"), "Armature") == 0)
			{
				pNode = pNode->FirstChildElement("node");
				rootJointName = pNode->Attribute("id");
				break;
			}

			pNode = pNode->NextSiblingElement("node");
		}

		pNode = pRoot->FirstChildElement("library_animations");

		

		if (pNode)
		{
			tinyxml2::XMLElement* pAnimation = pNode->FirstChildElement("animation");

			while (pAnimation)
			{
				std::string inputID, outputID;

				tinyxml2::XMLElement* pSampler = pAnimation->FirstChildElement("sampler");

				tinyxml2::XMLElement* pInput = pSampler->FirstChildElement("input");

				//get joint name
				tinyxml2::XMLElement* pChannel = pAnimation->FirstChildElement("channel");

				std::string jointName = Util::SplitString(pChannel->Attribute("target"), '/')[0];

				//get input and output IDs
				while (pInput)
				{
					if (strcmp(pInput->Attribute("semantic"), "INPUT") == 0)
					{
						inputID = pInput->Attribute("source");
						inputID.erase(inputID.begin());
					}
					else if(strcmp(pInput->Attribute("semantic"), "OUTPUT") == 0)
					{
						outputID = pInput->Attribute("source");
						outputID.erase(outputID.begin());
					}

					pInput = pInput->NextSiblingElement("input");
				}

				std::vector<std::string> rawtimesData;
				std::vector<std::string> rawMatricesData;

				tinyxml2::XMLElement* pSource = pAnimation->FirstChildElement("source");

				while (pSource)
				{
					if (strcmp(pSource->Attribute("id"), inputID.c_str()) == 0)
					{
						rawtimesData = Util::SplitString(pSource->FirstChildElement("float_array")->GetText(), ' ');
					}
					else if (strcmp(pSource->Attribute("id"), outputID.c_str()) == 0)
					{
						rawMatricesData = Util::SplitString(pSource->FirstChildElement("float_array")->GetText(), ' ');
					}
					pSource = pSource->NextSiblingElement("source");

				}

				//if this is the root bone then load the keyframe times
				if (strcmp(jointName.c_str(), rootJointName.c_str()) == 0)
				{
					for (std::string time : rawtimesData)
					{
						keyFrameTimes.push_back(atof(time.c_str()));
					}

					returnAnimationData.lengthSeconds = keyFrameTimes.back();

					keyFramesData.resize(keyFrameTimes.size());

					for (int i = 0; i< keyFramesData.size(); i++)
					{
						keyFramesData[i].time = keyFrameTimes[i];
					}
				}


				for (int i = 0; i < keyFramesData.size(); i++)
				{
					
					//convert the raw data into a matrix
					XMMATRIX matrix = XMMATRIX(atof(rawMatricesData[i * 16 + 0].c_str()), atof(rawMatricesData[i * 16 + 1].c_str()), atof(rawMatricesData[i * 16 + 2].c_str()), atof(rawMatricesData[i * 16 + 3].c_str()),
						atof(rawMatricesData[i * 16 + 4].c_str()), atof(rawMatricesData[i * 16 + 5].c_str()), atof(rawMatricesData[i * 16 + 6].c_str()), atof(rawMatricesData[i * 16 + 7].c_str()),
						atof(rawMatricesData[i * 16 + 8].c_str()), atof(rawMatricesData[i * 16 + 9].c_str()), atof(rawMatricesData[i * 16 + 10].c_str()), atof(rawMatricesData[i * 16 + 11].c_str()),
						atof(rawMatricesData[i * 16 + 12].c_str()), atof(rawMatricesData[i * 16 + 13].c_str()), atof(rawMatricesData[i * 16 + 14].c_str()), atof(rawMatricesData[i * 16 + 15].c_str()));

					XMFLOAT4X4 matrixAsFloats;

					XMStoreFloat4x4(&matrixAsFloats, matrix);

					keyFramesData[i].jointTransforms.insert(std::pair<std::string, XMFLOAT4X4> (jointName, matrixAsFloats));
				}

				pAnimation = pAnimation->NextSiblingElement("animation");
			}
			
		}
	}

	returnAnimationData.keyframes = keyFramesData;
	return returnAnimationData;
}

SkinningData ColladaLoader::LoadSkin(tinyxml2::XMLElement * node, int maxWeights)
{
	//find the skin node
	tinyxml2::XMLElement * pSkinNode = node->FirstChildElement("controller")->FirstChildElement("skin");

	tinyxml2::XMLElement * pJointsNode = pSkinNode->FirstChildElement("joints");

	tinyxml2::XMLElement * pInputNode = pJointsNode->FirstChildElement("input");

	std::string inverseBindId;

	while (pInputNode)
	{
		if (strcmp(pInputNode->Attribute("semantic"), "INV_BIND_MATRIX") == 0)
		{
			inverseBindId = pInputNode->Attribute("source");
			inverseBindId.erase(inverseBindId.begin());
		}

		pInputNode = pInputNode->NextSiblingElement("input");
	}



	//find the vertex weights node
	tinyxml2::XMLElement * pVertexWeightsNode = pSkinNode->FirstChildElement("vertex_weights");

	std::string jointDataId;
	std::string weightDataId;
	std::vector<std::string> jointNames;
	std::vector<float> weights;
	std::vector<int> effectorJointCounts;
	std::vector<VertexSkinData> vertexWeights;
	std::vector<XMFLOAT4X4> bindMatrices;

	pInputNode = pVertexWeightsNode->FirstChildElement("input");

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
		else if (strcmp(pSourceNode->Attribute("id"), inverseBindId.c_str()) == 0)
		{
			std::vector<std::string> rawData = Util::SplitString(pSourceNode->FirstChildElement("float_array")->GetText(), ' ');

			int boneCount = atoi(pSourceNode->FirstChildElement("technique_common")->FirstChildElement("accessor")->Attribute("count"));

			

			for (int i = 0; i < boneCount; i++)
			{
				bindMatrices.push_back(XMFLOAT4X4(atof(rawData[i * 16 + 0].c_str()), atof(rawData[i * 16 + 1].c_str()), atof(rawData[i * 16 + 2].c_str()), atof(rawData[i * 16 + 3].c_str()),
					atof(rawData[i * 16 + 4].c_str()), atof(rawData[i * 16 + 5].c_str()), atof(rawData[i * 16 + 6].c_str()), atof(rawData[i * 16 + 7].c_str()),
					atof(rawData[i * 16 + 8].c_str()), atof(rawData[i * 16 + 9].c_str()), atof(rawData[i * 16 + 10].c_str()), atof(rawData[i * 16 + 11].c_str()),
					atof(rawData[i * 16 + 12].c_str()), atof(rawData[i * 16 + 13].c_str()), atof(rawData[i * 16 + 14].c_str()), atof(rawData[i * 16 + 15].c_str())));
			}
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

	return SkinningData(jointNames, vertexWeights, bindMatrices);
}

SkeletonData ColladaLoader::LoadSkeleton(tinyxml2::XMLElement * node, std::vector<std::string> jointOrder, std::vector<XMFLOAT4X4> inverseBindTransforms)
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

	JointData* rootJoint = LoadJointData(pHeadNode, true, jointOrder, inverseBindTransforms);

	return SkeletonData(jointOrder.size(), *rootJoint);
}

JointData* ColladaLoader::LoadJointData(tinyxml2::XMLElement * node, bool isRoot, std::vector<std::string> jointOrder, std::vector<XMFLOAT4X4> inverseBindTransforms)
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

	//XMMatrixTranspose(matrix);
	if (isRoot)
	{
		//rotate the root bone so that it is facing upwards
		//matrix = matrix * XMMatrixRotationX(-XM_PIDIV2);
	}

	XMFLOAT4X4 matrixAsFloats;
	XMStoreFloat4x4(&matrixAsFloats, matrix);
	JointData* joint = new JointData(index, nameId, matrixAsFloats);

	joint->inverseBindTransform = inverseBindTransforms[index];

	tinyxml2::XMLElement * childNode = node->FirstChildElement("node");
	while (childNode)
	{
		joint->AddChild(LoadJointData(childNode, false, jointOrder, inverseBindTransforms));
		childNode = childNode->NextSiblingElement("node");
	}

	return joint;
}

IndexedSkeletalModel ColladaLoader::LoadGeometry(tinyxml2::XMLElement * node, std::vector<VertexSkinData> vertexSkinData)
{
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

	if (!pPolyNode)
	{
		pPolyNode = pMeshNode->FirstChildElement("triangles");
	}

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

				//rotation.RotateVectorByQuaternion(position);
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
				//rotation.RotateVectorByQuaternion(normal);
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

		VertexData* currentVertex = &verts.at(positionIndex);
		if (!currentVertex->IsSet())
		{
			currentVertex->textureIndex = texCoordIndex;
			currentVertex->normalIndex = normalIndex;
			indices.push_back(positionIndex);
		}
		else
		{
			DealWithAlreadyProcessedVertex(currentVertex, texCoordIndex, normalIndex, indices, verts);
		}
	}

	//Remove unused vertices -------------------------------------------------------------------------------------------------------
	for (VertexData vertex : verts)
	{
		vertex.AverageTangents();

		if (!vertex.IsSet())
		{
			vertex.textureIndex = 0;
			vertex.normalIndex = 0;
		}
	}	

	SkeletalVertex* finalVerts = new SkeletalVertex[verts.size()];

	for (int i = 0; i < verts.size(); i++)
	{
		VertexData currentVertex = verts.at(i);

		finalVerts[i].PosL.x = verts.at(i).position.x;
		finalVerts[i].PosL.y = verts.at(i).position.y;
		finalVerts[i].PosL.z = verts.at(i).position.z;

		finalVerts[i].NormL = normals.at(currentVertex.normalIndex);

		XMFLOAT2 textureCoord = TexCoords.at(currentVertex.textureIndex);
		finalVerts[i].Tex.x = textureCoord.x;
		finalVerts[i].Tex.y = 1 - textureCoord.y;

		finalVerts[i].Weights = currentVertex.weightsData.GetWeights();

		finalVerts[i].BoneIndices = currentVertex.weightsData.GetBoneIndices();
	}

	

	unsigned short* indicesArray = new unsigned short[indices.size()];
	unsigned int numMeshIndices = indices.size();
	for (unsigned int i = 0; i < numMeshIndices; ++i)
	{
		indicesArray[i] = indices[i];
	}
	
	InsertTangentsIntoArray(finalVerts, indicesArray, verts.size());

	IndexedSkeletalModel indexedSkeletalModel;

	indexedSkeletalModel.Vertices.assign(&finalVerts[0], &finalVerts[verts.size()]);
	indexedSkeletalModel.Indices.assign(&indicesArray[0], &indicesArray[numMeshIndices]);

	return indexedSkeletalModel;
}

void ColladaLoader::DealWithAlreadyProcessedVertex(VertexData *previousVertex, int newTextureIndex, int newNormalIndex, std::vector<int> &indices, std::vector<VertexData> &verts)
{
	//vertex has already been processed
	if (previousVertex->HasSameTextureAndNormal(newTextureIndex, newNormalIndex))
	{
		indices.push_back(previousVertex->index);
	}
	else
	{
		VertexData* anotherVertex = previousVertex->GetDuplicateVertex();
		if (anotherVertex != nullptr)
		{
			return DealWithAlreadyProcessedVertex(anotherVertex, newTextureIndex, newNormalIndex, indices, verts);
		}
		else
		{
			VertexData* duplicateVertex = new VertexData(verts.size(), previousVertex->position, previousVertex->weightsData);
			duplicateVertex->textureIndex = newTextureIndex;
			duplicateVertex->normalIndex = newNormalIndex;
			previousVertex->duplicateVertex = duplicateVertex;
			verts.push_back(*duplicateVertex);
			indices.push_back(duplicateVertex->index);
		}
	}
}

void ColladaLoader::InsertTangentsIntoArray(SkeletalVertex * vertices, unsigned short* indices, int vertexCount)
{
	int faceCount, i, index;
	SkeletalVertex vertex1, vertex2, vertex3;
	XMFLOAT3 tangent;


	// Calculate the number of faces in the model.
	faceCount = vertexCount / 3;

	// Initialize the index to the model data.
	index = 0;

	// Go through all the faces and calculate the the tangent, binormal, and normal vectors.
	for (i = 0; i < faceCount; i++)
	{
		// Get the three vertices for this face from the model.
		vertex1.PosL.x = vertices[indices[index]].PosL.x;
		vertex1.PosL.y = vertices[indices[index]].PosL.y;
		vertex1.PosL.z = vertices[indices[index]].PosL.z;
		vertex1.Tex.x = vertices[indices[index]].Tex.x;
		vertex1.Tex.y = vertices[indices[index]].Tex.y;
		vertex1.NormL.x = vertices[indices[index]].NormL.x;
		vertex1.NormL.y = vertices[indices[index]].NormL.y;
		vertex1.NormL.z = vertices[indices[index]].NormL.z;
		index++;

		vertex2.PosL.x = vertices[indices[index]].PosL.x;
		vertex2.PosL.y = vertices[indices[index]].PosL.y;
		vertex2.PosL.z = vertices[indices[index]].PosL.z;
		vertex2.Tex.x = vertices[indices[index]].Tex.x;
		vertex2.Tex.y = vertices[indices[index]].Tex.y;
		vertex2.NormL.x = vertices[indices[index]].NormL.x;
		vertex2.NormL.y = vertices[indices[index]].NormL.y;
		vertex2.NormL.z = vertices[indices[index]].NormL.z;
		index++;

		vertex3.PosL.x = vertices[indices[index]].PosL.x;
		vertex3.PosL.y = vertices[indices[index]].PosL.y;
		vertex3.PosL.z = vertices[indices[index]].PosL.z;
		vertex3.Tex.x = vertices[indices[index]].Tex.x;
		vertex3.Tex.y = vertices[indices[index]].Tex.y;
		vertex3.NormL.x = vertices[indices[index]].NormL.x;
		vertex3.NormL.y = vertices[indices[index]].NormL.y;
		vertex3.NormL.z = vertices[indices[index]].NormL.z;
		index++;

		tangent = CalculateTangent(vertex1, vertex2, vertex3);

		// Store the tangent for this face back in the model structure.

		vertices[indices[index - 1]].Tangent.x = tangent.x;
		vertices[indices[index - 1]].Tangent.y = tangent.y;
		vertices[indices[index - 1]].Tangent.z = tangent.z;
								   
		vertices[indices[index - 2]].Tangent.x = tangent.x;
		vertices[indices[index - 2]].Tangent.y = tangent.y;
		vertices[indices[index - 2]].Tangent.z = tangent.z;
								   
		vertices[indices[index - 3]].Tangent.x = tangent.x;
		vertices[indices[index - 3]].Tangent.y = tangent.y;
		vertices[indices[index - 3]].Tangent.z = tangent.z;
	}
}

XMFLOAT3 ColladaLoader::CalculateTangent(SkeletalVertex v0, SkeletalVertex v1, SkeletalVertex v2)
{
	XMVECTOR vv0 = XMLoadFloat3(&v0.PosL);
	XMVECTOR vv1 = XMLoadFloat3(&v1.PosL);
	XMVECTOR vv2 = XMLoadFloat3(&v2.PosL);

	XMVECTOR e0 = vv1 - vv0;
	XMVECTOR e1 = vv2 - vv0;

	//using Eric Lengyel's approach with a few modifications
	//from Mathematics for 3D Game Programmming and Computer Graphics
	// want to be able to trasform a vector in Object Space to Tangent Space
	// such that the x-axis cooresponds to the 's' direction and the
	// y-axis corresponds to the 't' direction, and the z-axis corresponds
	// to <0,0,1>, straight up out of the texture map

	//let P = v1 - v0
	XMVECTOR P = vv1 - vv0;
	//let Q = v2 - v0
	XMVECTOR Q = vv2 - vv0;
	float s1 = v1.Tex.x - v0.Tex.x;
	float t1 = v1.Tex.y - v0.Tex.y;
	float s2 = v2.Tex.x - v0.Tex.x;
	float t2 = v2.Tex.y - v0.Tex.y;


	//we need to solve the equation
	// P = s1*T + t1*B
	// Q = s2*T + t2*B
	// for T and B


	//this is a linear system with six unknowns and six equatinos, for TxTyTz BxByBz
	//[px,py,pz] = [s1,t1] * [Tx,Ty,Tz]
	// qx,qy,qz     s2,t2     Bx,By,Bz

	//multiplying both sides by the inverse of the s,t matrix gives
	//[Tx,Ty,Tz] = 1/(s1t2-s2t1) *  [t2,-t1] * [px,py,pz]
	// Bx,By,Bz                      -s2,s1	    qx,qy,qz  

	//solve this for the unormalized T and B to get from tangent to object space

	float tmp = 0.0f;
	if (fabsf(s1*t2 - s2 * t1) <= 0.0001f)
	{
		tmp = 1.0f;
	}
	else
	{
		tmp = 1.0f / (s1*t2 - s2 * t1);
	}

	XMFLOAT3 PF3, QF3;
	XMStoreFloat3(&PF3, P);
	XMStoreFloat3(&QF3, Q);

	XMFLOAT3 tangent;
	tangent.x = (t2*PF3.x - t1 * QF3.x);
	tangent.y = (t2*PF3.y - t1 * QF3.y);
	tangent.z = (t2*PF3.z - t1 * QF3.z);

	tangent.x = tangent.x*tmp;
	tangent.y = tangent.y*tmp;
	tangent.z = tangent.z*tmp;

	XMVECTOR vt = XMLoadFloat3(&tangent);

	vt = XMVector3Normalize(vt);

	XMStoreFloat3(&tangent, vt);

	return tangent;
}
