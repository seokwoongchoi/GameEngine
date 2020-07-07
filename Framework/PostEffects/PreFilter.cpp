#include "Framework.h"
#include "PreFilter.h"

PreFilter::PreFilter()
	
{
	wstring temp = L"../../_Textures/Environment/sky.dds";
	D3DX11CreateShaderResourceViewFromFile
	(
		D3D::GetDevice(), temp.c_str(), NULL, NULL, &cubeSRV, NULL
	);
	
}

PreFilter::~PreFilter()
{
}

void PreFilter::CreateBRDF(ID3D11DepthStencilView* dsv)
{
	// INTEGRATE BRDF & CREATE LUT

	D3D11_TEXTURE2D_DESC brdfLUTDesc;
	//ZeroMemory(&skyIBLDesc, sizeof(skyIBLDesc));
	brdfLUTDesc.Width = 512;
	brdfLUTDesc.Height = 512;
	brdfLUTDesc.MipLevels = 1;
	brdfLUTDesc.ArraySize = 1;
	brdfLUTDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	brdfLUTDesc.Usage = D3D11_USAGE_DEFAULT;
	brdfLUTDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	brdfLUTDesc.CPUAccessFlags = 0;
	brdfLUTDesc.MiscFlags = 0;
	brdfLUTDesc.SampleDesc.Count = 1;
	brdfLUTDesc.SampleDesc.Quality = 0;
	//---
	ID3D11RenderTargetView* brdfLUTRTV;
	//--
	D3D11_RENDER_TARGET_VIEW_DESC brdfLUTRTVDesc;
	ZeroMemory(&brdfLUTRTVDesc, sizeof(brdfLUTRTVDesc));
	brdfLUTRTVDesc.Format = brdfLUTDesc.Format;
	brdfLUTRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	//---
	D3D11_SHADER_RESOURCE_VIEW_DESC brdfLUTSRVDesc;
	ZeroMemory(&brdfLUTSRVDesc, sizeof(brdfLUTSRVDesc));
	brdfLUTSRVDesc.Format = brdfLUTSRVDesc.Format;
	brdfLUTSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	brdfLUTSRVDesc.TextureCube.MostDetailedMip = 0;
	brdfLUTSRVDesc.TextureCube.MipLevels = 1;
	//---
	D3D11_VIEWPORT brdfLUTviewport;
	brdfLUTviewport.Width = 512;
	brdfLUTviewport.Height = 512;
	brdfLUTviewport.MinDepth = 0.0f;
	brdfLUTviewport.MaxDepth = 1.0f;
	brdfLUTviewport.TopLeftX = 0.0f;
	brdfLUTviewport.TopLeftY = 0.0f;
	//---
	Check(D3D::GetDevice()->CreateTexture2D(&brdfLUTDesc, 0, &brdfLUTtex));
	Check(D3D::GetDevice()->CreateRenderTargetView(brdfLUTtex, &brdfLUTRTVDesc, &brdfLUTRTV));
	Check(D3D::GetDevice()->CreateShaderResourceView(brdfLUTtex, &brdfLUTSRVDesc, &brdfLUTSRV));

	

	D3D::GetDC()->OMSetRenderTargets(1, &brdfLUTRTV, nullptr);
	D3D::GetDC()->RSSetViewports(1, &brdfLUTviewport);
	D3D::GetDC()->ClearRenderTargetView(brdfLUTRTV, Color(0, 0, 0, 1));
	//D3D::GetDC()->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	D3D::GetDC()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	shader->DrawIndexed(0, 9, 6);

	
}

void PreFilter::CreatePreFilter(ID3D11DepthStencilView* dsv,Shader* shader)
{
	// PREFILTER ENVIRONMENT MAP
	unsigned int maxMipLevels = 5;
	D3D11_TEXTURE2D_DESC envMapDesc;
	//ZeroMemory(&skyIBLDesc, sizeof(skyIBLDesc));
	envMapDesc.Width = 256;
	envMapDesc.Height = 256;
	envMapDesc.MipLevels = maxMipLevels;
	envMapDesc.ArraySize = 6;
	envMapDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	envMapDesc.Usage = D3D11_USAGE_DEFAULT;
	envMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	envMapDesc.CPUAccessFlags = 0;
	envMapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
	envMapDesc.SampleDesc.Count = 1;
	envMapDesc.SampleDesc.Quality = 0;
	//---
	D3D11_SHADER_RESOURCE_VIEW_DESC envMapSRVDesc;
	ZeroMemory(&envMapSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	envMapSRVDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	envMapSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	envMapSRVDesc.TextureCube.MostDetailedMip = 0;
	envMapSRVDesc.TextureCube.MipLevels = maxMipLevels;
	//--
	//---
	Check(D3D::GetDevice()->CreateTexture2D(&envMapDesc, nullptr, &envMaptex));
	Check(D3D::GetDevice()->CreateShaderResourceView(envMaptex, &envMapSRVDesc, &envMapSRV));
	Vector3 direction[6] =
	{
		Vector3(1,0,0),Vector3(-1,0,0),
		Vector3(0,1,0),Vector3(0,-1,0),
		Vector3(0,0, 1),Vector3(0,0,-1)
	};


	Vector3 up[6] =
	{
		Vector3(0,1,0),Vector3(0,1,0),
		Vector3(0,0,-1),Vector3(0,0,1),
		Vector3(0,1,0),Vector3(0,1,0)
	};

	Vector3 position = Vector3(0, 0, 0);




	for (uint mip = 0; mip < maxMipLevels; mip++) {

		D3D11_RENDER_TARGET_VIEW_DESC envMapRTVDesc;
		ZeroMemory(&envMapRTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		envMapRTVDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		envMapRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		envMapRTVDesc.Texture2DArray.ArraySize = 1;
		envMapRTVDesc.Texture2DArray.MipSlice = mip;



		unsigned mipWidth = 256 * pow(0.5, mip);
		unsigned mipHeight = 256 * pow(0.5, mip);

		D3D11_VIEWPORT envMapviewport;
		envMapviewport.Width = mipWidth;
		envMapviewport.Height = mipHeight;
		envMapviewport.MinDepth = 0.0f;
		envMapviewport.MaxDepth = 1.0f;
		envMapviewport.TopLeftX = 0.0f;
		envMapviewport.TopLeftY = 0.0f;


		float roughness = (float)mip / (float)(maxMipLevels - 1);
		//float roughness = 0.0;
		for (int i = 0; i < 6; i++) {
			envMapRTVDesc.Texture2DArray.FirstArraySlice = i;
			Check(D3D::GetDevice()->CreateRenderTargetView(envMaptex, &envMapRTVDesc, &envMapRTV[i]));

			//-- Cam directions


			D3DXMatrixLookAtLH(&cubeDesc.view, &position, &direction[i], &up[i]);
			perspective = new Perspective(1280, 720, 0.1f, 100.f, Math::PI*0.5f);
			Matrix matrix;
			perspective->GetMatrix(&matrix);
			cubeDesc.projection = matrix;
			cubeDesc.roughness = roughness;

			D3D::GetDC()->OMSetRenderTargets(1, &envMapRTV[i], nullptr);
			D3D::GetDC()->RSSetViewports(1, &envMapviewport);
			D3D::GetDC()->ClearRenderTargetView(envMapRTV[i], Color(0, 0, 0, 1));
			D3D::GetDC()->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

			//---

			/*sCubeSRV = shader->AsSRV("EnvMap");
			sCubeSRV->SetResource(cubeSRV);*/
			buffer = new ConstantBuffer(&cubeDesc, sizeof(CB_CubeBuffer));
			sBuffer = shader->AsConstantBuffer("CB_CubeBuffer");

			buffer->Apply();
			sBuffer->SetConstantBuffer(buffer->Buffer());


			//D3D::GetDC()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			//shader->DrawIndexed(0, 11, 6);

			D3D::GetDC()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			shader->Draw(0, 0, 4);

		}
	}
}
