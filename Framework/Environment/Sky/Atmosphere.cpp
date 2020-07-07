#include "Framework.h"
#include "Atmosphere.h"
#include "Scattering.h"

#include "Moon.h"
Atmosphere::Atmosphere(Shader * shader)
	: Renderer(shader)
	, realTime(false), timeFactor(0.1f)
	, theta(1.676f), prevTheta(1)//2.015f
{


	moon = new Moon(shader);

	scatterDesc.InvWaveLength.x = 1.0f / powf(scatterDesc.WaveLength.x, 4.0f);
	scatterDesc.InvWaveLength.y = 1.0f / powf(scatterDesc.WaveLength.y, 4.0f);
	scatterDesc.InvWaveLength.z = 1.0f / powf(scatterDesc.WaveLength.z, 4.0f);

	scatterDesc.WaveLengthMie.x = powf(scatterDesc.WaveLength.x, -0.84f);
	scatterDesc.WaveLengthMie.y = powf(scatterDesc.WaveLength.y, -0.84f);
	scatterDesc.WaveLengthMie.z = powf(scatterDesc.WaveLength.z, -0.84f);

	scattering = new Scattering(shader);
	scatterBuffer = new ConstantBuffer(&scatterDesc, sizeof(ScatterDesc));
	sScatterBuffer = shader->AsConstantBuffer("CB_Scatter");
	

}

Atmosphere::~Atmosphere()
{
	SafeDelete(scattering);
	SafeDelete(scatterBuffer);
	SafeDelete(moon);
	
	SafeDelete(scattering);
	SafeDelete(scatterBuffer);
}

void Atmosphere::Update()
{
	Context::Get()->GetCamera()->Position(&position);
	position.y += 50.1f;

	GetTransform()->Position(position);
	GetTransform()->Scale(scale, scale, scale);
	GetTransform()->Rotation(0, 0, 0, 0);
	Super::Update();
	//Manual
	{
		
		//ImGui::SliderFloat("Theta", &theta, -2.175f, 2.175f);

		float x = sinf(theta);
		float y = cosf(theta);

		//Debug::DebugVector(Vector3(x, y, 0));
		Context::Get()->LightDirection() = Vector3(x, y, 1.0f);
		Vector3 dir = Context::Get()->LightDirection();
		//dir.x *= 1.3f;
		 SunPos = -1*10000 * dir;
		SunPos.y +=5500.f;
		
		 Context::Get()->LightPosition() = SunPos;
		//Debug::DebugVector(SunPos);
	   //	Color color= Color(0.74f, 0.59f,0.247f, 1);
		//color /=  2;
		//Context::Get()->LightSpecular() = color;
	}


}

void Atmosphere::ImGui()
{

	ImGui::Begin("Sky", 0, ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize);
	ImGui::SliderFloat("Sun Theta", &theta, 1.0f, Math::PI);

	
//	ImGui::InputFloat("dome scale", (float*)&scale, 0, 1000);
}
void Atmosphere::Render()
{
	
	
	//position = Vector3(0, 10, 0);
	//Scattering
	{
		
		scatterDesc.StarIntensity = Context::Get()->LightDirection().y;
		scatterDesc.Time = Time::Get()->Running();
		

		scatterBuffer->Apply();
		sScatterBuffer->SetConstantBuffer(scatterBuffer->Buffer());

	/*	ID3D11RenderTargetView* skyrt[1] = { skyTarget->RTV() };
		D3D::GetDC()->OMSetRenderTargets(1, skyrt, nullptr);
		D3D::Get()->Clear(Color(1, 1, 1, 1), skyrt[0], nullptr);*/
		Super::Render();
		scattering->Render();
	
	}
	

	////Moon
	//{
	//	scatterDesc.MoonAlpha = moon->MoonAlpha(theta);
	//	scatterBuffer->Apply();
	//	sScatterBuffer->SetConstantBuffer(scatterBuffer->Buffer());

	//	GetTransform()->World(moon->GetTransform(theta));

	//	Super::Render();
	//	moon->Render();
	//}

	////MoonGlow
	//{
	//	GetTransform()->World(moon->GetGlowTransform(theta));
	//	
	//	Super::Render();
	//	moon->Render(true);
	//}
	
}

void Atmosphere::Pass(const uint & index)
{
	scattering->Pass(index);

}



