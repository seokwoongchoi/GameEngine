#include "Framework.h"
#include "SSLR.h"

SSLR::SSLR(uint width,uint height)
	: OcclusionTex(NULL), OcclusionUAV(NULL), OcclusionSRV(NULL),
	LightRaysTex(NULL), LightRaysRTV(NULL), LightRaysSRV(NULL),
	 OcclusionCB(NULL),  OcclusionCS(NULL),  RayTraceCB(NULL),  FullScreenVS(NULL),  RayTracePS(NULL),  CombinePS(NULL),
	 AdditiveBlendState(NULL)
{
	this->width = 1280 / 2;
	this->height = 720 /2 ;
	downScaleGroups = 256;
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocate the occlusion resources
	D3D11_TEXTURE2D_DESC t2dDesc = {
		this->width , //UINT Width;
		this->height , //UINT Height;
		1, //UINT MipLevels;
		1, //UINT ArraySize;
		DXGI_FORMAT_R8_TYPELESS, //DXGI_FORMAT Format;
		1, //DXGI_SAMPLE_DESC SampleDesc;
		0,
		D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
		0,//UINT CPUAccessFlags;
		0//UINT MiscFlags;    
	};
	Check(D3D::GetDevice()->CreateTexture2D(&t2dDesc, NULL, &OcclusionTex));
	

	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
	ZeroMemory(&UAVDesc, sizeof(UAVDesc));
	UAVDesc.Format = DXGI_FORMAT_R8_UNORM;
	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	Check(D3D::GetDevice()->CreateUnorderedAccessView(OcclusionTex, &UAVDesc, &OcclusionUAV));
	

	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd =
	{
		DXGI_FORMAT_R8_UNORM,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		0,
		0
	};
	dsrvd.Texture2D.MipLevels = 1;
	Check(D3D::GetDevice()->CreateShaderResourceView(OcclusionTex, &dsrvd, &OcclusionSRV));
	

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocate the light rays resources
	t2dDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	Check(D3D::GetDevice()->CreateTexture2D(&t2dDesc, NULL, &LightRaysTex));
	

	D3D11_RENDER_TARGET_VIEW_DESC rtsvd =
	{
		DXGI_FORMAT_R8_UNORM,
		D3D11_RTV_DIMENSION_TEXTURE2D
	};
	Check(D3D::GetDevice()->CreateRenderTargetView(LightRaysTex, &rtsvd, &LightRaysRTV));
	
	Check(D3D::GetDevice()->CreateShaderResourceView(LightRaysTex, &dsrvd, &LightRaysSRV));
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocate the occlussion constant buffer
	D3D11_BUFFER_DESC CBDesc;
	ZeroMemory(&CBDesc, sizeof(CBDesc));
	CBDesc.Usage = D3D11_USAGE_DYNAMIC;
	CBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	CBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	CBDesc.ByteWidth = sizeof(CB_OCCLUSSION);
	Check(D3D::GetDevice()->CreateBuffer(&CBDesc, NULL, &OcclusionCB));
	//DXUT_SetDebugName(m_pOcclusionCB, "SSLR - Occlussion CB");

	CBDesc.ByteWidth = sizeof(CB_LIGHT_RAYS);
	Check(D3D::GetDevice()->CreateBuffer(&CBDesc, NULL, &RayTraceCB));
	//DXUT_SetDebugName(m_pRayTraceCB, "SSLR - Ray Trace CB");
	

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;// | D3DCOMPILE_WARNINGS_ARE_ERRORS;


	
	ID3DBlob* pShaderBlob = NULL;
	ID3DBlob* error;
	
	Check(D3DCompileFromFile(L"../Shader/Deferred/SSLR.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "Occlussion", "cs_5_0", dwShaderFlags, NULL, &pShaderBlob, &error));

	Check(D3D::GetDevice()->CreateComputeShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), NULL, &OcclusionCS));
	//DXUT_SetDebugName( OcclusionCS, "SSLR - Occlussion CS");
	SafeRelease(pShaderBlob);

	Check(D3DCompileFromFile(L"../Shader/Deferred/SSLR.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "RayTraceVS", "vs_5_0", dwShaderFlags, NULL, &pShaderBlob, &error));
	//V_RETURN(CompileShader(str, NULL, "RayTraceVS", "vs_5_0", dwShaderFlags, &pShaderBlob));
	Check(D3D::GetDevice()->CreateVertexShader(pShaderBlob->GetBufferPointer(),
		pShaderBlob->GetBufferSize(), NULL, & FullScreenVS));
//	DXUT_SetDebugName( FullScreenVS, "SSLR - Full Screen VS");
	SafeRelease(pShaderBlob);
	Check(D3DCompileFromFile(L"../Shader/Deferred/SSLR.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "RayTracePS", "ps_5_0", dwShaderFlags, NULL, &pShaderBlob, &error));
	
	Check(D3D::GetDevice()->CreatePixelShader(pShaderBlob->GetBufferPointer(),
		pShaderBlob->GetBufferSize(), NULL, & RayTracePS));
	//DXUT_SetDebugName( RayTracePS, "SSLR - Ray Trace PS");
	SafeRelease(pShaderBlob);
	Check(D3DCompileFromFile(L"../Shader/Deferred/SSLR.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CombinePS", "ps_5_0", dwShaderFlags, NULL, &pShaderBlob, &error));

//	V_RETURN(CompileShader(str, NULL, "CombinePS", "ps_5_0", dwShaderFlags, &pShaderBlob));
	Check(D3D::GetDevice()->CreatePixelShader(pShaderBlob->GetBufferPointer(),
		pShaderBlob->GetBufferSize(), NULL, & CombinePS));
	//DXUT_SetDebugName( CombinePS, "SSLR - Combine PS");
	SafeRelease(pShaderBlob);

	// Create the additive blend state
	D3D11_BLEND_DESC descBlend;
	descBlend.AlphaToCoverageEnable = FALSE;
	descBlend.IndependentBlendEnable = FALSE;
	const D3D11_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
	{
		TRUE,
		D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD,
		D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD,
		D3D11_COLOR_WRITE_ENABLE_ALL,
	};
	for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		descBlend.RenderTarget[i] = defaultRenderTargetBlendDesc;
	Check(D3D::GetDevice()->CreateBlendState(&descBlend, &AdditiveBlendState));

	
}

SSLR::~SSLR()
{
	
	SafeRelease( OcclusionTex);
	SafeRelease( OcclusionUAV);
	SafeRelease( OcclusionSRV);
	SafeRelease( LightRaysTex);
	SafeRelease( LightRaysRTV);
	SafeRelease( LightRaysSRV);
	SafeRelease( OcclusionCB);
	SafeRelease( OcclusionCS);
	SafeRelease( RayTraceCB);
	SafeRelease( FullScreenVS);
	SafeRelease( RayTracePS);
	SafeRelease( CombinePS);
	SafeRelease( AdditiveBlendState);
		
}

void SSLR::Render(ID3D11ShaderResourceView * ssaoSRV, ID3D11RenderTargetView* pLightAccumRTV)
{
	Camera* camera = Context::Get()->GetCamera();
	
	dir = Context::Get()->LightDirection();
	const float dotCamSun = -D3DXVec3Dot(&camera->Forward(), &dir);
	if (dotCamSun <= 0.0f||dir.y > 0.1)
	{
		return;
	}
	if (dir.y > -0.56f)
	{
		dir.y = -0.56f;
	}
	camera->Position(&vEyePos);
	vSunPos = -1 * dist* dir;
	vSunPos.x += vEyePos.x;
	vSunPos.z += vEyePos.z;
	mView = Context::Get()->View();
	mProj = Context::Get()->Projection();
	
	mViewProjection = mView * mProj;
	D3DXVec3TransformCoord(&vSunPosSS, &vSunPos, &mViewProjection);
	float factor = Math::Clamp(dotCamSun, 0.1f, 2.0);
	factor *= 2.0f;
	if (abs(vSunPosSS.x) >= fMaxSunDist/factor || abs(vSunPosSS.y) >= fMaxSunDist / factor)
	{
		return;
	}
	vSunColorAtt = yellow;
	vSunColorAtt *= intensity * factor;
	float fMaxDist = max(abs(vSunPosSS.x), abs(vSunPosSS.y));
	vSunColorAtt *= (fMaxSunDist - fMaxDist);
	PrepareOcclusion(ssaoSRV);
	RayTrace(D3DXVECTOR2(vSunPosSS.x, vSunPosSS.y), vSunColorAtt);
	if (!ShowRayTraceRes)
	Combine(pLightAccumRTV);

}

void SSLR::PrepareOcclusion(ID3D11ShaderResourceView * ssaoSRV)
{
	auto pd3dImmediateContext = D3D::GetDC();
	// Constants
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pd3dImmediateContext->Map( OcclusionCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	CB_OCCLUSSION* pOcclussion = static_cast<CB_OCCLUSSION*>(MappedResource.pData);
	pOcclussion->nWidth = width;
	pOcclussion->nHeight = height;
	pd3dImmediateContext->Unmap( OcclusionCB, 0);
	ID3D11Buffer* arrConstBuffers[1] = {  OcclusionCB };
	pd3dImmediateContext->CSSetConstantBuffers(0, 1, arrConstBuffers);

	// Output
	ID3D11UnorderedAccessView* arrUAVs[1] = {  OcclusionUAV };
	pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, arrUAVs, NULL);

	// Input
	ID3D11ShaderResourceView* arrViews[1] = { ssaoSRV };
	pd3dImmediateContext->CSSetShaderResources(0, 1, arrViews);

	// Shader
	pd3dImmediateContext->CSSetShader( OcclusionCS, NULL, 0);

	// Execute the downscales first pass with enough groups to cover the entire full res HDR buffer
	pd3dImmediateContext->Dispatch(downScaleGroups, 1, 1);

	// Cleanup
	pd3dImmediateContext->CSSetShader(NULL, NULL, 0);
	ZeroMemory(arrViews, sizeof(arrViews));
	pd3dImmediateContext->CSSetShaderResources(0, 1, arrViews);
	ZeroMemory(arrUAVs, sizeof(arrUAVs));
	pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, arrUAVs, NULL);
	ZeroMemory(arrConstBuffers, sizeof(arrConstBuffers));
	pd3dImmediateContext->CSSetConstantBuffers(0, 1, arrConstBuffers);
}

void SSLR::RayTrace(const D3DXVECTOR2 & vSunPosSS, const D3DXVECTOR3 & vSunColor)
{
	

	auto pd3dImmediateContext = D3D::GetDC();
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pd3dImmediateContext->ClearRenderTargetView( LightRaysRTV, ClearColor);

	D3D11_VIEWPORT oldvp;
	UINT num = 1;
	pd3dImmediateContext->RSGetViewports(&num, &oldvp);
	if (!ShowRayTraceRes)
	{
		D3D11_VIEWPORT vp[1] = { { 0, 0, (float)width, (float)height, 0.0f, 1.0f } };
		pd3dImmediateContext->RSSetViewports(1, vp);

		pd3dImmediateContext->OMSetRenderTargets(1, & LightRaysRTV, NULL);
	}

	// Constants
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pd3dImmediateContext->Map( RayTraceCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	CB_LIGHT_RAYS* pRayTrace = static_cast<CB_LIGHT_RAYS*>(MappedResource.pData);
	pRayTrace->vSunPos = D3DXVECTOR2(0.5f * vSunPosSS.x + 0.5f, -0.5f * vSunPosSS.y + 0.5f);
	pRayTrace->fInitDecay = decay;
	pRayTrace->fDistDecay = ddecay;
	pRayTrace->vRayColor = vSunColor + sunColor;
	pRayTrace->fMaxDeltaLen = fMaxDeltaLen;
	pd3dImmediateContext->Unmap( RayTraceCB, 0);
	ID3D11Buffer* arrConstBuffers[1] = {  RayTraceCB };
	pd3dImmediateContext->PSSetConstantBuffers(0, 1, arrConstBuffers);

	// Input
	ID3D11ShaderResourceView* arrViews[1] = {  OcclusionSRV };
	pd3dImmediateContext->PSSetShaderResources(0, 1, arrViews);

	// Primitive settings
	pd3dImmediateContext->IASetInputLayout(NULL);
	pd3dImmediateContext->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	pd3dImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// Set the shaders
	pd3dImmediateContext->VSSetShader( FullScreenVS, NULL, 0);
	pd3dImmediateContext->GSSetShader(NULL, NULL, 0);
	pd3dImmediateContext->PSSetShader( RayTracePS, NULL, 0);

	pd3dImmediateContext->Draw(4, 0);

	// Cleanup
	arrViews[0] = NULL;
	pd3dImmediateContext->PSSetShaderResources(0, 1, arrViews);
	pd3dImmediateContext->VSSetShader(NULL, NULL, 0);
	pd3dImmediateContext->PSSetShader(NULL, NULL, 0);
	pd3dImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dImmediateContext->RSSetViewports(1, &oldvp);
	
}

void SSLR::Combine(ID3D11RenderTargetView* pLightAccumRTV)
{

	auto pd3dImmediateContext = D3D::GetDC();
	ID3D11BlendState* pPrevBlendState;
	FLOAT prevBlendFactor[4];
	UINT prevSampleMask;
	pd3dImmediateContext->OMGetBlendState(&pPrevBlendState, prevBlendFactor, &prevSampleMask);
	pd3dImmediateContext->OMSetBlendState( AdditiveBlendState, prevBlendFactor, prevSampleMask);

	// Restore the light accumulation view
	pd3dImmediateContext->OMSetRenderTargets(1, &pLightAccumRTV, NULL);

	// Constants
	ID3D11Buffer* arrConstBuffers[1] = {  RayTraceCB };
	pd3dImmediateContext->PSSetConstantBuffers(0, 1, arrConstBuffers);

	// Input
	ID3D11ShaderResourceView* arrViews[1] = {  LightRaysSRV };
	pd3dImmediateContext->PSSetShaderResources(0, 1, arrViews);

	// Primitive settings
	pd3dImmediateContext->IASetInputLayout(NULL);
	pd3dImmediateContext->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	pd3dImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// Set the shaders
	pd3dImmediateContext->VSSetShader( FullScreenVS, NULL, 0);
	pd3dImmediateContext->GSSetShader(NULL, NULL, 0);
	pd3dImmediateContext->PSSetShader( CombinePS, NULL, 0);

	pd3dImmediateContext->Draw(4, 0);

	// Cleanup
	arrViews[0] = NULL;
	pd3dImmediateContext->PSSetShaderResources(0, 1, arrViews);
	pd3dImmediateContext->VSSetShader(NULL, NULL, 0);
	pd3dImmediateContext->PSSetShader(NULL, NULL, 0);
	pd3dImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dImmediateContext->OMSetBlendState(pPrevBlendState, prevBlendFactor, prevSampleMask);
}

void SSLR::ImGui()
{
	ImGui::Begin("SSLR",0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);


	ImGui::SliderFloat("fMaxSunDist", (float*)&fMaxSunDist, 0.0f, 20.0f, "%0.3f", 1.0f);
    ImGui::SliderFloat("vSunPos", (float*)&dist, 0.0, 10000.0f);
    ImGui::SliderFloat("sunintencity", (float*)&intensity, 0.0f, 10.0f);
	

	ImGui::SliderFloat("sunColor R", &yellow.x, 0.0f, 1.0f);
	ImGui::SliderFloat("sunColor G", &yellow.y, 0.0f, 1.0f);
	ImGui::SliderFloat("sunColor B", &yellow.z, 0.0f, 1.0f);
	ImGui::SliderFloat("fInitDecay", &decay, 0.0f, 10.f, "%0.3f", 1.0f);

	ImGui::SliderFloat("fDistDecay", &ddecay, 0.0f, 10.f, "%0.3f", 1.0f);
	ImGui::SliderFloat("fMaxDeltaLen", &fMaxDeltaLen, 0.0f, 10.1f, "%0.3f", 1.0f);


	ImGui::End();
}
