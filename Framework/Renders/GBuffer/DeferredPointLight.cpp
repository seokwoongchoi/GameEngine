#include "Framework.h"
#include "DeferredPointLight.h"


DeferredPointLight::DeferredPointLight(Shader * shader)
	:shader(shader)
{
	
	pointLightBuffer = new ConstantBuffer(&pointLightDesc, sizeof(PointLightDesc));
	sPointLightBuffer = shader->AsConstantBuffer("CB_PointLights");

}


DeferredPointLight::~DeferredPointLight()
{
}



void DeferredPointLight::Render()
{
	for (auto& light : pointLights)
	{
		//Super::Render();
		View = Context::Get()->View();
		Proj = Context::Get()->Projection();
		pointLightDesc.Light = light;
		D3DXMatrixScaling(&lightScaleMatrix, light.Range, light.Range, light.Range);
		D3DXMatrixTranslation(&lightTransMatrix, light.Position.x, light.Position.y, light.Position.z);
		D3DXMATRIX mWorldViewProjection = lightScaleMatrix * lightTransMatrix * View * Proj;
		pointLightDesc.WolrdViewProj = mWorldViewProjection;

		if (pointLightBuffer)
		{
			pointLightBuffer->Apply();
			sPointLightBuffer->SetConstantBuffer(pointLightBuffer->Buffer());
		}
		D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
		shader->Draw(0, 10, 2);
	}
	
		
}

PointLight & DeferredPointLight::GetPointLight(UINT index)
{
	return pointLights[index];
}



//uint DeferredPointLight::PointLights(OUT PointLight*  lights)
//{
//	memcpy(lights, pointLights.data(), sizeof(PointLight)*pointLights.size());
//
//	return pointLights.size();
//}
//
void DeferredPointLight::AddPointLight(PointLight & light)
{
	
	pointLights.emplace_back(light);
	

}




