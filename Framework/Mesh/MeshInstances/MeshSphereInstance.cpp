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

	//worlds�� �������� �ν��Ͻ� �Ѵ�
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

	//���̸� ����ī��Ʈ�� ������ ���� ���ñ���
	float phiStep = Math::PI / stackCount;
	//�߸����� �׸������� �Ѵܰ� ����
	float thetaStep = 2.0f * Math::PI / sliceCount;
	// �� ���� ���� ���� ������ ���
	for (UINT i = 1; i <= stackCount - 1; i++)
	{
		float phi = i * phiStep;
		//���� ������
		for (UINT j = 0; j <= sliceCount; j++)
		{
			//��Ÿ
			float theta = j * thetaStep;
			//������ ��ġ
			D3DXVECTOR3 p = D3DXVECTOR3 //������ ������ǥ��
			(
				(radius * sinf(phi) * cosf(theta)),
				(radius * cosf(phi)),
				(radius * sinf(phi) * sinf(theta))
			);

			//�ý��� �����Ҷ� ���°�?
			D3DXVECTOR3 t = D3DXVECTOR3
			(
				-radius * sinf(phi) * sinf(theta),
				0,
				radius * sinf(phi) * cosf(theta)
			);
			//�븻������
			D3DXVec3Normalize(&t, &t);
			//�����ǿ��� ������ ������ ����
			D3DXVECTOR3 n;
			D3DXVec3Normalize(&n, &p);
			D3DXVECTOR2 uv = D3DXVECTOR2(theta / (Math::PI * 2), phi / Math::PI);
			v.push_back(MeshVertex(p.x, p.y, p.z, uv.x, uv.y, n.x, n.y, n.z));
		}
	}
	v.push_back(MeshVertex(0, -radius, 0, 0, 0, 0, -1, 0));

	

	// �ֻ��� ���ÿ� ���� �ε��� ��� ��� ������ ���� ���ۿ� ���� ��ϵ�
   // ��� ���� ù ��° ���� ����
	
	for (UINT i = 1; i <= sliceCount; i++)
	{
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i);
	}

	// ���� ���ÿ� ���� �ε��� ���(�ؿ� ������� ����)
	// ù ��° ���� ù ��° ���� �ε����� ���� �ε��� ���ݶ���
	// ����� ��� ������ �ǳʶ�
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

	// �ϴ� ���ÿ� ���� �ε��� ��� �ϴ� ������ ���� ���ۿ� ���������� ��ϵ�
	// �ϴ� ���� �ϴ� ���� ����
	// ���� ������ ���������� �߰�
	UINT southPoleIndex = v.size() - 1;
	// ������ ���� ù ��° ���� �ε����� ���� �ε��� ���ݶ���
	baseIndex = southPoleIndex - ringVertexCount;

	//�� �Ʒ� ������ �� ���� ���� ����
	for (UINT i = 0; i < sliceCount; i++)
	{
		indices.push_back(southPoleIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}

	
	
}
