#include "Framework.h"
#include "SSAO.h"

SSAO::SSAO(uint width, uint height)
	:SSAOSampRadius(0.9f), Radius(13.0f) ,m_pDownscaleCB(NULL),
	m_pSSAO_RT(NULL), m_pSSAO_SRV(NULL), m_pSSAO_UAV(NULL),
	m_pMiniDepthBuffer(NULL), m_pMiniDepthUAV(NULL), m_pMiniDepthSRV(NULL),
	m_pDepthDownscaleCS(NULL), m_pComputeCS(NULL)
{

	this->width = width / 2;
	this->height = height / 2;

	downScaleGroups = (UINT)ceil((float)(this->width * this->height) / 1024.0f);
	
	


	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocate SSAO
	D3D11_TEXTURE2D_DESC t2dDesc = {
		this->width, //UINT Width;
		this->height, //UINT Height;
		1, //UINT MipLevels;
		1, //UINT ArraySize;
		DXGI_FORMAT_R32_TYPELESS,//DXGI_FORMAT_R8_TYPELESS, //DXGI_FORMAT Format;
		1, //DXGI_SAMPLE_DESC SampleDesc;
		0,
		D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
		0,//UINT CPUAccessFlags;
		0//UINT MiscFlags;    
	}; 
	
	Check(D3D::GetDevice()->CreateTexture2D(&t2dDesc, NULL, &m_pSSAO_RT));
	

	// Create the UAVs
	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
	ZeroMemory(&UAVDesc, sizeof(UAVDesc));
	UAVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		
	
	Check(D3D::GetDevice()->CreateUnorderedAccessView(m_pSSAO_RT, &UAVDesc, &m_pSSAO_UAV));
	

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
	SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;
	Check(D3D::GetDevice()->CreateShaderResourceView(m_pSSAO_RT, &SRVDesc, &m_pSSAO_SRV));


	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocate down scaled depth buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.StructureByteStride = 4 * sizeof(float);
	bufferDesc.ByteWidth = this->width * this->height * bufferDesc.StructureByteStride;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    Check(D3D::GetDevice()->CreateBuffer(&bufferDesc, NULL, &m_pMiniDepthBuffer));
	
	ZeroMemory(&UAVDesc, sizeof(UAVDesc));
	UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	UAVDesc.Buffer.FirstElement = 0;
	UAVDesc.Buffer.NumElements = this->width * this->height;
	Check(D3D::GetDevice()->CreateUnorderedAccessView(m_pMiniDepthBuffer, &UAVDesc, &m_pMiniDepthUAV));
	

	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SRVDesc.Buffer.FirstElement = 0;
	SRVDesc.Buffer.NumElements = this->width * this->height;
	Check(D3D::GetDevice()->CreateShaderResourceView(m_pMiniDepthBuffer, &SRVDesc, &m_pMiniDepthSRV));
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocate down scale depth constant buffer
	D3D11_BUFFER_DESC CBDesc;
	ZeroMemory(&CBDesc, sizeof(CBDesc));
	CBDesc.Usage = D3D11_USAGE_DYNAMIC;
	CBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	CBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	CBDesc.ByteWidth = sizeof(DownscaleCB);
	Check(D3D::GetDevice()->CreateBuffer(&CBDesc, NULL, &m_pDownscaleCB));
	//DXUT_SetDebugName(m_pDownscaleCB, "SSAO - Downscale Depth CB");

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Compile the shaders
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;// | D3DCOMPILE_WARNINGS_ARE_ERRORS;


	ID3DBlob* error;
	ID3DBlob* pShaderBlob = NULL;
	//V_RETURN(DXUTFindDXSDKMediaFileCch(str, MAX_PATH, L"SSAO.hlsl"));
	Check(D3DCompileFromFile(L"../Shader/Deferred/SSAO.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "DepthDownscale", "cs_5_0", dwShaderFlags, NULL, &pShaderBlob, &error));
	//V_RETURN(CompileShader(str, NULL, "DepthDownscale", "cs_5_0", dwShaderFlags, &pShaderBlob));
	Check(D3D::GetDevice()->CreateComputeShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), NULL, &m_pDepthDownscaleCS));
	SafeRelease(pShaderBlob);
	Check(D3DCompileFromFile(L"../Shader/Deferred/SSAO.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "SSAOCompute", "cs_5_0", dwShaderFlags, NULL, &pShaderBlob, &error));
	//V_RETURN(CompileShader(str, NULL, "SSAOCompute", "cs_5_0", dwShaderFlags, &pShaderBlob));
	Check(D3D::GetDevice()->CreateComputeShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), NULL, &m_pComputeCS));
	SafeRelease(pShaderBlob);

	
}

SSAO::~SSAO()
{
	

	SafeRelease(m_pDownscaleCB);
	SafeRelease(m_pSSAO_RT);
	SafeRelease(m_pSSAO_SRV);
	SafeRelease(m_pSSAO_UAV);
	SafeRelease(m_pMiniDepthBuffer);
	SafeRelease(m_pMiniDepthUAV);
	SafeRelease(m_pMiniDepthSRV);
	SafeRelease(m_pDepthDownscaleCS);
	SafeRelease(m_pComputeCS);

	
}

void SSAO::Compute(ID3D11ShaderResourceView * DepthSRV, ID3D11ShaderResourceView * NormalsSRV)
{
	auto DC = D3D::GetDC();
	DownscaleDepth(DC,DepthSRV, NormalsSRV);
	ComputeSSAO(DC);
}

void SSAO::DownscaleDepth(ID3D11DeviceContext* DC,ID3D11ShaderResourceView * DepthSRV, ID3D11ShaderResourceView * NormalsSRV)
{
	
	
	// Constants
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	DC->Map(m_pDownscaleCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	DownscaleCB* pDownscale = static_cast<DownscaleCB*>(MappedResource.pData);
	pDownscale->nWidth = width;
	pDownscale->nHeight = height;
	pDownscale->fHorResRcp = 1.0f / (float)pDownscale->nWidth;
	pDownscale->fVerResRcp = 1.0f / (float)pDownscale->nHeight;
	proj = Context::Get()->Projection();
	pDownscale->ProjParams.x = 1.0f / proj.m[0][0];
	pDownscale->ProjParams.y = 1.0f / proj.m[1][1];
	float fQ =1000.0f / 1000.0f-0.1f;
	pDownscale->ProjParams.z = -0.1f * fQ;
	pDownscale->ProjParams.w = -fQ;
	//D3DXMatrixTranspose(&pDownscale->ViewMatrix , &Context::Get()->View());
	pDownscale->ViewMatrix=Context::Get()->View();

	ImGui::SliderFloat("SSAOSampRadius", (float*)&SSAOSampRadius, 0.0f, 20.0f);

	pDownscale->fOffsetRadius = (float)SSAOSampRadius;
	ImGui::SliderFloat("Radius", (float*)&Radius, 0.0f, 20.0f);
	pDownscale->fRadius = Radius;
	pDownscale->fMaxDepth = 1000.0f;
	DC->Unmap(m_pDownscaleCB, 0);
	ID3D11Buffer* arrConstBuffers[1] = { m_pDownscaleCB };
	DC->CSSetConstantBuffers(0, 1, arrConstBuffers);

	// Output
	ID3D11UnorderedAccessView* arrUAVs[1] = { m_pMiniDepthUAV };
	DC->CSSetUnorderedAccessViews(0, 1, arrUAVs, NULL);

	// Input
	ID3D11ShaderResourceView* arrViews[2] = { DepthSRV, NormalsSRV };
	DC->CSSetShaderResources(0, 2, arrViews);

	// Shader
	DC->CSSetShader(m_pDepthDownscaleCS, NULL, 0);

	// Execute the downscales first pass with enough groups to cover the entire full res HDR buffer
	DC->Dispatch(downScaleGroups, 1, 1);

	// Cleanup
	DC->CSSetShader(NULL, NULL, 0);
	ZeroMemory(arrViews, sizeof(arrViews));
	DC->CSSetShaderResources(0, 2, arrViews);
	ZeroMemory(arrUAVs, sizeof(arrUAVs));
	DC->CSSetUnorderedAccessViews(0, 1, arrUAVs, NULL);
	ZeroMemory(arrConstBuffers, sizeof(arrConstBuffers));
	DC->CSSetConstantBuffers(0, 1, arrConstBuffers);
}

void SSAO::ComputeSSAO(ID3D11DeviceContext* DC)
{
	
	ID3D11Buffer* arrConstBuffers[1] = { m_pDownscaleCB };
	DC->CSSetConstantBuffers(0, 1, arrConstBuffers);

	// Output
	ID3D11UnorderedAccessView* arrUAVs[1] = { m_pSSAO_UAV };
	DC->CSSetUnorderedAccessViews(0, 1, arrUAVs, NULL);

	// Input
	ID3D11ShaderResourceView* arrViews[1] = { m_pMiniDepthSRV };
	DC->CSSetShaderResources(0, 1, arrViews);

	// Shader
	DC->CSSetShader(m_pComputeCS, NULL, 0);

	// Execute the downscales first pass with enough groups to cover the entire full res HDR buffer
	DC->Dispatch(downScaleGroups, 1, 1);

	// Cleanup
	DC->CSSetShader(NULL, NULL, 0);
	ZeroMemory(arrViews, sizeof(arrViews));
	DC->CSSetShaderResources(0, 1, arrViews);
	ZeroMemory(arrUAVs, sizeof(arrUAVs));
	DC->CSSetUnorderedAccessViews(0, 1, arrUAVs, NULL);
	ZeroMemory(arrConstBuffers, sizeof(arrConstBuffers));
	DC->CSSetConstantBuffers(0, 1, arrConstBuffers);
}
