#include "GeometryGenerator.h"
#include "Vector.h"

IndexedModel GeometryGenerator::CreateCube(float width, float height, float depth)
{
	IndexedModel returnModel;

	float w2 = 0.5f*width;
	float h2 = 0.5f*height;
	float d2 = 0.5f*depth;

	SimpleVertex vertices[24] =
	{
		// Fill in the front face vertex data.
		SimpleVertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f),
		SimpleVertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f),
		SimpleVertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
		SimpleVertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f),

		// Fill in the back face vertex data.
		SimpleVertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f),
		SimpleVertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f),
		SimpleVertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f),
		SimpleVertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f),

		// Fill in the top face vertex data.
		SimpleVertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f),
		SimpleVertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f),
		SimpleVertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
		SimpleVertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f),

		// Fill in the bottom face vertex data.
		SimpleVertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f),
		SimpleVertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f),
		SimpleVertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f),
		SimpleVertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f),

		// Fill in the left face vertex data.
		SimpleVertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f),
		SimpleVertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		SimpleVertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f),
		SimpleVertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f),

		// Fill in the right face vertex data.
		SimpleVertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f),
		SimpleVertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f),
		SimpleVertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f),
		SimpleVertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f)
	};

	returnModel.Vertices.assign(&vertices[0], &vertices[24]);

	WORD indices[] =
	{
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23
	};

	returnModel.Indices.assign(&indices[0], &indices[36]);

	return returnModel;
}

IndexedModel GeometryGenerator::CreateSphere(float radius, unsigned int longitudeLines, unsigned int latitudeLines)
{
	IndexedModel returnGeometry;

	if (longitudeLines < 3)
		longitudeLines = 3;
	if (latitudeLines < 2)
		latitudeLines = 2;

	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	SimpleVertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	SimpleVertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	returnGeometry.Vertices.push_back(topVertex);

	float phiStep = XM_PI / latitudeLines;
	float thetaStep = 2.0f*XM_PI / longitudeLines;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (int i = 1; i <= latitudeLines - 1; ++i)
	{
		float phi = i * phiStep;

		// Vertices of ring.
		for (int j = 0; j <= longitudeLines; ++j)
		{
			float theta = j * thetaStep;

			SimpleVertex v;

			// spherical to cartesian
			v.PosL.x = radius * sinf(phi)*cosf(theta);
			v.PosL.y = radius * cosf(phi);
			v.PosL.z = radius * sinf(phi)*sinf(theta);

			// Partial derivative of P with respect to theta
			v.Tangent.x = -radius * sinf(phi)*sinf(theta);
			v.Tangent.y = 0.0f;
			v.Tangent.z = +radius * sinf(phi)*cosf(theta);

			XMVECTOR T = XMLoadFloat3(&v.Tangent);
			XMStoreFloat3(&v.Tangent, XMVector3Normalize(T));

			XMVECTOR p = XMLoadFloat3(&v.PosL);
			XMStoreFloat3(&v.NormL, XMVector3Normalize(p));

			v.Tex.x = theta / XM_2PI;
			v.Tex.y = phi / XM_PI;

			returnGeometry.Vertices.push_back(v);
		}
	}

	returnGeometry.Vertices.push_back(bottomVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (WORD i = 1; i <= longitudeLines; ++i)
	{
		returnGeometry.Indices.push_back(0);
		returnGeometry.Indices.push_back(i + 1);
		returnGeometry.Indices.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	WORD baseIndex = 1;
	WORD ringVertexCount = longitudeLines + 1;
	for (WORD i = 0; i < latitudeLines - 2; ++i)
	{
		for (WORD j = 0; j < longitudeLines; ++j)
		{
			returnGeometry.Indices.push_back(baseIndex + i * ringVertexCount + j);
			returnGeometry.Indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			returnGeometry.Indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);

			returnGeometry.Indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);
			returnGeometry.Indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			returnGeometry.Indices.push_back(baseIndex + (i + 1)*ringVertexCount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	int southPoleIndex = (int)returnGeometry.Vertices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (int i = 0; i < longitudeLines; ++i)
	{
		returnGeometry.Indices.push_back(southPoleIndex);
		returnGeometry.Indices.push_back(baseIndex + i);
		returnGeometry.Indices.push_back(baseIndex + i + 1);
	}

	return returnGeometry;
}

IndexedModel GeometryGenerator::CreateGrid(float width, float length, unsigned int widthLines, unsigned int lengthLines, float tileU, float tileV)
{
	IndexedModel returnModel;

	if (widthLines < 2)
		widthLines == 2;
	if (lengthLines < 2)
		lengthLines == 2;

	int vertexCount = widthLines * lengthLines;
	int faceCount = (widthLines - 1)*(lengthLines - 1) * 2;

	//
	// Create the vertices.
	//

	float halfWidth = 0.5f*width;
	float halfDepth = 0.5f*length;

	float dx = width / (lengthLines - 1);
	float dz = length / (widthLines - 1);

	float du = tileU / (lengthLines - 1);
	float dv = tileV / (widthLines - 1);

	returnModel.Vertices.resize(vertexCount);

	for (int i = 0; i < widthLines; ++i)
	{
		float z = halfDepth - i * dz;
		for (int j = 0; j < lengthLines; ++j)
		{
			float x = -halfWidth + j * dx;

			returnModel.Vertices[i*lengthLines + j].PosL = XMFLOAT3(x, 0.0f, z);
			returnModel.Vertices[i*lengthLines + j].NormL = XMFLOAT3(0.0f, 1.0f, 0.0f);
			returnModel.Vertices[i*lengthLines + j].Tangent = XMFLOAT3(1.0f, 0.0f, 0.0f);

			// Stretch texture over grid.
			returnModel.Vertices[i*lengthLines + j].Tex.x = j * du;
			returnModel.Vertices[i*lengthLines + j].Tex.y = i * dv;
		}
	}

	//
	// Create the indices.
	//

	returnModel.Indices.resize(faceCount * 3); // 3 indices per face

												// Iterate over each quad and compute indices.
	WORD k = 0;
	for (WORD i = 0; i < lengthLines - 1; ++i)
	{
		for (WORD j = 0; j < widthLines - 1; ++j)
		{
			returnModel.Indices[k] = i * lengthLines + j;
			returnModel.Indices[k + 1] = i * lengthLines + j + 1;
			returnModel.Indices[k + 2] = (i + 1)*lengthLines + j;

			returnModel.Indices[k + 3] = (i + 1)*lengthLines + j;
			returnModel.Indices[k + 4] = i * lengthLines + j + 1;
			returnModel.Indices[k + 5] = (i + 1)*lengthLines + j + 1;

			k += 6; // next quad
		}
	}

	return returnModel;
}

IndexedModel GeometryGenerator::CreateFullScreenQuad()
{
	IndexedModel returnModel;

	returnModel.Vertices.resize(4);
	returnModel.Indices.resize(6);

	// Position coordinates specified in NDC space.
	returnModel.Vertices[0] = SimpleVertex(
		-1.f, -1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f);

	returnModel.Vertices[1] = SimpleVertex(
		-1.0f, +1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f);

	returnModel.Vertices[2] = SimpleVertex(
		+1.0f, +1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f);

	returnModel.Vertices[3] = SimpleVertex(
		+1.0f, -1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f);

	returnModel.Indices[0] = 0;
	returnModel.Indices[1] = 1;
	returnModel.Indices[2] = 2;

	returnModel.Indices[3] = 0;
	returnModel.Indices[4] = 2;
	returnModel.Indices[5] = 3;

	return returnModel;
}

IndexedModel GeometryGenerator::CreateCylinder(float bottomRadius, float topRadius, float height, int sliceCount, int stackCount)
{
	IndexedModel returnGeometry;

	//Build Stacks

	float stackHeight = height / stackCount;

	// Amount to increment radius as we move up each stack level from bottom to top.
	float radiusStep = (topRadius - bottomRadius) / stackCount;

	unsigned int ringCount = stackCount + 1;

	// Compute vertices for each stack ring starting at the bottom and moving up.
	for (UINT i = 0; i < ringCount; ++i)
	{
		float y = -0.5f*height + i * stackHeight;
		float r = bottomRadius + i * radiusStep;

		// vertices of ring
		float dTheta = 2.0f*XM_PI / sliceCount;
		for (int j = 0; j <= sliceCount; ++j)
		{
			SimpleVertex vertex;

			float c = cosf(j*dTheta);
			float s = sinf(j*dTheta);

			vertex.PosL = XMFLOAT3(r*c, y, r*s);

			vertex.Tex.x = (float)j / sliceCount;
			vertex.Tex.y = 1.0f - (float)i / stackCount;

			// Cylinder can be parameterized as follows, where we introduce v
			// parameter that goes in the same direction as the v tex-coord
			// so that the bitangent goes in the same direction as the v tex-coord.
			//   Let r0 be the bottom radius and let r1 be the top radius.
			//   y(v) = h - hv for v in [0,1].
			//   r(v) = r1 + (r0-r1)v
			//
			//   x(t, v) = r(v)*cos(t)
			//   y(t, v) = h - hv
			//   z(t, v) = r(v)*sin(t)
			// 
			//  dx/dt = -r(v)*sin(t)
			//  dy/dt = 0
			//  dz/dt = +r(v)*cos(t)
			//
			//  dx/dv = (r0-r1)*cos(t)
			//  dy/dv = -h
			//  dz/dv = (r0-r1)*sin(t)

			// This is unit length.
			vertex.Tangent = XMFLOAT3(-s, 0.0f, c);

			float dr = bottomRadius - topRadius;
			XMFLOAT3 bitangent(dr*c, -height, dr*s);

			XMVECTOR T = XMLoadFloat3(&vertex.Tangent);
			XMVECTOR B = XMLoadFloat3(&bitangent);
			XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, B));
			XMStoreFloat3(&vertex.NormL, N);

			returnGeometry.Vertices.push_back(vertex);
		}
	}

	// Add one because we duplicate the first and last vertex per ring
	// since the texture coordinates are different.
	unsigned int ringVertexCount = sliceCount + 1;

	// Compute indices for each stack.
	for (WORD i = 0; i < stackCount; ++i)
	{
		for (WORD j = 0; j < sliceCount; ++j)
		{
			returnGeometry.Indices.push_back(i*ringVertexCount + j);
			returnGeometry.Indices.push_back((i + 1)*ringVertexCount + j);
			returnGeometry.Indices.push_back((i + 1)*ringVertexCount + j + 1);

			returnGeometry.Indices.push_back(i*ringVertexCount + j);
			returnGeometry.Indices.push_back((i + 1)*ringVertexCount + j + 1);
			returnGeometry.Indices.push_back(i*ringVertexCount + j + 1);
		}
	}

	//build top cap

	WORD baseIndex = (WORD)returnGeometry.Vertices.size();

	float y = 0.5f*height;
	float dTheta = 2.0f*XM_PI / sliceCount;

	// Duplicate cap ring vertices because the texture coordinates and normals differ.
	for (UINT i = 0; i <= sliceCount; ++i)
	{
		float x = topRadius * cosf(i*dTheta);
		float z = topRadius * sinf(i*dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		returnGeometry.Vertices.push_back(SimpleVertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	// Cap center vertex.
	returnGeometry.Vertices.push_back(SimpleVertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	// Index of center vertex.
	WORD centerIndex = (WORD)returnGeometry.Vertices.size() - 1;

	for (WORD i = 0; i < sliceCount; ++i)
	{
		returnGeometry.Indices.push_back(centerIndex);
		returnGeometry.Indices.push_back(baseIndex + i + 1);
		returnGeometry.Indices.push_back(baseIndex + i);
	}

	//build bottom cap

	baseIndex = (WORD)returnGeometry.Vertices.size();
	y = -0.5f*height;

	// vertices of ring
	dTheta = 2.0f*XM_PI / sliceCount;
	for (UINT i = 0; i <= sliceCount; ++i)
	{
		float x = bottomRadius * cosf(i*dTheta);
		float z = bottomRadius * sinf(i*dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		returnGeometry.Vertices.push_back(SimpleVertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	// Cap center vertex.
	returnGeometry.Vertices.push_back(SimpleVertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	// Cache the index of center vertex.
	centerIndex = (WORD)returnGeometry.Vertices.size() - 1;

	for (WORD i = 0; i < sliceCount; ++i)
	{
		returnGeometry.Indices.push_back(centerIndex);
		returnGeometry.Indices.push_back(baseIndex + i);
		returnGeometry.Indices.push_back(baseIndex + i + 1);
	}

	return returnGeometry;
}

IndexedModel GeometryGenerator::CreateTorus(float diameter, float thickness, int segments)
{
	if (segments < 3)
		segments = 3;

	IndexedModel returnGeometry;

	size_t stride = segments + 1;

	for (int i = 0; i <= segments; i++)
	{
		float u = float(i) / segments;

		float outerAngle = i * XM_2PI / segments - XM_PIDIV2;

		XMMATRIX transform = XMMatrixTranslation(diameter / 2, 0, 0) * XMMatrixRotationY(outerAngle);

		for (int j = 0; j <= segments; j++)
		{
			float v = 1 - float(j) / segments;

			float innerAngle = j * XM_2PI / segments + XM_PI;
			float dx, dy;

			XMScalarSinCos(&dy, &dx, innerAngle);

			XMVECTOR normal = XMVectorSet(dx, dy, 0, 0);
			XMVECTOR tangent = XMVectorSet(dy, -dx, 0, 0);
			XMVECTOR position = XMVectorScale(normal, thickness / 2);
			XMVECTOR texCoord = XMVectorSet(u, v, 0, 0);

			position = XMVector3Transform(position, transform);
			normal = XMVector3TransformNormal(normal, transform);
			tangent = XMVector3TransformNormal(tangent, transform);

			returnGeometry.Vertices.push_back(SimpleVertex(position,normal,tangent, texCoord));

			// And create indices for two triangles.
			size_t nextI = (i + 1) % stride;
			size_t nextJ = (j + 1) % stride;
			
			returnGeometry.Indices.push_back(nextI * stride + j);
			returnGeometry.Indices.push_back(i * stride + nextJ);
			returnGeometry.Indices.push_back(i * stride + j);

			returnGeometry.Indices.push_back(nextI * stride + j);
			returnGeometry.Indices.push_back(nextI * stride + nextJ);
			returnGeometry.Indices.push_back(i * stride + nextJ);
		}
	}
	return returnGeometry;
}
