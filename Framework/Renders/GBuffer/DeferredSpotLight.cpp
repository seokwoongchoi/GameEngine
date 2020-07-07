#include "Framework.h"
#include "DeferredSpotLight.h"

DeferredSpotLight::DeferredSpotLight(Shader * shader)
	:shader(shader)
{
	D3D11_DEPTH_STENCIL_DESC descDepth;
	descDepth.DepthEnable = TRUE;
	descDepth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	descDepth.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	descDepth.StencilEnable = TRUE;
	descDepth.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	descDepth.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	const D3D11_DEPTH_STENCILOP_DESC noSkyStencilOp = { D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_EQUAL };
	descDepth.FrontFace = noSkyStencilOp;
	descDepth.BackFace = noSkyStencilOp;
	Check(D3D::GetDevice()->CreateDepthStencilState(&descDepth, &m_pNoDepthWriteGreatherStencilMaskState));
	//DXUT_SetDebugName(m_pNoDepthWriteLessStencilMaskState, "Depth Test Less / No Write, Stencil Mask DS");
	
	
	 DomainBuffer = new ConstantBuffer(&domainDesc, sizeof(DomainDesc));
	 sDomainBuffer = shader->AsConstantBuffer("CB_SpotDomain");

	pixelBuffer = new ConstantBuffer(&pixelDesc, sizeof(PixelDesc));
	sPixelLightBuffer = shader->AsConstantBuffer("CB_SpotPixel");
	
	/*spotPixelBuffer = new ConstantBuffer(&spotLight, sizeof(SpotLight));
	sSpotPixelLightBuffer = shader->AsConstantBuffer("CB_SpotLightsPixel");*/
}

DeferredSpotLight::~DeferredSpotLight()
{
}


void DeferredSpotLight::Render()
{
	for (auto& light : spotLights)
	{
		View = Context::Get()->View();
		Proj = Context::Get()->Projection();
		vDir = light.Direction;
		float fCosInnerAngle = cosf(Math::ToRadian(light.InnerAngle));
		float fSinOuterAngle = sinf(Math::ToRadian(light.OuterAngle));
		float fCosOuterAngle = cosf(Math::ToRadian(light.OuterAngle));
		pixelDesc.Diffuse = light.Diffuse;
		pixelDesc.Specular = light.Specular;
		pixelDesc.Position = light.Position;
		pixelDesc.Range = 1 / light.Range;
		pixelDesc.vDirToLight = -vDir;
		pixelDesc.Intensity = light.Intensity;
		
		pixelDesc.SpotCosConeAttRange = fCosInnerAngle - fCosOuterAngle;
		pixelDesc.SpotCosOuterCone = fCosOuterAngle;
		
	
		D3DXMatrixScaling(&lightScaleMatrix, light.Range, light.Range, light.Range);
		
		vUp    = (vDir.y > 0.9 || vDir.y < -0.9) ? D3DXVECTOR3(0.0f, 0.0f, vDir.y) : D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		
		D3DXVec3Cross(&vRight, &vUp, &vDir);
		D3DXVec3Normalize(&vRight, &vRight);
		D3DXVec3Cross(&vUp, &vDir, &vRight);
		D3DXVec3Normalize(&vUp, &vUp);
		vAt = light.Position + vDir * light.Range;
		
		D3DXMatrixIdentity(&m_LightWorldTransRotate);
		for (int i = 0; i < 3; i++)
		{
			m_LightWorldTransRotate.m[0][i] = (&vRight.x)[i];
			m_LightWorldTransRotate.m[1][i] = (&vUp.x)[i];
			m_LightWorldTransRotate.m[2][i] = (&vDir.x)[i];
			m_LightWorldTransRotate.m[3][i] = (&light.Position.x)[i];
		}
		
	
		 mWorldViewProjection = lightScaleMatrix * m_LightWorldTransRotate * View * Proj;
		domainDesc.WolrdViewProj= mWorldViewProjection;
		domainDesc.fSinAngle = fSinOuterAngle;
		domainDesc.fCosAngle = fCosOuterAngle;
		
		
		
		/*light.fSinAngle = light.fSinOuterAngle;
		light.fCosAngle = light.fCosOuterAngle;*/
		
		
		if (DomainBuffer)
		{
			DomainBuffer->Apply();
			sDomainBuffer->SetConstantBuffer(DomainBuffer->Buffer());
		}
		if (pixelBuffer)
		{
			pixelBuffer->Apply();
			sPixelLightBuffer->SetConstantBuffer(pixelBuffer->Buffer());
		}
		
		//ID3D11DepthStencilState* temp=nullptr;
		//UINT nPrevStencil;
		//D3D::GetDC()->OMGetDepthStencilState(&temp, &nPrevStencil);
		//D3D::GetDC()->OMSetDepthStencilState(m_pNoDepthWriteGreatherStencilMaskState,1);
		D3D::GetDC()->IASetInputLayout(NULL);
		D3D::GetDC()->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
		D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
		shader->Draw(0, 11, 1);
		//D3D::GetDC()->OMSetDepthStencilState(temp, nPrevStencil);
	}
}

void DeferredSpotLight::AddSpotLight(SpotLight & light)
{
	spotLights.emplace_back(light);
}

SpotLight & DeferredSpotLight::GetSpotLight(UINT index)
{
	return spotLights[index];
}
