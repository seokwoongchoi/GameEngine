#include "Framework.h"
#include "CascadedShadow.h"
//#include "CascadedMatrixSet.h"

CascadedShadow::CascadedShadow(Shader* Shadowshader,uint width, uint height)
	:shader(Shadowshader),width(width), height(height), bNeedUpdate(true)
{
	buffer = new ConstantBuffer(&cascaed, sizeof(cascaedDesc));
	sBuffer = Shadowshader->AsConstantBuffer("cbuffercbShadowMapCubeGS");
	//cascadedMatrixSet = new CascadedMatrixSet();
	//cascadedMatrixSet->Init(1280/720);
	camera = Context::Get()->GetCamera();
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
		desc.Format = DXGI_FORMAT_R32_TYPELESS;
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


		HRESULT hr = D3D::GetDevice()->CreateDepthStencilView(cascadedTexture, &desc, &cascadedDSV);
		Check(hr)
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
		HRESULT hr = D3D::GetDevice()->CreateShaderResourceView(cascadedTexture, &desc, &cascadedSRV);
		Check(hr)
	}
	
	
	////////////////////////////////////////////////////////////////////////

		//Create SamplerState
	{
		D3D11_SAMPLER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SAMPLER_DESC));
		desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;//D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;//D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;//D3D11_TEXTURE_ADDRESS_WRAP;
		desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		desc.MaxAnisotropy = 1;
		desc.MinLOD = 0;
		desc.MaxLOD = D3D11_FLOAT32_MAX;//FLT_MAX;

		Check(D3D::GetDevice()->CreateSamplerState(&desc, &samplerState));
	}
	
	
}

CascadedShadow::~CascadedShadow()
{
}

void CascadedShadow::Init(int shdowMapSize)
{
	vDirectionalDir = Context::Get()->LightDirection();
	this->shadowMapSize = shdowMapSize;
	this->cascadeTotalRange = 500.0f;



	arrCascadeRanges[0] = 0.1f;
	arrCascadeRanges[1] = 10.0f;
	arrCascadeRanges[2] = 300.0f;
	arrCascadeRanges[3] = cascadeTotalRange;

	for (int i = 0; i < 3; i++)
	{
		arrCascadeBoundCenter[i] = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		arrCascadeBoundRadius[i] = 0.0f;
	}
}

void CascadedShadow::Set()
{
	
	ID3D11RenderTargetView* nullRT = NULL;
	
	D3D::GetDC()->OMSetRenderTargets(1, &nullRT, cascadedDSV);
	D3D::Get()->Clear(Color(0, 0, 0, 1), nullRT, cascadedDSV);

	D3D11_VIEWPORT vp[3] = { { 0, 0, 1280, 720, 0.0f,1.0f }, { 0, 0, 1280, 720, 0.0f,1.0f }, { 0, 0, 1280, 720, 0.0f,1.0f } };
	D3D::GetDC()->RSSetViewports(3, vp);


	UpdateShadowMatrix();


	for (int i = 0; i < 3; i++)
	{
		cascaed.cascadeProj[i] = arrWorldToCascadeProj[i];
	}

	if (sBuffer)
	{
		buffer->Apply();
		sBuffer->SetConstantBuffer(buffer->Buffer());
	}
	
			
	
	
}

void CascadedShadow::UpdateShadowMatrix()
{
	//Super::Update();
	Vector3 camPos;
	Vector3 camRight;
	Vector3 camUp;
	Vector3 camForward;
	Context::Get()->GetCamera()->Position(&camPos);
	Context::Get()->GetCamera()->Right(&camRight);
	Context::Get()->GetCamera()->Up(&camUp);
	Context::Get()->GetCamera()->Forward(&camForward);
	auto view = Context::Get()->View();
	Matrix viewInv;
	D3DXMatrixInverse(&viewInv, nullptr, &view);
	//camera->Position(&camPos);
	//camPos = Vector3(viewInv._41, viewInv._42, viewInv._43);

	
	// Find the view matrix
	Vector3 worldCenter = Vector3(camPos.x,0, camPos.z) + camForward * cascadeTotalRange * 0.5f;
	Vector3 pos = worldCenter;
	Vector3 lookAt = worldCenter + vDirectionalDir * 1000.0f;
	Vector3 right = Vector3(1.0f, 0.0f, 0.0f);
	Vector3 up;
	D3DXVec3Cross(&up, &vDirectionalDir, &right);
	D3DXVec3Normalize(&up, &up);

	Matrix shadowView;
	D3DXMatrixLookAtLH(&shadowView, &pos, &lookAt, &up);
	// Get the bounds for the shadow space
	float fRadius;
	ExtractFrustumBoundSphere(arrCascadeRanges[0], arrCascadeRanges[3], shadowBoundCenter, fRadius,
		camPos, camRight, camUp, camForward);

	shadowBoundRadius = max(shadowBoundRadius, fRadius);
	// Find the projection matrix
	Matrix shadowProj;
	D3DXMatrixOrthoLH(&shadowProj, shadowBoundRadius, shadowBoundRadius, -shadowBoundRadius, shadowBoundRadius);
	// The combined transformation from world to shadow space
	worldToShadowSpace = shadowView * shadowProj;
	// For each cascade find the transformation from shadow to cascade space
	D3DXMATRIX shadowViewInv;
	D3DXMatrixTranspose(&shadowViewInv, &shadowView);
	//D3DXMatrixInverse(&shadowViewInv,nullptr ,&shadowView);

	for (uint i = 0; i < 3; i++)
	{
		Matrix cascadeTrans;
		Matrix S;
		// To avoid anti flickering we need to make the transformation invariant to camera rotation and translation
			// By encapsulating the cascade frustum with a sphere we achive the rotation invariance
		Vector3 newCenter;
		ExtractFrustumBoundSphere(arrCascadeRanges[i], arrCascadeRanges[i + 1], newCenter, fRadius,
			camPos, camRight, camUp, camForward);
		arrCascadeBoundRadius[i] = max(arrCascadeBoundRadius[i], fRadius);// Expend the radius to compensate for numerical errors
		// Only update the cascade bounds if it moved at least a full pixel unit
		// This makes the transformation invariant to translation
		Vector3 offset;
		if (CascadeNeedsUpdate(shadowView, i, newCenter, offset))
		{
			// To avoid flickering we need to move the bound center in full units
			Vector3 offsetOut;
			D3DXVec3TransformNormal(&offsetOut, &offset, &shadowViewInv);
			arrCascadeBoundCenter[i] += offsetOut;
		}
		// Get the cascade center in shadow space
		Vector3 cascadeCenterShadowSpace;
		D3DXVec3TransformCoord(&cascadeCenterShadowSpace, &arrCascadeBoundCenter[i], &worldToShadowSpace);
		// Update the translation from shadow to cascade space
		cascadeOffsetX[i] = -cascadeCenterShadowSpace.x;
		cascadeOffsetY[i] = -cascadeCenterShadowSpace.y;
		D3DXMatrixTranslation(&cascadeTrans, cascadeOffsetX[i], cascadeOffsetY[i], 0.0f);

		// Update the scale from shadow to cascade space
		cascadeScale[i] = shadowBoundRadius / arrCascadeBoundRadius[i];
		D3DXMatrixScaling(&S, cascadeScale[i], cascadeScale[i], 1.0f);

		// Combine the matrices to get the transformation from world to cascade space
		arrWorldToCascadeProj[i] = worldToShadowSpace * cascadeTrans * S;
	}
	// Set the values for the unused slots to someplace outside the shadow space
	for (int i = 3; i < 4; i++)
	{
		cascadeOffsetX[i] = 250.0f;
		cascadeOffsetY[i] = 250.0f;
		cascadeScale[i] = 0.1f;
	}

}



void CascadedShadow::GetWorldToShadowSpace(Matrix * matrix)
{
	memcpy(matrix, &worldToShadowSpace, sizeof(Matrix));
}

void CascadedShadow::GetToCascadeOffsetX(Vector4 * vec)
{
	*vec = cascadeOffsetX;
}

void CascadedShadow::GetToCascadeOffsetY(Vector4 * vec)
{
	*vec = cascadeOffsetY;
	
}

void CascadedShadow::GetToCascadeScale(Vector4 * vec)
{
	*vec = cascadeScale;
}



void CascadedShadow::ExtractFrustumPoints(float fNear, float fFar, D3DXVECTOR3 * arrFrustumCorners)
{
	Vector3 camPos;
	Vector3 camRight;
	Vector3 camUp;
	Vector3 camForward;
	/*camera->Position(&camPos);
	camera->Right(&camRight);
	camera->Up(&camUp);
	camera->Forward(&camForward);*/

	float aspectRatio = 1;
	float fTanFOVX = tanf(aspectRatio*D3DX_PI * 0.25f);
	float fTanFOVY = tanf(aspectRatio);

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

void CascadedShadow::ExtractFrustumBoundSphere(float fNear, float fFar, D3DXVECTOR3 & vBoundCenter, float & fBoundRadius, Vector3 camPos, Vector3 camRight, Vector3 camUp, Vector3 camForward)
{
	/*
	Vector3 camPos;
	camera->Position(&camPos);
	Vector3 camRight = camera->Right();
	Vector3 camUp = camera->Up();
	Vector3 camForward = camera->Forward();*/

	float aspectRatio = 1280.0f/720.0f;
	const float fTanFOVX = tanf(aspectRatio*D3DX_PI * 0.25f);
	const float fTanFOVY = tanf(aspectRatio);

	// The center of the sphere is in the center of the frustum
	vBoundCenter = camPos + camForward * (fNear + 0.5f * (fNear + fFar));

	// Radius is the distance to one of the frustum far corners
	const D3DXVECTOR3 vBoundSpan = camPos + (-camRight * fTanFOVX + camUp * fTanFOVY + camForward) * fFar - vBoundCenter;
	fBoundRadius = D3DXVec3Length(&vBoundSpan);
}

bool CascadedShadow::CascadeNeedsUpdate(const D3DXMATRIX & mShadowView, int iCascadeIdx, const D3DXVECTOR3 & newCenter, D3DXVECTOR3 & vOffset)
{
	// Find the offset between the new and old bound ceter
	Vector3 oldCenterInCascade;
	D3DXVec3TransformCoord(&oldCenterInCascade, &arrCascadeBoundCenter[iCascadeIdx], &mShadowView);

	Vector3 newCenterInCascade;
	D3DXVec3TransformCoord(&newCenterInCascade, &newCenter, &mShadowView);
	Vector3 centerDiff = newCenterInCascade - oldCenterInCascade;
	// Find the pixel size based on the diameters and map pixel size
	float fPixelSize = (float)shadowMapSize / (2.0f * arrCascadeBoundRadius[iCascadeIdx]);

	float fPixelOffX = centerDiff.x * fPixelSize;
	float fPixelOffY = centerDiff.y * fPixelSize;

	// Check if the center moved at least half a pixel unit
    this->bNeedUpdate = abs(fPixelOffX) > 0.5f || abs(fPixelOffY) > 0.5f;
	if (bNeedUpdate)
	{
		// Round to the 
		vOffset.x =0;//floorf(0.5f + fPixelOffX) / fPixelSize;
		vOffset.y =0;//floorf(0.5f + fPixelOffY) / fPixelSize;
		vOffset.z = centerDiff.z;
	}
	//cout << bNeedUpdate << endl;
	return bNeedUpdate;
}
