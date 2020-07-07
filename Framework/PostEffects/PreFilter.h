#pragma once
class PreFilter
{
public:
	PreFilter();
	~PreFilter();

	void CreateBRDF(ID3D11DepthStencilView* dsv);
	void CreatePreFilter(ID3D11DepthStencilView* dsv,Shader* shader);
	ID3D11ShaderResourceView* BRDFSRV() {return brdfLUTSRV;}
	ID3D11ShaderResourceView* PreFilterSRV() {return envMapSRV;}
	ID3D11ShaderResourceView* SRV() { return cubeSRV; }
private:
	Perspective* perspective;
private:
	struct CB_CubeBuffer
	{
		Matrix view;
		Matrix projection;
		float roughness;
		float Padding[3];
	};
	CB_CubeBuffer cubeDesc;
private:
	
	ID3D11Texture2D* envMaptex;
	ID3D11RenderTargetView* envMapRTV[6];
	ID3D11ShaderResourceView* envMapSRV;

	ID3D11Texture2D* brdfLUTtex;
	ID3D11RenderTargetView* brdfLUTRTV;
	ID3D11ShaderResourceView* brdfLUTSRV;

	ID3D11Texture2D* dsvTexture;
	ID3D11DepthStencilView* dsv;
private:
	class Shader* shader;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	ID3D11ShaderResourceView* cubeSRV;
	ID3DX11EffectShaderResourceVariable* sCubeSRV;
};
