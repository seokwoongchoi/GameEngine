#include "Framework.h"
#include "HDR.h"

//, fMiddleGrey(6.0), fWhite(6.0f)
HDR::HDR(uint width, uint height)
	:pass(0), fMiddleGrey(2.459f), fWhite(3.878f), m_fBloomThreshold(0.0), m_fBloomScale(1.1f), DC(NULL)

{// Find the amount of thread groups needed for the downscale operation

	
	this->width = width;
	this->height = height;
	downScaleGroups = 57;// (UINT)ceil((float)(width * height / 16) / 1024);

	// Allocate the downscaled target
	D3D11_TEXTURE2D_DESC dtd = {
		this->width / 4, //UINT Width;
		this->height / 4, //UINT Height;
		1, //UINT MipLevels;
		1, //UINT ArraySize;
		DXGI_FORMAT_R16G16B16A16_TYPELESS, //DXGI_FORMAT Format;
		1, //DXGI_SAMPLE_DESC SampleDesc;
		0,
		D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS,//UINT BindFlags;
		0,//UINT CPUAccessFlags;
		0//UINT MiscFlags;    
	};
	Check(D3D::GetDevice()->CreateTexture2D(&dtd, NULL, &m_pDownScaleRT));


	// Create the resource views
	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd;
	ZeroMemory(&dsrvd, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	dsrvd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	dsrvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	dsrvd.Texture2D.MipLevels = 1;
	Check(D3D::GetDevice()->CreateShaderResourceView(m_pDownScaleRT, &dsrvd, &m_pDownScaleSRV));


	// Create the UAVs
	D3D11_UNORDERED_ACCESS_VIEW_DESC DescUAV;
	ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	DescUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	DescUAV.Buffer.FirstElement = 0;
	DescUAV.Buffer.NumElements = this->width * this->height / 16;
	Check(D3D::GetDevice()->CreateUnorderedAccessView(m_pDownScaleRT, &DescUAV, &m_pDownScaleUAV));
	Check(D3D::GetDevice()->CreateTexture2D(&dtd, NULL, &m_pTempRT[0]));
	Check(D3D::GetDevice()->CreateShaderResourceView(m_pTempRT[0], &dsrvd, &m_pTempSRV[0]));
	Check(D3D::GetDevice()->CreateUnorderedAccessView(m_pTempRT[0], &DescUAV, &m_pTempUAV[0]));
	Check(D3D::GetDevice()->CreateTexture2D(&dtd, NULL, &m_pTempRT[1]));
	Check(D3D::GetDevice()->CreateShaderResourceView(m_pTempRT[1], &dsrvd, &m_pTempSRV[1]));
	Check(D3D::GetDevice()->CreateUnorderedAccessView(m_pTempRT[1], &DescUAV, &m_pTempUAV[1]));


	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocate bloom target
	Check(D3D::GetDevice()->CreateTexture2D(&dtd, NULL, &m_pBloomRT));
	Check(D3D::GetDevice()->CreateShaderResourceView(m_pBloomRT, &dsrvd, &m_pBloomSRV));
	Check(D3D::GetDevice()->CreateUnorderedAccessView(m_pBloomRT, &DescUAV, &m_pBloomUAV));

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocate down scaled luminance buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.StructureByteStride = sizeof(float);
	bufferDesc.ByteWidth = downScaleGroups * bufferDesc.StructureByteStride;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	Check(D3D::GetDevice()->CreateBuffer(&bufferDesc, NULL, &downScale1DBuffer));

//	D3D11_UNORDERED_ACCESS_VIEW_DESC DescUAV;
	ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.Format = DXGI_FORMAT_UNKNOWN;
	DescUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	DescUAV.Buffer.NumElements = downScaleGroups;
	Check(D3D::GetDevice()->CreateUnorderedAccessView(downScale1DBuffer, &DescUAV, &downScale1DUAV));
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocate average luminance buffer
	//D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd;
	ZeroMemory(&dsrvd, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	dsrvd.Format = DXGI_FORMAT_UNKNOWN;
	dsrvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	dsrvd.Buffer.NumElements = downScaleGroups;
	Check(D3D::GetDevice()->CreateShaderResourceView(downScale1DBuffer, &dsrvd, &downScale1DSRV));

	bufferDesc.ByteWidth = sizeof(float);
	Check(D3D::GetDevice()->CreateBuffer(&bufferDesc, NULL, &avgLumBuffer));
	Check(D3D::GetDevice()->CreateBuffer(&bufferDesc, NULL, &m_pPrevAvgLumBuffer));

	DescUAV.Buffer.NumElements = 1;
	Check(D3D::GetDevice()->CreateUnorderedAccessView(m_pPrevAvgLumBuffer, &DescUAV, &avgLumUAV));
	Check(D3D::GetDevice()->CreateUnorderedAccessView(avgLumBuffer, &DescUAV, &m_pPrevAvgLumUAV));

	dsrvd.Buffer.NumElements = 1;
	Check(D3D::GetDevice()->CreateShaderResourceView(avgLumBuffer, &dsrvd, &avgLumSRV));
	Check(D3D::GetDevice()->CreateShaderResourceView(m_pPrevAvgLumBuffer, &dsrvd, &m_pPrevAvgLumSRV));


	/////////////////////////////////////////////////////////////////////////
	const UINT nMaxBokehInst = 4056;
	bufferDesc.StructureByteStride = 7 * sizeof(float);
	bufferDesc.ByteWidth = nMaxBokehInst * bufferDesc.StructureByteStride;
	
	Check(D3D::GetDevice()->CreateBuffer(&bufferDesc, NULL, &m_pBokehBuffer));

	DescUAV.Buffer.FirstElement = 0;
	DescUAV.Buffer.NumElements = nMaxBokehInst;
	DescUAV.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
	Check(D3D::GetDevice()->CreateUnorderedAccessView(m_pBokehBuffer, &DescUAV, &m_pBokehUAV));


	dsrvd.Buffer.NumElements = nMaxBokehInst;
	Check(D3D::GetDevice()->CreateShaderResourceView(m_pBokehBuffer, &dsrvd, &m_pBokehSRV));
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
	bufferDesc.ByteWidth = 16;

	D3D11_SUBRESOURCE_DATA initData;
	UINT bufferInit[4] = { 0, 1, 0, 0 };
	initData.pSysMem = bufferInit;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	Check(D3D::GetDevice()->CreateBuffer(&bufferDesc, &initData, &m_pBokehIndirectDrawBuffer));

	wstring temp = L"../../_Textures/Environment/Bokeh.dds";
	//wstring temp = L"../../_Textures/Environment/sky_ocean.dds";
	D3DX11CreateShaderResourceViewFromFile
	(
		D3D::GetDevice(), temp.c_str(), NULL, NULL, &m_pBokehTexView, NULL
	);


	
	
	// Create the two samplers
	//D3D11_SAMPLER_DESC samDesc;
	//ZeroMemory(&samDesc, sizeof(samDesc));
	//
	//samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	//samDesc.AddressU = samDesc.AddressV = samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//
	//samDesc.MaxAnisotropy = 1;
	//samDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	//samDesc.MaxLOD = D3D11_FLOAT32_MAX;
	//Check(D3D::GetDevice()->CreateSamplerState(&samDesc, &g_pSampPoint));
	//shader->AsSampler("PointSampler")->SetSampler(0, g_pSampPoint);


	/////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocate constant buffers
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.ByteWidth = sizeof(TDownScaleCB);
	Check(D3D::GetDevice()->CreateBuffer(&bufferDesc, NULL, &m_pDownScaleCB));
//	DXUT_SetDebugName(m_pDownScaleCB, "PostFX - Down Scale CB");

	bufferDesc.ByteWidth = sizeof(TFinalPassCB);
	Check(D3D::GetDevice()->CreateBuffer(&bufferDesc, NULL, &m_pFinalPassCB));
	//DXUT_SetDebugName(m_pFinalPassCB, "PostFX - Final Pass CB");

	

	// Compile the shaders
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;// | D3DCOMPILE_WARNINGS_ARE_ERRORS;

	ID3DBlob* pShaderBlob = NULL;
	ID3DBlob* error;
	
	Check(D3DCompileFromFile(L"../Shader/Deferred/PostDownScaleFX.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "DownScaleFirstPass", "cs_5_0", dwShaderFlags, NULL, &pShaderBlob,&error));
	//Check(D3DCompileFromFile(str.c_str(), NULL, "DownScaleFirstPass", "cs_5_0", dwShaderFlags, &pShaderBlob));
	Check(D3D::GetDevice()->CreateComputeShader(pShaderBlob->GetBufferPointer(),
		pShaderBlob->GetBufferSize(), NULL, &m_pDownScaleFirstPassCS));
	//DXUT_SetDebugName(m_pDownScaleFirstPassCS, "Post FX - Down Scale First Pass CS");
	SafeRelease(pShaderBlob);

	Check(D3DCompileFromFile(L"../Shader/Deferred/PostDownScaleFX.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "DownScaleSecondPass", "cs_5_0", dwShaderFlags, NULL, &pShaderBlob, &error));
	//V_RETURN(CompileShader(str, NULL, "DownScaleSecondPass", "cs_5_0", dwShaderFlags, &pShaderBlob));
	Check(D3D::GetDevice()->CreateComputeShader(pShaderBlob->GetBufferPointer(),
		pShaderBlob->GetBufferSize(), NULL, &m_pDownScaleSecondPassCS));
	//DXUT_SetDebugName(m_pDownScaleSecondPassCS, "Post FX - Down Scale Second Pass CS");
	SafeRelease(pShaderBlob);

	Check(D3DCompileFromFile(L"../Shader/Deferred/PostDownScaleFX.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BloomReveal", "cs_5_0", dwShaderFlags, NULL, &pShaderBlob, &error));
	//V_RETURN(CompileShader(str, NULL, "BloomReveal", "cs_5_0", dwShaderFlags, &pShaderBlob));
	Check(D3D::GetDevice()->CreateComputeShader(pShaderBlob->GetBufferPointer(),
		pShaderBlob->GetBufferSize(), NULL, &m_pBloomRevealCS));
	//DXUT_SetDebugName(m_pBloomRevealCS, "Post FX - Bloom Reveal CS");
	SafeRelease(pShaderBlob);


	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// NVidia Gaussian Blur

	Check(D3DCompileFromFile(L"../Shader/Deferred/Blur.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VerticalFilter", "cs_5_0", dwShaderFlags, NULL, &pShaderBlob, &error));
	//V_RETURN(CompileShader(str, NULL, "VerticalFilter", "cs_5_0", dwShaderFlags, &pShaderBlob));
	Check(D3D::GetDevice()->CreateComputeShader(pShaderBlob->GetBufferPointer(),
		pShaderBlob->GetBufferSize(), NULL, &m_VerticalBlurCS));
	//DXUT_SetDebugName(m_VerticalBlurCS, "Post FX - Vertical Blur CS");
	SafeRelease(pShaderBlob);
	Check(D3DCompileFromFile(L"../Shader/Deferred/Blur.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "HorizFilter", "cs_5_0", dwShaderFlags, NULL, &pShaderBlob, &error));
	//V_RETURN(CompileShader(str, NULL, "HorizFilter", "cs_5_0", dwShaderFlags, &pShaderBlob));
	Check(D3D::GetDevice()->CreateComputeShader(pShaderBlob->GetBufferPointer(),
		pShaderBlob->GetBufferSize(), NULL, &m_HorizontalBlurCS));
//	DXUT_SetDebugName(m_HorizontalBlurCS, "Post FX - Horizontal Blur CS");
	SafeRelease(pShaderBlob);

	//V_RETURN(DXUTFindDXSDKMediaFileCch(str, MAX_PATH, L"PostFX.hlsl"));
	Check(D3DCompileFromFile(L"../Shader/Deferred/PostEffect.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "FullScreenQuadVS", "vs_5_0", dwShaderFlags, NULL, &pShaderBlob, &error));
	//V_RETURN(CompileShader(str, NULL, "FullScreenQuadVS", "vs_5_0", dwShaderFlags, &pShaderBlob));
	Check(D3D::GetDevice()->CreateVertexShader(pShaderBlob->GetBufferPointer(),
		pShaderBlob->GetBufferSize(), NULL, &m_pFullScreenQuadVS));
	//DXUT_SetDebugName(m_pFullScreenQuadVS, "Post FX - Full Screen Quad VS");
	SafeRelease(pShaderBlob);
	Check(D3DCompileFromFile(L"../Shader/Deferred/PostEffect.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "FinalPassPS", "ps_5_0", dwShaderFlags, NULL, &pShaderBlob, &error));
	//V_RETURN(CompileShader(str, NULL, "FinalPassPS", "ps_5_0", dwShaderFlags, &pShaderBlob));
	Check(D3D::GetDevice()->CreatePixelShader(pShaderBlob->GetBufferPointer(),
		pShaderBlob->GetBufferSize(), NULL, &m_pFinalPassPS));
	//DXUT_SetDebugName(m_pFinalPassPS, "Post FX - Final Pass PS");
	SafeRelease(pShaderBlob);

	
	
	/*ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.ByteWidth = sizeof(TDownScaleCB);
	Check(D3D::GetDevice()->CreateBuffer(&bufferDesc, NULL, &downScaleCB));

	bufferDesc.ByteWidth = sizeof(TFinalPassCB);
	Check(D3D::GetDevice()->CreateBuffer(&bufferDesc, NULL, &finalPassCB));*/

	

	// Create the two samplers
	D3D11_SAMPLER_DESC samDesc;
	ZeroMemory(&samDesc, sizeof(samDesc));
	samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samDesc.AddressU = samDesc.AddressV = samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.MaxAnisotropy = 1;
	samDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Check(D3D::GetDevice()->CreateSamplerState(&samDesc, &g_pSampLinear));
	//DXUT_SetDebugName(g_pSampLinear, "Linear Sampler");

	samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	Check(D3D::GetDevice() ->CreateSamplerState(&samDesc, &g_pSampPoint));
	//DXUT_SetDebugName(g_pSampPoint, "Point Sampler");
	
}

HDR::~HDR()
{
	SafeRelease(downScale1DBuffer);
	SafeRelease(downScale1DUAV);
	SafeRelease(downScale1DSRV);
	//SafeRelease(downScaleCB);
	//SafeRelease(finalPassCB);
	SafeRelease(avgLumBuffer);
	SafeRelease(avgLumUAV);
	SafeRelease(avgLumSRV);
}

void HDR::PostProcessing(ID3D11ShaderResourceView * pHDRSRV, ID3D11RenderTargetView * oldTarget, ID3D11ShaderResourceView* dsv)
{
	DC = D3D::GetDC();
	
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	DC->Map(m_pDownScaleCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	auto pDownScale = static_cast<TDownScaleCB*>(MappedResource.pData);
	pDownScale->nWidth = width / 4;
	pDownScale->nHeight = height / 4;
	pDownScale->nTotalPixels = pDownScale->nWidth * pDownScale->nHeight;
	pDownScale->nGroupSize = downScaleGroups;
	pDownScale->fAdaptation = m_fAdaptation;
	pDownScale->fBloomThreshold = m_fBloomThreshold;
	DC->Unmap(m_pDownScaleCB, 0);
	ID3D11Buffer* arrConstBuffers[1] = { m_pDownScaleCB };
	DC->CSSetConstantBuffers(0, 1, arrConstBuffers);

	ID3D11RenderTargetView* rt[1] = { NULL };
	DC->OMSetRenderTargets(1, rt, nullptr);
	
	//Thread::Get()->AddTask([&]()
	//{
		DownScale(pHDRSRV);
		//DownScaleBlur();
		//BokehHightlightScan(pHDRSRV, dsv);
		Bloom();
		Blur(m_pTempSRV[0], m_pBloomUAV);
	//});
	// Cleanup
		ZeroMemory(&arrConstBuffers, sizeof(arrConstBuffers));
		DC->CSSetConstantBuffers(0, 1, arrConstBuffers);

	
	/*ID3D11CommandList* commadList = D3D::GetCommand();
	DC->FinishCommandList(false, &commadList);*/




	rt[0] = oldTarget;
	D3D::GetDC()->OMSetRenderTargets(1, rt, nullptr);
	FinalPass(pHDRSRV,dsv);
	ID3D11Buffer* pTempBuffer = avgLumBuffer;
	ID3D11UnorderedAccessView* pTempUAV = avgLumUAV;
	ID3D11ShaderResourceView* p_TempSRV = avgLumSRV;
	avgLumBuffer = m_pPrevAvgLumBuffer;
	avgLumUAV = m_pPrevAvgLumUAV;
	avgLumSRV = m_pPrevAvgLumSRV;
	m_pPrevAvgLumBuffer = pTempBuffer;
	m_pPrevAvgLumUAV = pTempUAV;
	m_pPrevAvgLumSRV = p_TempSRV;
	//BokehRender();
}

void HDR::DownScale( ID3D11ShaderResourceView * srv)
{
	
	ID3D11UnorderedAccessView* arrUAVs[2] = { downScale1DUAV, m_pDownScaleUAV };
	DC->CSSetUnorderedAccessViews(0, 2, arrUAVs, NULL);
	// Input
	ID3D11ShaderResourceView* arrViews[2] = { srv, NULL };
	DC->CSSetShaderResources(0, 1, arrViews);
	
	DC->CSSetShader(m_pDownScaleFirstPassCS, NULL, 0);
	DC->Dispatch(downScaleGroups, 1, 1);


	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Second pass - reduce to a single pixel

	// Outoput
	ZeroMemory(arrUAVs, sizeof(arrUAVs));
	arrUAVs[0] = avgLumUAV;
	DC->CSSetUnorderedAccessViews(0, 2, arrUAVs, NULL);

	// Input
	arrViews[0] = downScale1DSRV;
	arrViews[1] = m_pPrevAvgLumSRV;
	DC->CSSetShaderResources(0, 2, arrViews);

	// Shader
	DC->CSSetShader(m_pDownScaleSecondPassCS, NULL, 0);

	// Excute with a single group - this group has enough threads to process all the pixels
	DC->Dispatch(1, 1, 1);

	// Cleanup
	DC->CSSetShader(NULL, NULL, 0);
	ZeroMemory(arrViews, sizeof(arrViews));
	DC->CSSetShaderResources(0, 2, arrViews);
	ZeroMemory(arrUAVs, sizeof(arrUAVs));
	DC->CSSetUnorderedAccessViews(0, 2, arrUAVs, NULL);

	//PrevAvgLumEffect->SetResource(m_pPrevAvgLumSRV);
	//AverageValues1DEffect->SetResource(downScale1DSRV);
	//AverageLumEffect->SetUnorderedAccessView(avgLumUAV);
	//downScaleShader->Dispatch(0, 1, 1, 1, 1);


}

void HDR::DownScaleBlur()
{
	/*sDownScaleUAV->SetUnorderedAccessView(m_pDownScaleUAV);
	downScaleShader->Dispatch(0, 2, downScaleGroups, 1, 1);*/
	
}

void HDR::Bloom()
{

	// Input
	ID3D11ShaderResourceView* arrViews[2] = { m_pDownScaleSRV, avgLumSRV };
	DC->CSSetShaderResources(0, 2, arrViews);

	// Output
	ID3D11UnorderedAccessView* arrUAVs[1] = { m_pTempUAV[0] };
	DC->CSSetUnorderedAccessViews(0, 1, arrUAVs, NULL);

	// Shader
	DC->CSSetShader(m_pBloomRevealCS, NULL, 0);

	// Execute the downscales first pass with enough groups to cover the entire full res HDR buffer
	DC->Dispatch(downScaleGroups, 1, 1);

	// Cleanup
	DC->CSSetShader(NULL, NULL, 0);
	ZeroMemory(arrViews, sizeof(arrViews));
	DC->CSSetShaderResources(0, 2, arrViews);
	ZeroMemory(arrUAVs, sizeof(arrUAVs));
	DC->CSSetUnorderedAccessViews(0, 1, arrUAVs, NULL);

	
	
	//sfirstDownScaleSRV->SetResource(m_pDownScaleSRV);//0
	//ffirstAvgLumEffect->SetResource(avgLumSRV);//1

	//fBloomEffect->SetUnorderedAccessView(m_pTempUAV[0]);

	//downScaleShader->Dispatch(0, 3, downScaleGroups, 1, 1); 
	////Blur(m_pTempSRV[0], m_pBloomUAV);
}

void HDR::ImGui()
{
	ImGui::Begin("HDR", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	ImGui::SliderFloat("fWhite", (float*)&fWhite, 0, 15.0f);
	ImGui::SliderFloat("fMiddleGrey", (float*)&fMiddleGrey, 0, 15.0f,"%0.3f",1.0f);
	ImGui::SliderFloat("DOFFarStart", (float*)&DOFFarValues1, 0.0f, 1000.0f, "%0.3f", 1.0f);
	ImGui::SliderFloat("RangeRcp", (float*)&DOFFarValues2, 0.0f, 1000.0f, "%0.3f", 1.0f);
	ImGui::SliderFloat("BloomScale", (float*)&m_fBloomScale, 0.1f, 5.0f, "%0.3f", 1.0f);
	ImGui::SliderFloat("BloomTheadHold", (float*)&m_fBloomThreshold, 0.0f, 2.0f, "%0.3f", 1.0f);


	ImGui::End();
}

void HDR::BokehHightlightScan(ID3D11ShaderResourceView * pHDRSRV, ID3D11ShaderResourceView * pDepthSRV)
{
	bokehUAVEffect->SetUnorderedAccessView(m_pBokehUAV);
	bokehHDRTextureEffect->SetResource(pHDRSRV);
	bokehDepthTextureEffect->SetResource(pDepthSRV);
	bokehAvgLumEffect->SetResource(avgLumSRV);

	BokehHightlightScanCB.nWidth = width;
	BokehHightlightScanCB.nHeight = height;

	proj= Context::Get()->Projection();
	
	BokehHightlightScanCB.ProjectionValues[0] = proj.m[3][2];
	BokehHightlightScanCB.ProjectionValues[1] = -proj.m[2][2];

	
	BokehHightlightScanCB.fDOFFarStart = DOFFarValues1;
	BokehHightlightScanCB.fDOFFarRangeRcp = 1.0f/DOFFarValues2;
	BokehHightlightScanCB.fMiddleGrey = fMiddleGrey;
	BokehHightlightScanCB.fLumWhiteSqr = fWhite;
	BokehHightlightScanCB.fLumWhiteSqr *= BokehHightlightScanCB.fMiddleGrey; // Scale by the middle gray value
	BokehHightlightScanCB.fLumWhiteSqr *= BokehHightlightScanCB.fLumWhiteSqr; // Square
	
	ImGui::SliderFloat("m_fBokehBlurThreshold", (float*)&m_fBokehBlurThreshold, 0.f, 10.0f);
	BokehHightlightScanCB.fBokehBlurThreshold = m_fBokehBlurThreshold;
	ImGui::SliderFloat("m_fBokehLumThreshold", (float*)&m_fBokehLumThreshold, 0.f, 10.0f);
	BokehHightlightScanCB.fBokehLumThreshold = m_fBokehLumThreshold;
	ImGui::SliderFloat("m_fBokehRadiusScale", (float*)&m_fBokehRadiusScale, 0.f, 10.0f);
	BokehHightlightScanCB.fRadiusScale = m_fBokehRadiusScale;
	ImGui::SliderFloat("m_fBokehColorScale", (float*)&m_fBokehColorScale, 0.f, 10.0f);
	BokehHightlightScanCB.fColorScale = m_fBokehColorScale;
	

	bokehCSBuffer->Apply();
	sBokehCSBuffer->SetConstantBuffer(bokehCSBuffer->Buffer());

	//bokehCSShader->Dispatch(0,0,(UINT)ceil((float)(width * height) /1024.0f), 1, 1);
	//bokehCSShader->Dispatch(0, 0, 1280, 1, 1);
}

void HDR::Blur(ID3D11ShaderResourceView * pInput, ID3D11UnorderedAccessView * pOutput)
{
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Second pass - horizontal Gaussian filter

	// Output
	ID3D11UnorderedAccessView* arrUAVs[1] = { m_pTempUAV[1] };
	DC->CSSetUnorderedAccessViews(0, 1, arrUAVs, NULL);

	// Input
	ID3D11ShaderResourceView* arrViews[1] = { pInput };
	DC->CSSetShaderResources(0, 1, arrViews);

	// Shader
	DC->CSSetShader(m_HorizontalBlurCS, NULL, 0);

	// Execute the horizontal filter
	DC->Dispatch((UINT)ceil((width / 4.0f) / (128.0f - 12.0f)), (UINT)ceil(height / 4.0f), 1);

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// First pass - vertical Gaussian filter

	// Output
	arrUAVs[0] = pOutput;
	DC->CSSetUnorderedAccessViews(0, 1, arrUAVs, NULL);

	// Input
	arrViews[0] = m_pTempSRV[1];
	DC->CSSetShaderResources(0, 1, arrViews);

	// Shader
	DC->CSSetShader(m_VerticalBlurCS, NULL, 0);

	// Execute the vertical filter
	DC->Dispatch((UINT)ceil(width / 4.0f), (UINT)ceil((height / 4.0f) / (128.0f - 12.0f)), 1);

	// Cleanup
	DC->CSSetShader(NULL, NULL, 0);
	ZeroMemory(arrViews, sizeof(arrViews));
	DC->CSSetShaderResources(0, 1, arrViews);
	ZeroMemory(arrUAVs, sizeof(arrUAVs));
	DC->CSSetUnorderedAccessViews(0, 1, arrUAVs, NULL);

	//////////////////////////////////////////////////////////////////////////////////////////////////////
 //   // Second pass - horizontal gaussian filter
	//blurShader->AsSRV("Input")->SetResource(pInput);
	//blurShader->AsUAV("Output")->SetUnorderedAccessView(m_pTempUAV[1]);

	//// Execute the horizontal filter
	////blurShader->Dispatch(0, 1, (UINT)ceil((width / 4.0f) / (128.0f - 12.0f)), (UINT)ceil(height / 4.0f), 1);

	//blurShader->Dispatch(0, 1,1280,720, 1);


	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//// First pass - vertical gaussian filter
	//blurShader->AsSRV("Input")->SetResource(m_pTempSRV[1]);
	//blurShader->AsUAV("Output")->SetUnorderedAccessView(pOutput);

	////blurShader->Dispatch(0, 0, (UINT)ceil(width / 4.0f), (UINT)ceil((height / 4.0f) / (128.0f - 12.0f)), 1);
	//blurShader->Dispatch(0, 0,1280,720, 1);
}


void HDR::FinalPass(ID3D11ShaderResourceView* srv, ID3D11ShaderResourceView* dsv)
{

	/*auto commandList = D3D::GetCommand();
	D3D::GetDC()->ExecuteCommandList(commandList, false);*/
	

//	proj = Context::Get()->Projection();
	
	//ID3D11ShaderResourceView* arrViews[6] = { srv, avgLumSRV, m_pBloomSRV, m_pDownScaleSRV, dsv };
	ID3D11ShaderResourceView* arrViews[3] = { srv, avgLumSRV, m_pBloomSRV };
	D3D::GetDC()->PSSetShaderResources(0, 3, arrViews);

	// Constants
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	D3D::GetDC()->Map(m_pFinalPassCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	TFinalPassCB* pFinalPass = (TFinalPassCB*)MappedResource.pData;
	pFinalPass->fMiddleGrey = fMiddleGrey;
	pFinalPass->fLumWhiteSqr = fWhite;
	pFinalPass->fLumWhiteSqr *= pFinalPass->fMiddleGrey; // Scale by the middle grey value
	pFinalPass->fLumWhiteSqr *= pFinalPass->fLumWhiteSqr; // Square
	pFinalPass->fBloomScale = m_fBloomScale;
	
	D3D::GetDC()->Unmap(m_pFinalPassCB, 0);
	ID3D11Buffer* arrConstBuffers[1] = { m_pFinalPassCB };
	D3D::GetDC()->PSSetConstantBuffers(0, 1, arrConstBuffers);
	

	D3D::GetDC()->IASetInputLayout(NULL);
	D3D::GetDC()->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	D3D::GetDC()->IASetIndexBuffer(NULL, DXGI_FORMAT_UNKNOWN, 0);
	D3D::GetDC()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D11SamplerState* arrSamplers[2] = { g_pSampPoint, g_pSampLinear };
	D3D::GetDC()->PSSetSamplers(0, 2, arrSamplers);

	// Set the shaders
	D3D::GetDC()->VSSetShader(m_pFullScreenQuadVS, NULL, 0);
	D3D::GetDC()->PSSetShader(m_pFinalPassPS, NULL, 0);

	D3D::GetDC()->Draw(4, 0);

	// Cleanup
	ZeroMemory(arrViews, sizeof(arrViews));
	D3D::GetDC()->PSSetShaderResources(0, 3, arrViews);
	ZeroMemory(arrConstBuffers, sizeof(arrConstBuffers));
	D3D::GetDC()->PSSetConstantBuffers(0, 1, arrConstBuffers);
	D3D::GetDC()->VSSetShader(NULL, NULL, 0);
	D3D::GetDC()->PSSetShader(NULL, NULL, 0);
	//ZeroMemory(arrViews, sizeof(arrViews));
	

	
	
}

void HDR::BokehRender()
{
	
	/*DC->CopyStructureCount(m_pBokehIndirectDrawBuffer, 0, m_pBokehUAV);
	

	BokehRenderCB.AspectRatio[0] = 1.0f;
	BokehRenderCB.AspectRatio[1] = (float)width / (float)height;



	bokehSRVEffect->SetResource(m_pBokehSRV);

	bokehRenderBuffer->Apply();
	sBokehRenderBuffer->SetConstantBuffer(bokehRenderBuffer->Buffer());


	DC->IASetInputLayout(NULL);
	DC->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	DC->IASetIndexBuffer(NULL, DXGI_FORMAT_UNKNOWN, 0);
	DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
*/
	

	//bokehShader->DrawInstancedIndirect(0,0, m_pBokehIndirectDrawBuffer,0);
	
}




