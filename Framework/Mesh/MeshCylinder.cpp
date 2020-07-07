#include "Framework.h"
#include "MeshCylinder.h"

MeshCylinder::MeshCylinder(Shader * shader, float radius, float height, UINT sliceCount, UINT stackCount)
	: Mesh(shader)
	, topRadius(radius), bottomRadius(radius), height(height), sliceCount(sliceCount), stackCount(stackCount)
{

}

MeshCylinder::~MeshCylinder()
{

}

void MeshCylinder::Create()
{
	vector<MeshVertex> vertices1;

	// 스택 쌓기. 전체 길이를 스택만큼 나눠서 만들 스택의 수를 구한다
	float stackHeight = height / (float)stackCount;

	//각 스택 레벨을 아래에서 위로 올릴 때 반경을 증가시킬 양.
	float radiusStep = (topRadius - bottomRadius) / (float)stackCount;

	//
	UINT ringCount = stackCount + 1;
	//하단에서 시작하여 위로 이동하는 각 스택 링에 대한 정점을 계산
	for (UINT i = 0; i < ringCount; i++)
	{
		//각 스택 레벨별 높이 
		float y = -0.5f * height + i * stackHeight;
		//각 스택 레벨별 반지름
		float r = bottomRadius + i * radiusStep;
		//각 스택 레벨별 각도
		float theta = 2.0f * Math::PI / (float)sliceCount;

		for (UINT k = 0; k <= sliceCount; k++)
		{
			//각 스텍레벨별 원을 구성할 정점들 계산
			float c = cosf(k * theta);
			float s = sinf(k * theta);
			
			//실린더는 다음과 같이 매개변수를 지정할 수 있으며, 여기서 v를 소개한다.
			//	v tex - coord와 동일한 방향으로 이동하는 매개 변수
			//	bit이 v tex - coord와 같은 방향으로 가도록 한다.
			//	r0은 하단 반경이 되도록 하고 r1은 상단 반경이 되도록 한다.
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

			
			MeshVertex vertex;
			vertex.Position = D3DXVECTOR3(r * c, y, r * s);
			vertex.Uv = D3DXVECTOR2((float)k / (float)sliceCount, 1.0f - (float)i / (float)stackCount);
			
			//버텍스 포지션에 계산한 위치값 대입

			//유닛 길이 정점좌표와 수직인 단위벡터
			vertex.Tangent = D3DXVECTOR3(-s, 0.0f, c);

			float dr = bottomRadius - topRadius;
			D3DXVECTOR3 biTangent = D3DXVECTOR3(dr * c, -height, dr * s);

			D3DXVec3Cross(&vertex.Normal, &vertex.Tangent, &biTangent);
			D3DXVec3Normalize(&vertex.Normal, &vertex.Normal);

			vertices1.push_back(vertex);
		}
	}


	vector<UINT> indices;
	//링당 첫 번째 정점과 마지막 정점을 복제하기 때문에 정점 추가
	UINT ringVertexCount = sliceCount + 1;
	for (UINT y = 0; y < stackCount; y++)
	{
		for (UINT x = 0; x < sliceCount; x++)
		{
			indices.push_back(y * ringVertexCount + x);
			indices.push_back((y + 1) * ringVertexCount + x);
			indices.push_back((y + 1) * ringVertexCount + (x + 1));

			indices.push_back(y * ringVertexCount + x);
			indices.push_back((y + 1) * ringVertexCount + x + 1);
			indices.push_back(y * ringVertexCount + x + 1);
		}
	}

	BuildTopCap(vertices1, indices);
	BuildBottomCap(vertices1, indices);


	this->vertices = new MeshVertex[vertices1.size()];
	vertexCount = vertices1.size();
	copy(vertices1.begin(), vertices1.end(), stdext::checked_array_iterator<MeshVertex *>(this->vertices, vertexCount));

	this->indices = new UINT[indices.size()];
	indexCount = indices.size();
	copy(indices.begin(), indices.end(), stdext::checked_array_iterator<UINT *>(this->indices, indexCount));
}

void MeshCylinder::BuildTopCap(vector<MeshVertex>& vertices, vector<UINT>& indices)
{
	float y = 0.5f * height;
	float theta = 2.0f * Math::PI / (float)sliceCount;

	for (UINT i = 0; i <= sliceCount; i++)
	{
		float x = topRadius * cosf(i * theta);
		float z = topRadius * sinf(i * theta);

		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		vertices.push_back(MeshVertex(x, y, z,u,v, 0, 1, 0,1,0,0));
	}
	vertices.push_back(MeshVertex(0, y, 0,0.5f,0.5f, 0, 1, 0,1,0,0));


	UINT baseIndex = vertices.size() - sliceCount - 2;
	UINT centerIndex = vertices.size() - 1;

	for (UINT i = 0; i < sliceCount; i++)
	{
		indices.push_back(centerIndex);
		indices.push_back(baseIndex + i + 1);
		indices.push_back(baseIndex + i);
	}
}

void MeshCylinder::BuildBottomCap(vector<MeshVertex>& vertices, vector<UINT>& indices)
{
	float y = -0.5f * height;
	float theta = 2.0f * Math::PI / (float)sliceCount;

	for (UINT i = 0; i <= sliceCount; i++)
	{
		float x = bottomRadius * cosf(i * theta);
		float z = bottomRadius * sinf(i * theta);

		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		vertices.push_back(MeshVertex(x, y, z,u,v, 0, -1, 0,-1,0,0));
	}
	vertices.push_back(MeshVertex(0, y, 0,0.5f,0.5f, 0, -1, 0,-1,0,0));


	UINT baseIndex = vertices.size() - sliceCount - 2;
	UINT centerIndex = vertices.size() - 1;

	for (UINT i = 0; i < sliceCount; i++)
	{
		indices.push_back(centerIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}
}