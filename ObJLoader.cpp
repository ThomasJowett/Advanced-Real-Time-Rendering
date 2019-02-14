#include "ObJLoader.h"
#include <fstream>

IndexedModel OBJLoader::Load(const char * filename, bool invertCoordinates)
{
	std::string binaryFilename = filename;
	binaryFilename.append("Binary");
	std::ifstream binaryInFile;
	binaryInFile.open(binaryFilename, std::ios::in | std::ios::binary);

	if (!binaryInFile.good())
	{
		std::ifstream inFile;
		inFile.open(filename);

		if (!inFile.good())
		{
			return IndexedModel();
		}
		else
		{
			//Vectors to store the vertex positions, normals and texture coordinates. Need to use vectors since they're resizeable and we have
			//no way of knowing ahead of time how large these meshes will be
			std::vector<XMFLOAT3> verts;
			std::vector<XMFLOAT3> normals;
			std::vector<XMFLOAT2> TexCoords;

			//DirectX uses 1 index buffer, OBJ is optimized for storage and not rendering and so uses 3 smaller index buffers.....great...
			//We'll have to merge this into 1 index buffer which we'll do after loading in all of the required data.
			std::vector<unsigned short> vertIndices;
			std::vector<unsigned short> normalIndices;
			std::vector<unsigned short> textureIndices;

			std::string input;

			XMFLOAT3 vert;
			XMFLOAT2 TexCoord;
			XMFLOAT3 normal;
			unsigned short vInd[3]; //indices for the vertex position
			unsigned short tInd[3]; //indices for the texture coordinate
			unsigned short nInd[3]; //indices for the normal
			std::string beforeFirstSlash;
			std::string afterFirstSlash;
			std::string afterSecondSlash;

			while (!inFile.eof()) //While we have yet to reach the end of the file...
			{
				inFile >> input; //Get the next input from the file

								 //Check what type of input it was, we are only interested in vertex positions, texture coordinates, normals and indices, nothing else
				if (input.compare("v") == 0) //Vertex position
				{
					inFile >> vert.x;
					inFile >> vert.y;
					inFile >> vert.z;

					verts.push_back(vert);
				}
				else if (input.compare("vt") == 0) //Texture coordinate
				{
					inFile >> TexCoord.x;
					inFile >> TexCoord.y;

					//diferent modeling software stores the texture coordinates differently

					if (invertCoordinates)
					{
						TexCoord.y = 1.0f - TexCoord.y;
					}

					TexCoords.push_back(TexCoord);
				}
				else if (input.compare("vn") == 0) //Normal
				{
					inFile >> normal.x;
					inFile >> normal.y;
					inFile >> normal.z;

					normals.push_back(normal);
				}
				else if (input.compare("f") == 0) //Face
				{
					for (int i = 0; i < 3; ++i)
					{
						inFile >> input;
						int slash = input.find("/"); //Find first forward slash
						int secondSlash = input.find("/", slash + 1); //Find second forward slash

																	  //Extract from string
						beforeFirstSlash = input.substr(0, slash); //The vertex position index
						afterFirstSlash = input.substr(slash + 1, secondSlash - slash - 1); //The texture coordinate index
						afterSecondSlash = input.substr(secondSlash + 1); //The normal index

																		  //Parse into int
						vInd[i] = (unsigned short)atoi(beforeFirstSlash.c_str()); //atoi = "ASCII to int"
						tInd[i] = (unsigned short)atoi(afterFirstSlash.c_str());
						nInd[i] = (unsigned short)atoi(afterSecondSlash.c_str());
					}

					//Place into vectors
					for (int i = 0; i < 3; ++i)
					{
						vertIndices.push_back(vInd[i] - 1);		//Minus 1 from each as these as OBJ indexes start from 1 whereas C++ arrays start from 0
						textureIndices.push_back(tInd[i] - 1);	//which is really annoying. Apart from Lua and SQL, there's not much else that has indexing 
						normalIndices.push_back(nInd[i] - 1);	//starting at 1. So many more languages index from 0, the .OBJ people screwed up there.
					}
				}
			}
			inFile.close(); //Finished with input file now, all the data we need has now been loaded in

			std::vector<XMFLOAT3> expandedVertices;
			std::vector<XMFLOAT3> expandedNormals;
			std::vector<XMFLOAT2> expandedTexCoords;
			unsigned int numIndices = vertIndices.size();
			for (unsigned int i = 0; i < numIndices; i++)
			{
				expandedVertices.push_back(verts[vertIndices[i]]);
				expandedTexCoords.push_back(TexCoords[textureIndices[i]]);
				expandedNormals.push_back(normals[normalIndices[i]]);
			}

			//Now to (finally) form the final vertex, texture coord, normal list and single index buffer using the above expanded vectors
			std::vector<unsigned short> meshIndices;
			meshIndices.reserve(numIndices);
			std::vector<XMFLOAT3> meshVertices;
			meshVertices.reserve(expandedVertices.size());
			std::vector<XMFLOAT3> meshNormals;
			meshNormals.reserve(expandedNormals.size());
			std::vector<XMFLOAT2> meshTexCoords;
			meshTexCoords.reserve(expandedTexCoords.size());

			CreateIndices(expandedVertices, expandedTexCoords, expandedNormals, meshIndices, meshVertices, meshTexCoords, meshNormals);


			//Turn data from vector form to arrays
			SimpleVertex* finalVerts = new SimpleVertex[meshVertices.size()];
			unsigned int numMeshVertices = meshVertices.size();
			for (unsigned int i = 0; i < numMeshVertices; ++i)
			{
				finalVerts[i].PosL = meshVertices[i];
				finalVerts[i].NormL = meshNormals[i];
				finalVerts[i].Tex = meshTexCoords[i];
			}

			InsertTangentsIntoArray(finalVerts, numMeshVertices);

			unsigned short* indicesArray = new unsigned short[meshIndices.size()];
			unsigned int numMeshIndices = meshIndices.size();
			for (unsigned int i = 0; i < numMeshIndices; ++i)
			{
				indicesArray[i] = meshIndices[i];
			}

			//Create the indexed model
			IndexedModel returnGeometry;

			returnGeometry.Vertices.assign(&finalVerts[0], &finalVerts[numMeshVertices]);
			returnGeometry.Indices.assign(&indicesArray[0], &indicesArray[numMeshIndices]);

			//Output data into binary file, the next time you run this function, the binary file will exist and will load that instead which is much quicker than parsing into vectors
			std::ofstream outbin(binaryFilename.c_str(), std::ios::out | std::ios::binary);
			outbin.write((char*)&numMeshVertices, sizeof(unsigned int));
			outbin.write((char*)&numMeshIndices, sizeof(unsigned int));
			outbin.write((char*)finalVerts, sizeof(SimpleVertex) * numMeshVertices);
			outbin.write((char*)indicesArray, sizeof(unsigned short) * numMeshIndices);
			outbin.close();

			delete[] indicesArray;
			delete[] finalVerts;

			return returnGeometry;
		}
	}
	else
	{
		IndexedModel returnGeometry;

		unsigned int numVertices;
		unsigned int numIndices;

		//Read in array sizes
		binaryInFile.read((char*)&numVertices, sizeof(unsigned int));
		binaryInFile.read((char*)&numIndices, sizeof(unsigned int));

		//Read in data from binary file
		SimpleVertex* finalVerts = new SimpleVertex[numVertices];
		unsigned short* indices = new unsigned short[numIndices];
		binaryInFile.read((char*)finalVerts, sizeof(SimpleVertex) * numVertices);
		binaryInFile.read((char*)indices, sizeof(unsigned short) * numIndices);

		returnGeometry.Vertices.assign(&finalVerts[0], &finalVerts[numVertices]);
		returnGeometry.Indices.assign(&indices[0], &indices[numIndices]);

		delete[] indices;
		delete[] finalVerts;

		return returnGeometry;
	}
			
}

bool OBJLoader::FindSimilarVertex(const SimpleVertex & vertex, std::map<SimpleVertex, unsigned short>& vertToIndexMap, unsigned short & index)
{
	auto it = vertToIndexMap.find(vertex);

	if (it == vertToIndexMap.end())
	{
		return false;
	}
	else
	{
		index = it->second;
		return true;
	}
}

void OBJLoader::CreateIndices(const std::vector<XMFLOAT3>& inVertices, 
	const std::vector<XMFLOAT2>& inTexCoords, 
	const std::vector<XMFLOAT3>& inNormals, 
	std::vector<unsigned short>& outIndices, 
	std::vector<XMFLOAT3>& outVertices, 
	std::vector<XMFLOAT2>& outTexCoords, 
	std::vector<XMFLOAT3>& outNormals)
{
	// Mapping from an already-existing SimpleVertex to its corresponding index
	std::map<SimpleVertex, unsigned short> vertToIndexMap;

	std::pair<SimpleVertex, unsigned short> pair;

	int numVertices = inVertices.size();

	for (int i = 0; i < numVertices; ++i) //For each vertex
	{
		SimpleVertex vertex = { inVertices[i], inNormals[i], XMFLOAT3(), inTexCoords[i] };

		unsigned short index;
		// See if a vertex already exists in the buffer that has the same attributes as this one
		bool found = FindSimilarVertex(vertex, vertToIndexMap, index);
		
		if (found) //if found, re-use it's index for the index buffer
		{
			outIndices.push_back(index);
		}
		else //if not found, add it to the buffer
		{
			outVertices.push_back(vertex.PosL);
			outTexCoords.push_back(vertex.Tex);
			outNormals.push_back(vertex.NormL);

			unsigned short newIndex = (unsigned short)outVertices.size() - 1;

			outIndices.push_back(newIndex);

			//Add it to the map
			pair.first = vertex;
			pair.second = newIndex;

			//vertToIndexMap.insert(pair);
		}
	}
}

void OBJLoader::InsertTangentsIntoArray(SimpleVertex* vertices, int vertexCount)
{
	int faceCount, i, index;
	SimpleVertex vertex1, vertex2, vertex3;
	XMFLOAT3 tangent;


	// Calculate the number of faces in the model.
	faceCount = vertexCount / 3;

	// Initialize the index to the model data.
	index = 0;

	// Go through all the faces and calculate the the tangent, binormal, and normal vectors.
	for (i = 0; i<faceCount; i++)
	{
		if (i == 8)
		{
			int x = 1;
		}
		// Get the three vertices for this face from the model.
		vertex1.PosL.x = vertices[index].PosL.x;
		vertex1.PosL.y = vertices[index].PosL.y;
		vertex1.PosL.z = vertices[index].PosL.z;
		vertex1.Tex.x = vertices[index].Tex.x;
		vertex1.Tex.y = vertices[index].Tex.y;
		vertex1.NormL.x = vertices[index].NormL.x;
		vertex1.NormL.y = vertices[index].NormL.y;
		vertex1.NormL.z = vertices[index].NormL.z;
		index++;

		vertex2.PosL.x = vertices[index].PosL.x;
		vertex2.PosL.y = vertices[index].PosL.y;
		vertex2.PosL.z = vertices[index].PosL.z;
		vertex2.Tex.x = vertices[index].Tex.x;
		vertex2.Tex.y = vertices[index].Tex.y;
		vertex2.NormL.x = vertices[index].NormL.x;
		vertex2.NormL.y = vertices[index].NormL.y;
		vertex2.NormL.z = vertices[index].NormL.z;
		index++;

		vertex3.PosL.x = vertices[index].PosL.x;
		vertex3.PosL.y = vertices[index].PosL.y;
		vertex3.PosL.z = vertices[index].PosL.z;
		vertex3.Tex.x = vertices[index].Tex.x;
		vertex3.Tex.y = vertices[index].Tex.y;
		vertex3.NormL.x = vertices[index].NormL.x;
		vertex3.NormL.y = vertices[index].NormL.y;
		vertex3.NormL.z = vertices[index].NormL.z;
		index++;

		tangent = CalculateTangent(vertex1, vertex2, vertex3);

		// Store the normal, tangent, and binormal for this face back in the model structure.
		
		vertices[index - 1].Tangent.x = tangent.x;
		vertices[index - 1].Tangent.y = tangent.y;
		vertices[index - 1].Tangent.z = tangent.z;

		vertices[index - 2].Tangent.x = tangent.x;
		vertices[index - 2].Tangent.y = tangent.y;
		vertices[index - 2].Tangent.z = tangent.z;
							
		vertices[index - 3].Tangent.x = tangent.x;
		vertices[index - 3].Tangent.y = tangent.y;
		vertices[index - 3].Tangent.z = tangent.z;
	}
}

XMFLOAT3 OBJLoader::CalculateTangent(SimpleVertex v0, SimpleVertex v1, SimpleVertex v2)
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
