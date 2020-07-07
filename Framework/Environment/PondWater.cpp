#include "Framework.h"
#include "PondWater.h"
#include "Viewer/Fixity.h"

PondWater::PondWater(Shader* shader, float height, float radius)
	:Renderer(shader),height(height),radius(radius), waveSpeed(0.003f), mapSize(1024,1024)
{
	vertexCount = 4;
	indexCount = 6;

	VertexTexture vertices[4];
	vertices[0].Position = Vector3(-radius, 0.0f, -radius);
	vertices[1].Position = Vector3(-radius, 0.0f, +radius);
	vertices[2].Position = Vector3(+radius, 0.0f, -radius);
	vertices[3].Position = Vector3(+radius, 0.0f, +radius);

	vertices[0].Uv = Vector2(0, 1);
	vertices[1].Uv = Vector2(0, 0);
	vertices[2].Uv = Vector2(1, 1);
	vertices[3].Uv = Vector2(1, 0);

	UINT indices[6] = { 0,1,2,2,1,3 };

	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(VertexTexture));
	indexBuffer = new IndexBuffer(indices, indexCount);

	waveMap = new Texture(L"Environment/Wave.dds");
	shader->AsSRV("WaveMap")->SetResource(waveMap->SRV());

	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	shader->AsConstantBuffer("CB_PondBuffer")->SetConstantBuffer(buffer->Buffer());

	sClipPlane = shader->AsVector("WaterClipPlane");

	fixity = new Fixity();

	rendertarget = new RenderTarget(static_cast<uint>(mapSize.x), static_cast<uint>(mapSize.y));
	refraction = new RenderTarget(static_cast<uint>(mapSize.x), static_cast<uint>(mapSize.y));
	
	depthStencil = new DepthStencil(static_cast<uint>(mapSize.x), static_cast<uint>(mapSize.y));
	viewport = new Viewport(static_cast<uint>(mapSize.x), static_cast<uint>(mapSize.y));

	shader->AsSRV("ReflectionMap")->SetResource(rendertarget->SRV());
	shader->AsSRV("RefractionMap")->SetResource(refraction->SRV());
}

PondWater::~PondWater()
{
}

void PondWater::Update()
{
	Super::Update();
	desc.WaveTranslation += waveSpeed;
	if (desc.WaveTranslation > 1.0f)desc.WaveTranslation -= 1.f;

	buffer->Apply();

	GetTransform()->Position(0, height, 0);

	Vector3 position, rotation;
	Context::Get()->GetCamera()->Position(&position);
	Context::Get()->GetCamera()->Rotation(&rotation);

	rotation.x *= -1.0;
	fixity->Rotation(rotation);

	position.y = -position.y + height * 2.0f;
	fixity->Position(position);

	fixity->GetMatrix(&desc.ReflectionMatrix);

}

void PondWater::Render()
{
	Super::Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	shader->DrawIndexed(0, Pass(), indexCount);
}

void PondWater::SetReflection()
{
	Plane clipPlane = Plane(0, 1, 0, -height);
	sClipPlane->SetFloatVector(clipPlane);

	
	Context::Get()->SetSubCamera(fixity);

	rendertarget->Set(depthStencil->DSV());
	viewport->RSSetViewport();
}

void PondWater::SetRefraction()
{
	Plane clipPlane = Plane(0, -1, 0, height+0.1f);
	sClipPlane->SetFloatVector(clipPlane);
	
	
	refraction->Set(depthStencil->DSV());
	viewport->RSSetViewport();
}
