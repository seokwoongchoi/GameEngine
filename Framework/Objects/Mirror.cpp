#include "Framework.h"
#include "Mirror.h"
#include "Viewer/Fixity.h"

Mirror::Mirror(Shader * shader, float height, float radius)
	:Renderer(shader),height(height),radius(radius), mapSize(1024, 1024)
{
	vertexCount = 4;
	indexCount = 6;

	VertexTexture vertices[4];
	vertices[0].Position = Vector3(-radius, -radius,0.0f);
	vertices[1].Position = Vector3(-radius, +radius,0.0f);
	vertices[2].Position = Vector3(+radius, -radius,0.0f);
	vertices[3].Position = Vector3(+radius, +radius,0.0f);

	vertices[0].Uv = Vector2(0, 1);
	vertices[1].Uv = Vector2(0, 0);
	vertices[2].Uv = Vector2(1, 1);
	vertices[3].Uv = Vector2(1, 0);

	UINT indices[6] = { 0,1,2,2,1,3 };

	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(VertexTexture));
	indexBuffer = new IndexBuffer(indices, indexCount);

	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	shader->AsConstantBuffer("CB_MirrorBuffer")->SetConstantBuffer(buffer->Buffer());
	fixity = new Fixity();

	rendertarget = new RenderTarget(static_cast<uint>(mapSize.x), static_cast<uint>(mapSize.y));
	
	depthStencil = new DepthStencil(static_cast<uint>(mapSize.x), static_cast<uint>(mapSize.y));
	viewport = new Viewport(static_cast<uint>(mapSize.x), static_cast<uint>(mapSize.y));

	shader->AsSRV("ReflectionMap")->SetResource(rendertarget->SRV());
	
}

Mirror::~Mirror()
{
}

void Mirror::Update()
{
	Super::Update();

	buffer->Apply();

	GetTransform()->Position(15, 10,10);

	static Vector3 position = Vector3(15,8, 20);
	static Vector3 rotation = Vector3(0, 3.15f, 0);
	
	
	auto forward=GetTransform()->Direction();
	
	fixity->Rotation(rotation);
	

	fixity->Position(position);

	fixity->GetMatrix(&desc.ReflectionMatrix);

}

void Mirror::Render()
{
	Super::Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	shader->DrawIndexed(0, Pass(), indexCount);
}

void Mirror::SetReflection()
{
	//Plane clipPlane = Plane(0, 1, 0, -height);
	//sClipPlane->SetFloatVector(clipPlane);


	Context::Get()->SetSubCamera(fixity);

	rendertarget->Set(depthStencil->DSV());
	viewport->RSSetViewport();
}

void Mirror::SetRefraction()
{
}
