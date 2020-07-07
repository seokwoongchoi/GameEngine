#include "Framework.h"
#include "MeshCylinderInstance.h"

MeshCylinderInstance::MeshCylinderInstance(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount )
	:Renderer(L"024_Instance2.fx")
	, drawCount(0)
	, topRadius(topRadius), bottomRadius(bottomRadius), height(height), sliceCount(sliceCount), stackCount(stackCount)
	
{
	for (uint i = 0; i < MAX_INSTANCE; i++)
	{
		//transforms[i] = new Transform(shader);
		transforms[i] = new Transform();
		D3DXMatrixIdentity(&worlds[i]);
		
	}

	CreateSphereVertex();

	vertexBuffer = new VertexBuffer(v.data(), v.size(), sizeof(MeshVertex));
    indexBuffer = new IndexBuffer(indices.data(), indices.size());
	
	instanceBuffer = new VertexBuffer(worlds, MAX_INSTANCE, sizeof(Matrix), 1, true);



	//worlds�� �������� �ν��Ͻ� �Ѵ�
}

MeshCylinderInstance::~MeshCylinderInstance()
{
}

void MeshCylinderInstance::Update()
{
	Super::Update();


	for (uint i = 0; i < drawCount; i++)
	{
		memcpy(&worlds[i], &transforms[i]->World(), sizeof(Matrix));

	}
	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(instanceBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, worlds, sizeof(Matrix)*MAX_INSTANCE);
	}
	D3D::GetDC()->Unmap(instanceBuffer->Buffer(), 0);





}

void MeshCylinderInstance::Render()
{
	Super::Render();

	

	
	
		
		instanceBuffer->Render();
		shader->DrawIndexedInstanced(0, 0, indices.size() , drawCount);





}

uint MeshCylinderInstance::Push()
{
	drawCount++;

	return drawCount - 1;
}

Transform * MeshCylinderInstance::GetTransform(uint index)
{
	return transforms[index];
}

void MeshCylinderInstance::CreateSphereVertex()
{
	
	
	
	// ���� �ױ�. ��ü ���̸� ���ø�ŭ ������ ���� ������ ���� ���Ѵ�
	float stackHeight = height / (float)stackCount;

	//�� ���� ������ �Ʒ����� ���� �ø� �� �ݰ��� ������ų ��.
	float radiusStep = (topRadius - bottomRadius) / (float)stackCount;

	//
	UINT ringCount = stackCount + 1;
	//�ϴܿ��� �����Ͽ� ���� �̵��ϴ� �� ���� ���� ���� ������ ���
	for (UINT i = 0; i < ringCount; i++)
	{
		//�� ���� ������ ���� 
		float y = -0.5f * height + i * stackHeight;
		//�� ���� ������ ������
		float r = bottomRadius + i * radiusStep;
		//�� ���� ������ ����
		float theta = 2.0f * Math::PI / (float)sliceCount;

		for (UINT k = 0; k <= sliceCount; k++)
		{
			//�� ���ط����� ���� ������ ������ ���
			float c = cosf(k * theta);
			float s = sinf(k * theta);

			//�Ǹ����� ������ ���� �Ű������� ������ �� ������, ���⼭ v�� �Ұ��Ѵ�.
			//	v tex - coord�� ������ �������� �̵��ϴ� �Ű� ����
			//	bit�� v tex - coord�� ���� �������� ������ �Ѵ�.
			//	r0�� �ϴ� �ݰ��� �ǵ��� �ϰ� r1�� ��� �ݰ��� �ǵ��� �Ѵ�.
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
			//���ؽ� �����ǿ� ����� ��ġ�� ����

			//���� ���� ������ǥ�� ������ ��������
			D3DXVECTOR3 tangent = D3DXVECTOR3(-s, 0.0f, c);

			float dr = bottomRadius - topRadius;
			D3DXVECTOR3 biTangent = D3DXVECTOR3(dr * c, -height, dr * s);

			D3DXVec3Cross(&vertex.Normal, &tangent, &biTangent);
			D3DXVec3Normalize(&vertex.Normal, &vertex.Normal);

			v.push_back(vertex);
		}
	}


	
	//���� ù ��° ������ ������ ������ �����ϱ� ������ ���� �߰�
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

	BuildTopCap(v, indices);
	BuildBottomCap(v, indices);

	
	
}

void MeshCylinderInstance::BuildTopCap(vector<MeshVertex>& vertices, vector<UINT>& indices)
{
	float y = 0.5f * height;
	float theta = 2.0f * Math::PI / (float)sliceCount;

	for (UINT i = 0; i <= sliceCount; i++)
	{
		float x = topRadius * cosf(i * theta);
		float z = topRadius * sinf(i * theta);

		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		vertices.push_back(MeshVertex(x, y, z, u, v, 0, 1, 0));
	}
	vertices.push_back(MeshVertex(0, y, 0, 0.5f, 0.5f, 0, 1, 0));


	UINT baseIndex = vertices.size() - sliceCount - 2;
	UINT centerIndex = vertices.size() - 1;

	for (UINT i = 0; i < sliceCount; i++)
	{
		indices.push_back(centerIndex);
		indices.push_back(baseIndex + i + 1);
		indices.push_back(baseIndex + i);
	}
}

void MeshCylinderInstance::BuildBottomCap(vector<MeshVertex>& vertices, vector<UINT>& indices)
{
	float y = -0.5f * height;
	float theta = 2.0f * Math::PI / (float)sliceCount;

	for (UINT i = 0; i <= sliceCount; i++)
	{
		float x = bottomRadius * cosf(i * theta);
		float z = bottomRadius * sinf(i * theta);

		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		vertices.push_back(MeshVertex(x, y, z, u, v, 0, -1, 0));
	}
	vertices.push_back(MeshVertex(0, y, 0, 0.5f, 0.5f, 0, -1, 0));


	UINT baseIndex = vertices.size() - sliceCount - 2;
	UINT centerIndex = vertices.size() - 1;

	for (UINT i = 0; i < sliceCount; i++)
	{
		indices.push_back(centerIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}
}
