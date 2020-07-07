#include "Framework.h"
#include "MeshCubeInstance.h"

MeshCubeInstance::MeshCubeInstance()
	:Renderer(L"024_Instance1.fx")
	, drawCount(0)

{
	for (uint i = 0; i < MAX_INSTANCE; i++)
	{
		//transforms[i] = new Transform(shader);
		transforms[i] = new Transform();
		D3DXMatrixIdentity(&worlds[i]);

	}

	CreateCubeVertex();

	vertexBuffer = new VertexBuffer(v.data(), v.size(), sizeof(MeshVertex));
	

	uint indices[36] = 
	{  0, 1, 2, 0, 2, 3,
	   4, 5, 6, 4, 6, 7,
	   8, 9, 10, 8, 10, 11,
	   12, 13, 14, 12, 14, 15,
	   16, 17, 18, 16, 18, 19,
	   20, 21, 22, 20, 22, 23
	};
	indexBuffer = new IndexBuffer(indices, 36);
	instanceBuffer = new VertexBuffer(worlds, MAX_INSTANCE, sizeof(Matrix), 1, true);



	//worlds를 기준으로 인스턴스 한다
}

MeshCubeInstance::~MeshCubeInstance()
{
}

void MeshCubeInstance::Update()
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

void MeshCubeInstance::Render()
{
	Super::Render();

	   	  
	instanceBuffer->Render();
	shader->DrawIndexedInstanced(0, 0, 36, drawCount);





}

uint MeshCubeInstance::Push()
{
	drawCount++;

	return drawCount - 1;
}

Transform * MeshCubeInstance::GetTransform(uint index)
{
	return transforms[index];
}

void MeshCubeInstance::CreateCubeVertex()
{
	float w = 0.5f;
	float h = 0.5f;
	float d = 0.5f;

	//Front
	v.push_back(MeshVertex(-w, -h, -d, 0, 1, 0, 0, -1));
	v.push_back(MeshVertex(-w, +h, -d, 0, 0, 0, 0, -1));
	v.push_back(MeshVertex(+w, +h, -d, 1, 0, 0, 0, -1));
	v.push_back(MeshVertex(+w, -h, -d, 1, 1, 0, 0, -1));

	//Back
	v.push_back(MeshVertex(-w, -h, +d, 1, 1, 0, 0, 1));
	v.push_back(MeshVertex(+w, -h, +d, 0, 1, 0, 0, 1));
	v.push_back(MeshVertex(+w, +h, +d, 0, 0, 0, 0, 1));
	v.push_back(MeshVertex(-w, +h, +d, 1, 0, 0, 0, 1));

	//Top
	v.push_back(MeshVertex(-w, +h, -d, 0, 1, 0, 1, 0));
	v.push_back(MeshVertex(-w, +h, +d, 0, 0, 0, 1, 0));
	v.push_back(MeshVertex(+w, +h, +d, 1, 0, 0, 1, 0));
	v.push_back(MeshVertex(+w, +h, -d, 1, 1, 0, 1, 0));

	//Bottom
	v.push_back(MeshVertex(-w, -h, -d, 1, 1, 0, -1, 0));
	v.push_back(MeshVertex(+w, -h, -d, 0, 1, 0, -1, 0));
	v.push_back(MeshVertex(+w, -h, +d, 0, 0, 0, -1, 0));
	v.push_back(MeshVertex(-w, -h, +d, 1, 0, 0, -1, 0));

	//Left
	v.push_back(MeshVertex(-w, -h, +d, 0, 1, -1, 0, 0));
	v.push_back(MeshVertex(-w, +h, +d, 0, 0, -1, 0, 0));
	v.push_back(MeshVertex(-w, +h, -d, 1, 0, -1, 0, 0));
	v.push_back(MeshVertex(-w, -h, -d, 1, 1, -1, 0, 0));

	//Right
	v.push_back(MeshVertex(+w, -h, -d, 0, 1, 1, 0, 0));
	v.push_back(MeshVertex(+w, +h, -d, 0, 0, 1, 0, 0));
	v.push_back(MeshVertex(+w, +h, +d, 1, 0, 1, 0, 0));
	v.push_back(MeshVertex(+w, -h, +d, 1, 1, 1, 0, 0));


	
	
	
}
