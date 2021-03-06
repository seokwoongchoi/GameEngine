#include "Framework.h"
#include "Shadow.h"

Shadow::Shadow(Shader * shader, Vector3  position, float radius, UINT width, UINT height)
	: shader(shader), position(position), radius(radius), width(width), height(height)
{
	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	sBuffer = shader->AsConstantBuffer("CB_Shadow");

	renderTarget = new RenderTarget(width, height);
	depthStencil = new DepthStencil(width, height, true);
	viewport = new Viewport((float)width, (float)height);
	desc.MapSize = Vector2((float)width, (float)height);

	shader->AsSRV("ShadowMap")->SetResource(depthStencil->SRV());


	//Create SamplerState
	{
		D3D11_SAMPLER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SAMPLER_DESC));
		desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		desc.MaxAnisotropy = 1;
		desc.MinLOD = 0;
		desc.MaxLOD = FLT_MAX;

		Check(D3D::GetDevice()->CreateSamplerState(&desc, &samplerState));
	}
	shader->AsSampler("ShadowSampler")->SetSampler(0, samplerState);
}

Shadow::~Shadow()
{
	SafeDelete(buffer);

	SafeDelete(renderTarget);
	SafeDelete(depthStencil);
	SafeDelete(viewport);
}

void Shadow::Set()
{
	ImGui::InputInt("Index", (int *)&desc.Quality);
	desc.Quality %= 3;

	ImGui::SliderFloat3("Direction", (float *)&Context::Get()->LightDirection(), -1, 1);
	ImGui::SliderFloat3("Position", (float *)&position, -20, 20);

	UpdateVolume();


	renderTarget->Set(depthStencil->DSV());
	viewport->RSSetViewport();

	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());
}

void Shadow::UpdateVolume()
{
	Vector3 up = Vector3(0, 1, 0);
	Vector3 direction = Context::Get()->LightDirection();
	Vector3 position = direction * radius * -2.0f;

	D3DXMatrixLookAtLH(&desc.View, &position, &this->position, &up);


	Vector3 cube;
	D3DXVec3TransformCoord(&cube, &this->position, &desc.View);

	float l = cube.x - radius;
	float b = cube.y - radius;
	float n = cube.z - radius;

	float r = cube.x + radius;
	float t = cube.y + radius;
	float f = cube.z + radius;

	D3DXMatrixOrthoLH(&desc.Projection, r - l, t - b, n, f);
}
