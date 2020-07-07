#include "Framework.h"
#include "MeshSphere.h"

MeshSphere::MeshSphere(Shader * shader, float radius, UINT stackCount, UINT sliceCount)
	: Mesh(shader), radius(radius), stackCount(stackCount), sliceCount(sliceCount)
{

}

MeshSphere::~MeshSphere()
{

}

void MeshSphere::Create()
{
	vector<MeshVertex> v;
	v.push_back(MeshVertex(0, radius,0,0, 0, 0, 1, 0,1,0,0));

	//파이를 스텍카운트로 나눠서 만들 스택구함
	float phiStep = Math::PI / stackCount;
	//잘린원을 그리기위한 한단계 각도
	float thetaStep = 2.0f * Math::PI / sliceCount;
	// 각 스택 링에 대한 정점을 계산
	for (UINT i = 1; i <= stackCount - 1; i++)
	{
		float phi = i * phiStep;
		//링의 정점들
		for (UINT j = 0; j <= sliceCount; j++)
		{
			//세타
			float theta = j * thetaStep;
			//정점의 위치
			D3DXVECTOR3 p = D3DXVECTOR3 //포지션 구면좌표계
			(
				(radius * sinf(phi) * cosf(theta)),
				(radius * cosf(phi)),
				(radius * sinf(phi) * sinf(theta))
			);

			//택스쳐 맵핑할때 쓰는것?
			D3DXVECTOR3 t = D3DXVECTOR3
			(
				-radius * sinf(phi) * sinf(theta),
				0,
				radius * sinf(phi) * cosf(theta)
			);
			//노말라이즈
			D3DXVec3Normalize(&t, &t);
			//포지션에서 나오는 방향을 저장
			D3DXVECTOR3 n;
			D3DXVec3Normalize(&n, &p);
			D3DXVECTOR2 uv = D3DXVECTOR2(theta / (Math::PI * 2), phi / Math::PI);
			v.push_back(MeshVertex(p.x, p.y, p.z, uv.x, uv.y, n.x, n.y, n.z,t.x,t.y,t.z));
		}
	}
	v.push_back(MeshVertex(0, -radius,0,0,0, 0, -1, 0,-1,0,0));

	vertices = new MeshVertex[v.size()];
	vertexCount = v.size();
	//벡터의 데이터를 배열로 카피
	copy(v.begin(), v.end(), stdext::checked_array_iterator<MeshVertex*>(vertices, vertexCount));

	// 최상위 스택에 대한 인덱스 계산 상단 스택이 정점 버퍼에 먼저 기록됨
   // 상단 폴을 첫 번째 링에 연결
	vector<UINT> indices;
	for (UINT i = 1; i <= sliceCount; i++)
	{
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i);
	}

	// 내부 스택에 대한 인덱스 계산(극에 연결되지 않음)
    // 첫 번째 링의 첫 번째 정점 인덱스에 대한 인덱스 간격띄우기
    // 꼭대기 기둥 정점을 건너뜀
	UINT baseIndex = 1;
	UINT ringVertexCount = sliceCount + 1;
	for (UINT i = 0; i < stackCount - 2; i++)
	{
		for (UINT j = 0; j < sliceCount; j++)
		{
			indices.push_back(baseIndex + i * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

    // 하단 스택에 대한 인덱스 계산 하단 스택이 정점 버퍼에 마지막으로 기록됨
    // 하단 폴을 하단 링에 연결
    // 남극 정점이 마지막으로 추가
	UINT southPoleIndex = v.size() - 1;
	// 마지막 링의 첫 번째 정점 인덱스에 대한 인덱스 간격띄우기
	baseIndex = southPoleIndex - ringVertexCount;
	
	//맨 아래 정점과 그 위의 원을 연결
	for (UINT i = 0; i < sliceCount; i++)
	{
		indices.push_back(southPoleIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}

	this->indices = new UINT[indices.size()];
	indexCount = indices.size();
	sphereIndex = indexCount;
	copy(indices.begin(), indices.end(), stdext::checked_array_iterator<UINT *>(this->indices, indexCount));
}