#include "Framework.h"
#include "Fog.h"

Fog::Fog(Shader * shader)
	:shader(shader)
{
	fogBuffer = new ConstantBuffer(&fogCB, sizeof(CB_FOG_PS));
	sFogBuffer = shader->AsConstantBuffer("cbFog");

	D3DXVECTOR3 g_vFogColor = D3DXVECTOR3(0.5f, 0.5f, 0.5f);
	//D3DXVECTOR3 g_vFogHighlightColor = D3DXVECTOR3(0.8f, 0.7f, 0.4f);
	D3DXVECTOR3 g_vFogHighlightColor = D3DXVECTOR3(0.5f, 0.5f, 0.5f);
	float g_fFogStartDepth = 37.0f;
	
	float g_fFogGlobalDensity = 1.5f;
	float g_fFogHeightFalloff = 0.2f;

	fogCB.vColor = g_vFogColor;
	fogCB.vHighlightColor = g_vFogHighlightColor;
	fogCB.fStartDepth = g_fFogStartDepth;
	fogCB.fGlobalDensity = g_fFogGlobalDensity;
	
	fogCB.fHeightFalloff = g_fFogHeightFalloff;

}

Fog::~Fog()
{
}

void Fog::Update()
{
	ImGui::Begin("Fog", 0, ImGuiWindowFlags_NoMove);
	ImGui::SliderFloat("ClreaColor R", &fogCB.vColor.x, 0.0f, 1.0f);
	ImGui::SliderFloat("ClreaColor G", &fogCB.vColor.y, 0.0f, 1.0f);
	ImGui::SliderFloat("ClreaColor B", &fogCB.vColor.z, 0.0f, 1.0f);


	ImGui::SliderFloat("HighlightColor R", &fogCB.vHighlightColor.x, 0.0f, 1.0f);
	ImGui::SliderFloat("HighlightColor G", &fogCB.vHighlightColor.y, 0.0f, 1.0f);
	ImGui::SliderFloat("HighlightColor B", &fogCB.vHighlightColor.z, 0.0f, 1.0f);
		
	ImGui::SliderFloat("StartDepth", &fogCB.fStartDepth, 10.0f, 100.0f);
	ImGui::SliderFloat("Density", &fogCB.fGlobalDensity, 0.0f, 10.0f);
	ImGui::SliderFloat("HeightFalloff", &fogCB.fHeightFalloff, 0.0f, 10.0f);
	
	ImGui::End();
	
	
}

void Fog::Render()
{
	if (fogBuffer)
	{
		fogBuffer->Apply();
		sFogBuffer->SetConstantBuffer(fogBuffer->Buffer());
	}
}

void Fog::ImGui()
{
}
