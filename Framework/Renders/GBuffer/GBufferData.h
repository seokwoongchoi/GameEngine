#pragma once
struct CB_GBufferUnpack
{
	Vector4 PerspectiveValuse;
	Matrix ViewInv;
	
	
};
class GBufferData
{
public:
	GBufferData() {}
	~GBufferData() {}

	void Init(class Shader* shader, uint width = 1280, uint height = 720);
	void Destroy();
	void Update();
	void Render(ID3D11ShaderResourceView* ssaoSRV);
	
	/*operator ID3D11ShaderResourceView*() { return SRV(); }
	operator ID3D11RenderTargetView*() { return RTV(); }
	operator ID3D11Texture2D*() { return Texture(); }*/
public:
	inline ID3D11RenderTargetView* DiffuseRTV()
	{
		return diffuseRTV;
	}

	inline ID3D11RenderTargetView* SpecularRTV()
	{
		return specularRTV;
	}

	inline ID3D11RenderTargetView* NormalRTV()
	{
		return normalRTV;
	}

	inline ID3D11DepthStencilView* DepthstencilDSV()
	{
		return depthStencilDSV;
	}

	inline ID3D11DepthStencilView* DepthstencilDSVReadOnly()
	{
		return depthStencilReadOnlyDSV;
	}


public:

	inline ID3D11ShaderResourceView* NormalSRV()
	{
		return normalSRV;
	}

	inline ID3D11ShaderResourceView* DepthstencilSRV()
	{
		return depthStencilSRV;
	}

public:
	inline ID3D11Texture2D* DiffuseTexture()
	{
		return diffuseTexture;
	}

private:
	void CreateViews();
	// GBuffer textures
	ID3D11Texture2D* depthStencilTexture;
	ID3D11Texture2D* diffuseTexture;
	ID3D11Texture2D* normalTexture;
	ID3D11Texture2D* specularTexture;

	// GBuffer render views
	ID3D11DepthStencilView* depthStencilDSV;
	ID3D11DepthStencilView* depthStencilReadOnlyDSV;
	ID3D11RenderTargetView* diffuseRTV;
	ID3D11RenderTargetView* normalRTV;
	ID3D11RenderTargetView* specularRTV;

	// GBuffer shader resource views
	ID3D11ShaderResourceView* depthStencilSRV;
	ID3D11ShaderResourceView* diffuseSRV;
	ID3D11ShaderResourceView* normalSRV;
	ID3D11ShaderResourceView* specularSRV;

	ID3D11DepthStencilState * depthStencilState;
protected:
	class Shader* shader;
	struct CB_GBufferUnpack unpack;
	ConstantBuffer* unpackBuffer;
	ID3DX11EffectConstantBuffer* sUnpackBuffer;

	
	ID3DX11EffectShaderResourceVariable* sDiffuse;
	ID3DX11EffectShaderResourceVariable* sNormal;
	ID3DX11EffectShaderResourceVariable* sSpecular;
	
	ID3DX11EffectShaderResourceVariable* sDepth;
	ID3DX11EffectShaderResourceVariable* sSsao;
	
private:
	Matrix proj;
	Matrix view;

	uint width;
	uint height;
};

