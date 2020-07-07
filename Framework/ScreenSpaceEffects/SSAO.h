#pragma once
class SSAO
{
public:
	SSAO(uint width=1280.0f,uint height=720.0f);
	~SSAO();
	SSAO(const SSAO& rhs) = delete;
	SSAO& operator=(const SSAO& rhs) = delete;
	
	void Compute(ID3D11ShaderResourceView* DepthSRV, ID3D11ShaderResourceView* NormalsSRV);
	
	inline ID3D11ShaderResourceView* GetSSAOSRV() { return m_pSSAO_SRV; }
	inline ID3D11ShaderResourceView* GetMiniDepthSRV() { return m_pSSAO_SRV; }
private:

	void DownscaleDepth(ID3D11DeviceContext* DC,ID3D11ShaderResourceView* DepthSRV, ID3D11ShaderResourceView* NormalsSRV);
	
	void ComputeSSAO(ID3D11DeviceContext* DC);

	
private:
	uint downScaleGroups;
	UINT width;
	UINT height;
	float SSAOSampRadius;
	float Radius;

private:
	struct DownscaleCB
	{
		UINT nWidth;
		UINT nHeight;
		float fHorResRcp;
		float fVerResRcp;
		D3DXVECTOR4 ProjParams;
		D3DXMATRIX ViewMatrix;
		float fOffsetRadius;
		float fRadius;
		float fMaxDepth;
		UINT pad;
	};
	ID3D11Buffer* m_pDownscaleCB;

private:
	
	// SSAO values for usage with the directional light
	ID3D11Texture2D* m_pSSAO_RT;
	ID3D11UnorderedAccessView* m_pSSAO_UAV;
	ID3D11ShaderResourceView* m_pSSAO_SRV;

	// Downscaled depth buffer (1/4 size)
	ID3D11Buffer* m_pMiniDepthBuffer;
	ID3D11UnorderedAccessView* m_pMiniDepthUAV;
	ID3D11ShaderResourceView* m_pMiniDepthSRV;
	//StructuredBuffer* depthBuffer;
	//StructuredBuffer* ssaoBuffer;
		// Shaders
	ID3D11ComputeShader* m_pDepthDownscaleCS;
	ID3D11ComputeShader* m_pComputeCS;

	Matrix proj;
};
