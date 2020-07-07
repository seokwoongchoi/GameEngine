#include "Framework.h"
#include "RenderTarget.h"

RenderTarget::RenderTarget(uint width, uint height, DXGI_FORMAT format)
	:format(format),srv(nullptr)
{
	this->width = (width < 1) ? (uint)D3D::Width() : width;
	this->height = (height < 1) ? (uint)D3D::Height() : height;
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = this->width;
	textureDesc.Height = this->height;
	textureDesc.Format = format;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;

	D3D::GetDevice()->CreateTexture2D(&textureDesc, nullptr, &backBuffer);

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	D3D::GetDevice()->CreateRenderTargetView(backBuffer,&rtvDesc,&rtv);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	
	D3D::GetDevice()->CreateShaderResourceView(backBuffer,&srvDesc,&srv);
	
	
}

RenderTarget::~RenderTarget()
{
	SafeRelease(backBuffer);
	SafeRelease(rtv);
	SafeRelease(srv);

}

void RenderTarget::SaveTexture(wstring file)
{
	Check(D3DX11SaveTextureToFile(D3D::GetDC(), backBuffer, D3DX11_IFF_PNG, file.c_str()));
	
}

void RenderTarget::Set(ID3D11DepthStencilView * dsv)
{
	D3D::Get()->SetRenderTarget(rtv, dsv);
	D3D::Get()->Clear(Color(0, 0, 0, 1), rtv, dsv);
}



//void RenderTarget::Sets(vector<RenderTarget*>& rtvs, UINT count, ID3D11DepthStencilView * dsv)
//{
//	vector< ID3D11RenderTargetView*> views;
//	for (uint i = 0; i < count; i++)
//	{
//		views.emplace_back(rtvs[i]->RTV());
//
//	}
//	D3D::GetDC()->OMSetRenderTargets(views.size(), &views[0], dsv);
//	for (uint i = 0; i < views.size(); i++)
//		D3D::Get()->Clear(Color(0, 0, 0, 1),views[i],dsv);
//}

void RenderTarget::Sets(RenderTarget ** targets, UINT count, DepthStencil * depthStencil)
{
	vector<ID3D11RenderTargetView *> rtvs;
	for (UINT i = 0; i < count; i++)
	{
		ID3D11RenderTargetView* rtv = targets[i]->RTV();
		rtvs.emplace_back(rtv);
		D3D::GetDC()->ClearRenderTargetView(rtv, Color(0, 0, 0, 1));
	}

	D3D::GetDC()->ClearDepthStencilView(depthStencil->DSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	D3D::GetDC()->OMSetRenderTargets(rtvs.size(), &rtvs[0], depthStencil->DSV());
}


