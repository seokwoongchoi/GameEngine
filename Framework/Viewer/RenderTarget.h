#pragma once
class RenderTarget //
{
public:
	RenderTarget(uint width=0,uint height=0,DXGI_FORMAT format=DXGI_FORMAT_R8G8B8A8_UNORM);
	~RenderTarget();

	inline ID3D11RenderTargetView* RTV() { return rtv; }
	inline ID3D11ShaderResourceView* SRV() { return srv; }

	void SaveTexture(wstring file);
	

	void Set(ID3D11DepthStencilView* dsv);
	
	//static void Sets(vector<RenderTarget*>& rtvs, UINT count, ID3D11DepthStencilView* dsv);
	static void Sets(RenderTarget** targets, UINT count, class DepthStencil* depthStencil);
private:
	uint width;
	uint height;

	DXGI_FORMAT format;
	
	ID3D11Texture2D* backBuffer;
	ID3D11RenderTargetView* rtv;
	ID3D11ShaderResourceView* srv;
};
