#include "Framework.h"
#include "MeshSphereInstance.h"

MeshSphereInstance::MeshSphereInstance()
	:Renderer(L"024_Instance.fx")
	, drawCount(0)
	
{
	for (uint i = 0; i < MAX_INSTANCE; i++)
	{
		//transforms[i] = new Transform(shader);
		transforms[i] = new Transform();
		D3DXMatrixIdentity(&instdesc[i].worlds);
		
	}

	CreateSphereVertex();

	vertexBuffer = new VertexBuffer(v.data(), v.size(), sizeof(MeshVertex));
    indexBuffer = new IndexBuffer(indices.data(), indices.size());
	randomBuffer = new ConstantBuffer(&colorDesc, sizeof(ColorDesc));
	sRandomBuffer = shader->AsConstantBuffer("CB_randomColor");

	instanceBuffer = new VertexBuffer(&instdesc, MAX_INSTANCE, sizeof(instDesc), 1, true);

	
	vector<wstring> names;
	names.emplace_back(L"Green.png");
	names.emplace_back(L"Magenta.png");
	names.emplace_back(L"Red.png");
	names.emplace_back(L"White.png");
	names.emplace_back(L"Yellow.png");
	names.emplace_back(L"Box.png");
	textures = new TextureArray(names);

	//worlds를 기준으로 인스턴스 한다
}

MeshSphereInstance::~MeshSphereInstance()
{
}

void MeshSphereInstance::Update()
{
	Super::Update();


	for (uint i = 0; i < drawCount; i++)
	{
		memcpy(&instdesc[i].worlds, &transforms[i]->World(), sizeof(Matrix));

	}
	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(instanceBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, &instdesc, sizeof(instDesc)*MAX_INSTANCE);
	}
	D3D::GetDC()->Unmap(instanceBuffer->Buffer(), 0);


	Vector3 color;
	color.x = Math::Random(0.0f, 1.0f);
	color.y = Math::Random(0.0f, 1.0f);
	color.z = Math::Random(0.0f, 1.0f);
	colorDesc.RandomColor.r = Math::Random(0.0f, 1.0f);
	colorDesc.RandomColor.g = Math::Random(0.0f, 1.0f);
	colorDesc.RandomColor.b = Math::Random(0.0f, 1.0f);
	colorDesc.RandomColor.a = 1.0f;

}

void MeshSphereInstance::Render()
{
	Super::Render();
		
	//randomBuffer->Apply();
	//sRandomBuffer->SetConstantBuffer(randomBuffer->Buffer());

	shader->AsSRV("Maps")->SetResource(textures->SRV());
	instanceBuffer->Render();
	shader->DrawIndexedInstanced(0, 0, indices.size() , drawCount);
}

uint MeshSphereInstance::Push()
{
	drawCount++;

	return drawCount - 1;
}

Transform * MeshSphereInstance::GetTransform(uint index)
{
	return transforms[index];
}

void MeshSphereInstance::SetColor(uint index, Color color)
{
	instdesc[index].color = color;
}

void MeshSphereInstance::CreateSphereVertex()
{
	float radius = 1.0f;
	UINT stackCount = 20; 
	UINT sliceCount = 20;
	v.push_back(MeshVertex(0, radius, 0, 0, 0, 0, 1, 0));

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
			v.push_back(MeshVertex(p.x, p.y, p.z, uv.x, uv.y, n.x, n.y, n.z));
		}
	}
	v.push_back(MeshVertex(0, -radius, 0, 0, 0, 0, -1, 0));

	

	// 최상위 스택에 대한 인덱스 계산 상단 스택이 정점 버퍼에 먼저 기록됨
   // 상단 폴을 첫 번째 링에 연결
	
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

	
	
}
