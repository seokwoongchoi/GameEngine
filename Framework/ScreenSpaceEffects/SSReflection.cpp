#include "Framework.h"
#include "SSReflection.h"



SSReflection::SSReflection(Shader* deferredShader, uint width, uint height)
	:deferredShader(deferredShader), m_pReflectTexture(nullptr),m_ReflectRTV(nullptr), m_ReflectSRV(nullptr)
{
	ViewAngleThreshold= 0.2f;
	EdgeDistThreshold= 0.2f;
	DepthBias= 0.0025f;
	ReflectionScale= 2.0f;
	shader = new Shader(L"Deferred/SSReflection.fxo");
	/*D3D11_DEPTH_STENCIL_DESC descDepth;
	descDepth.DepthEnable = TRUE;
	descDepth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	descDepth.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	descDepth.StencilEnable = FALSE;
	V_RETURN(g_pDevice->CreateDepthStencilState(&descDepth, &m_pDepthEqualNoWrite));
	DXUT_SetDebugName(m_pDepthEqualNoWrite, "Reflection DS");

	D3D11_BLEND_DESC descBlend;
	descBlend.AlphaToCoverageEnable = FALSE;
	descBlend.IndependentBlendEnable = FALSE;
	const D3D11_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
	{
		TRUE,
		D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD,
		D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD,
		D3D11_COLOR_WRITE_ENABLE_ALL,
	};
*/
	reflectionBuffer = new ConstantBuffer(&reflectionCB, sizeof(SSReflectionCB));
	sReflectionBuffer = deferredShader->AsConstantBuffer("SSReflectionCB");
	// Create the HDR render target
	D3D11_TEXTURE2D_DESC dtd = {
		width, //UINT Width;
		height, //UINT Height;
		1, //UINT MipLevels;
		1, //UINT ArraySize;
		DXGI_FORMAT_R16G16B16A16_TYPELESS, //DXGI_FORMAT Format;
		1, //DXGI_SAMPLE_DESC SampleDesc;
		0,
		D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
		0,//UINT CPUAccessFlags;
		0//UINT MiscFlags;    
	};
	Check(D3D::GetDevice()->CreateTexture2D(&dtd, NULL, &m_pReflectTexture));
	
	D3D11_RENDER_TARGET_VIEW_DESC rtsvd =
	{
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		D3D11_RTV_DIMENSION_TEXTURE2D
	};
	Check(D3D::GetDevice()->CreateRenderTargetView(m_pReflectTexture, &rtsvd, &m_ReflectRTV));
	

	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd =
	{
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		0,
		0
	};
	dsrvd.Texture2D.MipLevels = 1;
	Check(D3D::GetDevice()->CreateShaderResourceView(m_pReflectTexture, &dsrvd, &m_ReflectSRV));
	

	
}

SSReflection::~SSReflection()
{
}

void SSReflection::PreRenderRelection(ID3D11ShaderResourceView * pDiffuseSRV, ID3D11DepthStencilView * ptDepthReadOnlyDSV)
{
	//pd3dImmediateContext->OMSetDepthStencilState(m_pDepthEqualNoWrite, 0);

	// Clear to black
	//float ClearColor[4] = { 0.0f, 0.0, 0.0, 0.0f };
	//D3D::GetDC()->ClearRenderTargetView(m_ReflectRTV, ClearColor);

	/*D3D11_VIEWPORT oldvp;
	UINT num = 1;
	D3D::GetDC()->RSGetViewports(&num, &oldvp);
	
	D3D11_VIEWPORT vp[1] = { { 0, 0, (float)1280, (float)720, 0.0f, 1.0f } };
	D3D::GetDC()->RSSetViewports(1, vp);*/
		
	//D3D::GetDC()->OMSetRenderTargets(1, &m_ReflectRTV, ptDepthReadOnlyDSV);
	

	
	reflectionCB.ViewAngleThreshold = ViewAngleThreshold;
	reflectionCB.EdgeDistThreshold = EdgeDistThreshold;
	reflectionCB.DepthBias = DepthBias;
	reflectionCB.ReflectionScale = ReflectionScale;

	if (reflectionBuffer)
	{
		reflectionBuffer->Apply();
		sReflectionBuffer->SetConstantBuffer(reflectionBuffer->Buffer());
	}
	deferredShader->AsSRV("HDRTex")->SetResource(pDiffuseSRV);

	D3D::Get()->SetRenderTarget(m_ReflectRTV, ptDepthReadOnlyDSV);
	D3D::Get()->Clear(Color(0, 0, 0, 1), m_ReflectRTV, ptDepthReadOnlyDSV);

	//D3D::GetDC()->RSSetViewports(1, &oldvp);

}


void SSReflection::DoReflectionBlend()
{
	//pd3dImmediateContext->OMSetBlendState(m_pAddativeBlendState, prevBlendFactor, prevSampleMask);
	shader->AsSRV("HDRTex")->SetResource(m_ReflectSRV);
	D3D::GetDC()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	shader->Draw(0, 0, 4);
}
