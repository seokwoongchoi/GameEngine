#pragma once
class SSReflection
{
public:
	SSReflection(class Shader* deferredShader,uint width,uint height);
	~SSReflection();

	void PreRenderRelection(ID3D11ShaderResourceView* pDiffuseSRV, ID3D11DepthStencilView* ptDepthReadOnlyDSV);
	// Do the reflections blend with light accumulation
	void DoReflectionBlend();

	ID3D11ShaderResourceView* GetSRV() {return m_ReflectSRV;}
private:
	class Shader* shader;
	class Shader* deferredShader;
private:
	float ViewAngleThreshold;
	float EdgeDistThreshold;
	float DepthBias;
	float ReflectionScale;
private:
	struct SSReflectionCB
	{
		float ViewAngleThreshold;
		float EdgeDistThreshold;
		float DepthBias;
		float ReflectionScale;
	};

	SSReflectionCB reflectionCB;

	ConstantBuffer* reflectionBuffer;
	ID3DX11EffectConstantBuffer* sReflectionBuffer;
private:
	// Reflection light accumulation buffer`
	ID3D11Texture2D* m_pReflectTexture;
	ID3D11RenderTargetView* m_ReflectRTV;
	ID3D11ShaderResourceView* m_ReflectSRV;

};

