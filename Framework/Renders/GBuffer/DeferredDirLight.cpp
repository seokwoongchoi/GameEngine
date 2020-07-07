#include "Framework.h"
#include "DeferredDirLight.h"


DeferredDirLight::DeferredDirLight(Shader* shader, uint width, uint height)
	:shader(shader),width(width),height(height), sCascadedProjView(nullptr), cascadedSRV(nullptr), offsetOutsave(0,0,0), bFirst(true)

	
{
	for (uint i = 0; i < 3; i++)
	{
		vp[i].Width = 1280.0f;
		vp[i].Height = 720.0f;
		vp[i].TopLeftX = 0;
		vp[i].TopLeftY = 0;
		vp[i].MinDepth = 0.0f;
		vp[i].MaxDepth = 1.0f;
	}
		
	sShadow = shader->AsSRV("CascadeShadowMapTexture");
	//sStaticShadow = shader->AsSRV("StaticCascadeShadowMapTexture");

	LightBuffer = new ConstantBuffer(&lightDesc, sizeof(LightDesc));
	sLightBuffer = shader->AsConstantBuffer("CB_Light");

	CascadedShadowBuffer = new ConstantBuffer(&cascadedShadowDesc, sizeof(CascadedShadowDesc));
	sCascadedShadowBuffer = shader->AsConstantBuffer("CB_CasecadedShadow");

	
	camera = Context::Get()->GetCamera();
	//pixelCS = new Shader(L"Deferred/PixelPickingCS.fx");
	////////////////////////////////////////////////////////////////////////
	//Create Texture DSV

	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = this->width;
		desc.Height = this->height;
		desc.MipLevels = 1;
		desc.ArraySize = 3;

		desc.MiscFlags = 0;
		desc.Format =  DXGI_FORMAT_R32_TYPELESS;
		desc.Usage = D3D11_USAGE_DEFAULT;
		
		desc.SampleDesc.Count = 1;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;

		HRESULT hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, &cascadedTexture);
		Check(hr)
	}


	//Create DSV
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		desc.Format = DXGI_FORMAT_D32_FLOAT;//;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;

		desc.Texture2DArray.FirstArraySlice = 0;
		desc.Texture2DArray.ArraySize = 3;


		Check(D3D::GetDevice()->CreateDepthStencilView(cascadedTexture, &desc, &cascadedDSV));
	//	Check(D3D::GetDevice()->CreateDepthStencilView(cascadedTexture, &desc, &staticCascadedDSV));

	}
	//Create SRV
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		desc.Format = DXGI_FORMAT_R32_FLOAT;
		desc.Texture2DArray.MipLevels = 1;

		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.FirstArraySlice = 0;
		desc.Texture2DArray.ArraySize = 3;
		Check(D3D::GetDevice()->CreateShaderResourceView(cascadedTexture, &desc, &cascadedSRV));
		//Check(D3D::GetDevice()->CreateShaderResourceView(cascadedTexture, &desc, &staticCascadedSRV));
		
	}




	////////////////////////////////////////////////////////////////////////

		//Create SamplerState
	{
		D3D11_SAMPLER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; //D3D11_TEXTURE_ADDRESS_CLAMP;//
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;	//D3D11_TEXTURE_ADDRESS_CLAMP;//
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;	//D3D11_TEXTURE_ADDRESS_CLAMP;//
		desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		desc.MaxAnisotropy = 1;
		desc.MinLOD = 0;
		desc.MaxLOD = D3D11_FLOAT32_MAX;//FLT_MAX;

		Check(D3D::GetDevice()->CreateSamplerState(&desc, &samplerState));
	}
	
	shader->AsSampler("PCFSampler")->SetSampler(0, samplerState);
	sCascadedProjView = shader->AsMatrix("CascadeViewProj");

	
	
}

DeferredDirLight::~DeferredDirLight()
{
	SafeDelete(LightBuffer);
}

void DeferredDirLight::InitShadowMap(int size)
{
	//vDirectionalDir = Context::Get()->LightDirection();
	//D3DXMatrixIdentity(&worldToShadowSpace);
	//cascadeOffsetX=Vector4(0,0,0,1);
	//cascadeOffsetY = Vector4(0, 0, 0, 1);
	//cascadeScale = Vector4(0, 0, 0, 1);
	
	this->cascadeTotalRange = 100;



	arrCascadeRanges[0] = 0.1f;
	arrCascadeRanges[1] = 25.0f;
	arrCascadeRanges[2] = 50.0f;
	arrCascadeRanges[3] = cascadeTotalRange;

	for (int i = 0; i < 3; i++)
	{
		arrCascadeBoundCenter[i] = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		arrCascadeBoundRadius[i] = 0.0f;
	}
}

void DeferredDirLight::Update()
{
	
	
	lightDesc.Ambient = Context::Get()->LightAmbient();
	lightDesc.Specular = Context::Get()->LightSpecular();
	lightDesc.Direction = Context::Get()->LightDirection();
	lightDesc.Position = Context::Get()->LightPosition();

	
	UpdateShadowMatrix();
	lightDesc.ToShadowSpace = worldToShadowSpace;
	lightDesc.ToCascadeSpace[0] = cascadeOffsetX;
	lightDesc.ToCascadeSpace[1] = cascadeOffsetY;
	lightDesc.ToCascadeSpace[2] = cascadeScale;
	
	

}

void DeferredDirLight::UpdateShadowMatrix()
{

	view = Context::Get()->View();
	Matrix viewInv;
	D3DXMatrixInverse(&viewInv, nullptr, &view);
	camPos     = Vector3(viewInv._41, viewInv._42, viewInv._43);
	camForward = Vector3(viewInv._31, viewInv._32, viewInv._33);
	camUp      = Vector3(viewInv._21, viewInv._22, viewInv._23);
	camRight    = Vector3(viewInv._11, viewInv._12, viewInv._13);

	
	// Find the view matrix
	worldCenter = camPos + camForward * cascadeTotalRange * 0.5f;
	pos         = worldCenter;
	lookAt      = worldCenter + Context::Get()->LightDirection() * 1000.0f;
	right       = Vector3(1.0f, 0.0f, 0.0f);
	
	D3DXVec3Cross(&up, &Context::Get()->LightDirection(), &right);
	D3DXVec3Normalize(&up, &up);

	
	D3DXMatrixLookAtLH(&shadowView, &pos, &lookAt, &up);
	// Get the bounds for the shadow space
	float fRadius;
	ExtractFrustumBoundSphere(arrCascadeRanges[0], arrCascadeRanges[3], shadowBoundCenter, fRadius);
		

	shadowBoundRadius = max(shadowBoundRadius, fRadius);
	// Find the projection matrix
	Matrix shadowProj;
	D3DXMatrixOrthoLH(&shadowProj, shadowBoundRadius, shadowBoundRadius, -shadowBoundRadius, shadowBoundRadius);
	// The combined transformation from world to shadow space
	worldToShadowSpace = shadowView * shadowProj;
	// For each cascade find the transformation from shadow to cascade space
	
	D3DXMatrixTranspose(&shadowViewInv, &shadowView);
	//D3DXMatrixInverse(&shadowViewInv,nullptr ,&shadowView);

	for (uint i = 0; i < 3; i++)
	{
		
		// To avoid anti flickering we need to make the transformation invariant to camera rotation and translation
			// By encapsulating the cascade frustum with a sphere we achive the rotation invariance
		
		ExtractFrustumBoundSphere(arrCascadeRanges[i], arrCascadeRanges[i + 1], newCenter, fRadius);
		arrCascadeBoundRadius[i] = max(arrCascadeBoundRadius[i], fRadius);// Expend the radius to compensate for numerical errors
		// Only update the cascade bounds if it moved at least a full pixel unit
		// This makes the transformation invariant to translation
		
		if (CascadeNeedsUpdate(shadowView, i, newCenter, offset))
		{
			// To avoid flickering we need to move the bound center in full units
			Vector3 offsetOut;
			
			D3DXVec3TransformNormal(&offsetOut, &offset, &shadowViewInv);
			
			
			arrCascadeBoundCenter[i] += offsetOut;
		}
		// Get the cascade center in shadow space
	
		D3DXVec3TransformCoord(&cascadeCenterShadowSpace, &arrCascadeBoundCenter[i], &worldToShadowSpace);
		// Update the translation from shadow to cascade space
		cascadeOffsetX[i] = -cascadeCenterShadowSpace.x;
		cascadeOffsetY[i] = -cascadeCenterShadowSpace.y;
		D3DXMatrixTranslation(&cascadeTrans, cascadeOffsetX[i], cascadeOffsetY[i], 0.0f);

		// Update the scale from shadow to cascade space
		cascadeScale[i] = shadowBoundRadius / arrCascadeBoundRadius[i];
		D3DXMatrixScaling(&S, cascadeScale[i], cascadeScale[i], 1.0f);

		// Combine the matrices to get the transformation from world to cascade space
		cascadedShadowDesc.arrWorldToCascadeProj[i] = worldToShadowSpace * cascadeTrans * S;
	}
	// Set the values for the unused slots to someplace outside the shadow space
	for (int i = 3; i < 4; i++)
	{
		cascadeOffsetX[i] = 400.0f;
		cascadeOffsetY[i] = 400.0f;
		cascadeScale[i] = 0.1f;
	}
	
}




void DeferredDirLight::Render()
{
	
	//if (sLightBuffer)
	{

		LightBuffer->Apply();
		sLightBuffer->SetConstantBuffer(LightBuffer->Buffer());
	}
	//if (bFirst)
	{
		sShadow->SetResource(cascadedSRV);
		//bFirst = false;
	}

	
	D3D::GetDC()->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	//D3D::GetDC()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//shader->DrawIndexed(0, pass, 6);
	D3D::GetDC()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	static bool SSAO = false;
	if (Keyboard::Get()->Down('O'))
	{
		SSAO = SSAO ? false : true;
	}
	uint pass = SSAO ? 26 : 9;
	shader->Draw(0, pass,4);
}

void DeferredDirLight::SetShadowMap()
{
	
	

	D3D::GetDC()->RSSetViewports(3, vp);
	ID3D11RenderTargetView* nullRT = NULL;

	D3D::GetDC()->OMSetRenderTargets(1, &nullRT, cascadedDSV);
	D3D::GetDC()->ClearRenderTargetView(nullRT,Color(1,1,1,1));
	D3D::GetDC()->ClearDepthStencilView(cascadedDSV, D3D11_CLEAR_DEPTH , 1, 0);
	
	CascadedShadowBuffer->Apply();
	sCascadedShadowBuffer->SetConstantBuffer(CascadedShadowBuffer->Buffer());
}



void DeferredDirLight::ExtractFrustumPoints(float fNear, float fFar, D3DXVECTOR3 * arrFrustumCorners)
{
	float aspectRatio = D3D::Width() / D3D::Height();

	
	const  float fTanFOVX= tanf(aspectRatio*Math::PI * 0.25f);
	const  float fTanFOVY= tanf(aspectRatio);

	// Calculate the points on the near plane
	arrFrustumCorners[0] = camPos + (-camRight * fTanFOVX + camUp * fTanFOVY + camForward) * fNear;
	arrFrustumCorners[1] = camPos + (camRight * fTanFOVX + camUp * fTanFOVY + camForward) * fNear;
	arrFrustumCorners[2] = camPos + (camRight * fTanFOVX - camUp * fTanFOVY + camForward) * fNear;
	arrFrustumCorners[3] = camPos + (-camRight * fTanFOVX - camUp * fTanFOVY + camForward) * fNear;

	// Calculate the points on the far plane
	arrFrustumCorners[4] = camPos + (-camRight * fTanFOVX + camUp * fTanFOVY + camForward) * fFar;
	arrFrustumCorners[5] = camPos + (camRight * fTanFOVX + camUp * fTanFOVY + camForward) * fFar;
	arrFrustumCorners[6] = camPos + (camRight * fTanFOVX - camUp * fTanFOVY + camForward) * fFar;
	arrFrustumCorners[7] = camPos + (-camRight * fTanFOVX - camUp * fTanFOVY + camForward) * fFar;
}

void DeferredDirLight::ExtractFrustumBoundSphere(float fNear, float fFar, D3DXVECTOR3 & vBoundCenter, float & fBoundRadius)
{
	
	float aspectRatio = D3D::Width() / D3D::Height();
	
	const  float fTanFOVX = tanf(aspectRatio*Math::PI * 0.25f);
	const  float fTanFOVY = tanf(aspectRatio);

	// The center of the sphere is in the center of the frustum
	vBoundCenter = camPos + camForward * (fNear + 0.5f * (fNear + fFar));

	// Radius is the distance to one of the frustum far corners
	vBoundSpan = camPos + (-camRight * fTanFOVX + camUp * fTanFOVY + camForward) * fFar - vBoundCenter;
	fBoundRadius = D3DXVec3Length(&vBoundSpan);
}

bool DeferredDirLight::CascadeNeedsUpdate(const D3DXMATRIX & mShadowView, int iCascadeIdx, const D3DXVECTOR3 & newCenter, D3DXVECTOR3 & vOffset)
{
	// Find the offset between the new and old bound ceter
	
	D3DXVec3TransformCoord(&oldCenterInCascade, &arrCascadeBoundCenter[iCascadeIdx], &mShadowView);

	
	D3DXVec3TransformCoord(&newCenterInCascade, &newCenter, &mShadowView);
	 centerDiff = newCenterInCascade - oldCenterInCascade;
	// Find the pixel size based on the diameters and map pixel size
	 Vector2 fPixelSize;
	 fPixelSize.x= (float)1280.0f / (2.0f* arrCascadeBoundRadius[iCascadeIdx]);
	 fPixelSize.y = (float)720.0f / (2.0f* arrCascadeBoundRadius[iCascadeIdx]);
	 float fPixelOffX = centerDiff.x * fPixelSize.x;
	 float fPixelOffY = centerDiff.y * fPixelSize.y;

	// Check if the center moved at least half a pixel unit
	 bool bNeedUpdate = abs(fPixelOffX)> 0.5f || abs(fPixelOffY) > 0.5f;
	if (bNeedUpdate)
	{
		// Round to the 
		vOffset.x =floorf(0.5f + fPixelOffX) / fPixelSize.x;
		vOffset.y =floorf(0.5f + fPixelOffY) / fPixelSize.y;
		vOffset.z = centerDiff.z;
		//D3DXVec3Normalize(&vOffset, &vOffset);
	}
	//cout << bNeedUpdate << endl;
	return bNeedUpdate;
}
