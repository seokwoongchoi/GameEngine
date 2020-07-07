#include "Framework.h"
#include "GBufferData.h"


void GBufferData::Init(Shader * shader, uint width, uint height)
{
	
	this->shader = shader;
	this->width = width;
	this->height = height;

		depthStencilTexture=nullptr;
		diffuseTexture=nullptr;
		normalTexture=nullptr;
		specularTexture=nullptr;
		depthStencilDSV=nullptr;
		depthStencilReadOnlyDSV=nullptr;
		diffuseRTV=nullptr;
		normalRTV=nullptr;
		specularRTV=nullptr;
		depthStencilSRV=nullptr;
		diffuseSRV=nullptr;
		normalSRV=nullptr;
		specularSRV=nullptr;
		depthStencilState = nullptr;

		CreateViews();
		
		
		unpackBuffer = new ConstantBuffer(&unpack, sizeof(CB_GBufferUnpack));
		sUnpackBuffer = shader->AsConstantBuffer("cbGBufferUnpack");


	
		sDiffuse = shader->AsSRV("ColorSpecIntTexture");
		sNormal = shader->AsSRV("NormalTexture");
		sSpecular = shader->AsSRV("SpecPowTexture");

		sDepth = shader->AsSRV("DepthTexture");
		sSsao = shader->AsSRV("SsaoTexture");
}

void GBufferData::Destroy()
{
	// Clear all allocated targets
	SafeRelease(depthStencilTexture);
	SafeRelease(diffuseTexture);
	SafeRelease(normalTexture);
	SafeRelease(specularTexture);

	// Clear all views
	SafeRelease(depthStencilDSV);
	SafeRelease(depthStencilReadOnlyDSV);
	SafeRelease(diffuseRTV);
	SafeRelease(normalRTV);
	SafeRelease(specularRTV);
	SafeRelease(depthStencilSRV);
	SafeRelease(diffuseSRV);
	SafeRelease(normalSRV);
	SafeRelease(specularSRV);

	// Clear the depth stencil state
	SafeRelease(depthStencilState);
}

void GBufferData::Update()
{
	proj = Context::Get()->Projection();
	view = Context::Get()->View();
	unpack.PerspectiveValuse.x = 1 / proj.m[0][0];
	unpack.PerspectiveValuse.y = 1 / proj.m[1][1];

	
	unpack.PerspectiveValuse.z = proj.m[3][2];
	unpack.PerspectiveValuse.w = -proj.m[2][2];
	
	D3DXMatrixInverse(&unpack.ViewInv, NULL, &view);

	
	
	

	
}

void GBufferData::Render(ID3D11ShaderResourceView* ssaoSRV)
{

	
	/*ID3D11ShaderResourceView* arrViews[5] = { depthStencilSRV,diffuseSRV, normalSRV ,specularSRV,ssaoSRV };
	D3D::GetDC()->PSSetShaderResources(0, 5, arrViews);*/
	sDepth->SetResource(depthStencilSRV);
	sDiffuse->SetResource(diffuseSRV);
	sNormal  ->SetResource(normalSRV);
	sSpecular->SetResource(specularSRV);
	sSsao->SetResource(ssaoSRV);
	unpackBuffer->Apply();
	sUnpackBuffer->SetConstantBuffer(unpackBuffer->Buffer());
	
	//// Cleanup
	//ZeroMemory(arrViews, sizeof(arrViews));
	//D3D::GetDC()->PSSetShaderResources(0,5, arrViews);
}



void GBufferData::CreateViews()
{
	// Clear all allocated targets
	SafeRelease(depthStencilTexture);
	SafeRelease(diffuseTexture);
	SafeRelease(normalTexture);
	SafeRelease(specularTexture);

	// Clear all views
	SafeRelease(depthStencilDSV);
	SafeRelease(depthStencilReadOnlyDSV);
	SafeRelease(diffuseRTV);
	SafeRelease(normalRTV);
	SafeRelease(specularRTV);
	SafeRelease(depthStencilSRV);
	SafeRelease(diffuseSRV);
	SafeRelease(normalSRV);
	SafeRelease(specularSRV);

	// Clear the depth stencil state
	SafeRelease(depthStencilState);


	// Texture formats
	static const DXGI_FORMAT depthStencilTextureFormat = DXGI_FORMAT_R24G8_TYPELESS;
	static const DXGI_FORMAT basicColorTextureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static const DXGI_FORMAT normalTextureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static const DXGI_FORMAT specPowTextureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Render view formats
	static const DXGI_FORMAT depthStencilRenderViewFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	static const DXGI_FORMAT basicColorRenderViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static const DXGI_FORMAT normalRenderViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static const DXGI_FORMAT specPowRenderViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Resource view formats
	static const DXGI_FORMAT depthStencilResourceViewFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	static const DXGI_FORMAT basicColorResourceViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static const DXGI_FORMAT normalResourceViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static const DXGI_FORMAT specPowResourceViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Allocate the depth stencil target
	D3D11_TEXTURE2D_DESC dtd = {
		width, //UINT Width;
		height, //UINT Height;
		1, //UINT MipLevels;
		1, //UINT ArraySize;
		DXGI_FORMAT_UNKNOWN, //DXGI_FORMAT Format;
		1, //DXGI_SAMPLE_DESC SampleDesc;
		0,
		D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
		0,//UINT CPUAccessFlags;
		0//UINT MiscFlags;    
	};
	dtd.Format = depthStencilTextureFormat;
	Check(D3D::GetDevice()->CreateTexture2D(&dtd, NULL, &depthStencilTexture));
	
	// Allocate the base color with specular intensity target
	dtd.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	dtd.Format = basicColorTextureFormat;
	Check(D3D::GetDevice()->CreateTexture2D(&dtd, NULL, &diffuseTexture));


	// Allocate the base color with specular intensity target
	dtd.Format = normalTextureFormat;
	Check(D3D::GetDevice()->CreateTexture2D(&dtd, NULL, &normalTexture));
	
	// Allocate the specular power target
	dtd.Format = specPowTextureFormat;
	Check(D3D::GetDevice()->CreateTexture2D(&dtd, NULL, &specularTexture));
	
	// Create the render target views
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd =
	{
		depthStencilRenderViewFormat,
		D3D11_DSV_DIMENSION_TEXTURE2D,
		0
	};
	Check(D3D::GetDevice()->CreateDepthStencilView(depthStencilTexture, &dsvd, &depthStencilDSV));


	dsvd.Flags = D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL;
	Check(D3D::GetDevice()->CreateDepthStencilView(depthStencilTexture, &dsvd, &depthStencilReadOnlyDSV));

	D3D11_RENDER_TARGET_VIEW_DESC rtsvd =
	{
		basicColorRenderViewFormat,
		D3D11_RTV_DIMENSION_TEXTURE2D
	};
	Check(D3D::GetDevice()->CreateRenderTargetView(diffuseTexture, &rtsvd, &diffuseRTV));
	

	rtsvd.Format = normalRenderViewFormat;
	Check(D3D::GetDevice()->CreateRenderTargetView(normalTexture, &rtsvd, &normalRTV));
	

	rtsvd.Format = specPowRenderViewFormat;
	Check(D3D::GetDevice()->CreateRenderTargetView(specularTexture, &rtsvd, &specularRTV));
	

	// Create the resource views
	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd =
	{
		depthStencilResourceViewFormat,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		0,
		0
	};
	dsrvd.Texture2D.MipLevels = 1;
	Check(D3D::GetDevice()->CreateShaderResourceView(depthStencilTexture, &dsrvd, &depthStencilSRV));
	dsrvd.Format = basicColorResourceViewFormat;
	Check(D3D::GetDevice()->CreateShaderResourceView(diffuseTexture, &dsrvd, &diffuseSRV));


	dsrvd.Format = normalResourceViewFormat;
	Check(D3D::GetDevice()->CreateShaderResourceView(normalTexture, &dsrvd, &normalSRV));
	
	dsrvd.Format = specPowResourceViewFormat;
	Check(D3D::GetDevice()->CreateShaderResourceView(specularTexture, &dsrvd, &specularSRV));
	

	D3D11_DEPTH_STENCIL_DESC descDepth;
	descDepth.DepthEnable = TRUE;
	descDepth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	descDepth.DepthFunc = D3D11_COMPARISON_LESS;
	descDepth.StencilEnable = TRUE;
	descDepth.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	descDepth.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	const D3D11_DEPTH_STENCILOP_DESC stencilMarkOp = { D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE, D3D11_COMPARISON_ALWAYS };
	descDepth.FrontFace = stencilMarkOp;
	descDepth.BackFace = stencilMarkOp;
	Check(D3D::GetDevice()->CreateDepthStencilState(&descDepth, &depthStencilState));
	

	
}
