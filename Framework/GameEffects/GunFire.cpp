#include "Framework.h"
#include "GunFire.h"

GunFire::GunFire()
	:Effect()
{
	shader = new Shader(L"Deferred/EffectRender.fx");
	buffer = new ConstantBuffer(&bufferDesc, sizeof(BufferDesc));
	sBuffer = shader->AsConstantBuffer("CB_Effect");
	
	effectTexture = new Texture(L"Environment/gunFire3.jpg");
	textureEffect = this->shader->AsSRV("effectTexture");
	vertex.position = Vector3(0,0,0);

	for (uint i = 0; i < 2; i++)
	{
		vertices.emplace_back(vertex);
		//vertices.emplace_back(vertex);
	
	}
	D3DXMatrixIdentity(&world);
	
	vertexBuffer = new VertexBuffer(&vertices[0], 2, sizeof(EffectPosition));
	preframe = new PerFrame(shader);
}

GunFire::~GunFire()
{
}

void GunFire::Update()
{
	
}

void GunFire::Render()
{
	//for (uint i = 0; i < drawCount; i++)
	{
		
		vertexBuffer->Render();
		Matrix T;
		
		Vector3 pos,scale;
		Quaternion q;
		D3DXMatrixDecompose(&scale, &q, &pos, transforms);
		//transforms[i]->Position(&pos);
		D3DXMatrixTranslation(&T, pos.x, pos.y, pos.z);

		bufferDesc.World = world*T;
		
		buffer->Apply();
		sBuffer->SetConstantBuffer(buffer->Buffer());
		preframe->Update();
		preframe->Render();
		textureEffect->SetResource(effectTexture->SRV());
		D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		QueryPerformanceCounter((LARGE_INTEGER *)&currentTime);
		if (chrono::_Is_even<uint>(currentTime))
			shader->Draw(0, 1, 2);
	}
	
}

void GunFire::PreviewRender()
{
	vertexBuffer->Render();
	
	bufferDesc.World = world;
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	textureEffect->SetResource(effectTexture->SRV());
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	QueryPerformanceCounter((LARGE_INTEGER *)&currentTime);
	if (chrono::_Is_even<uint>(currentTime))
		shader->Draw(0, 0, 2);
}
