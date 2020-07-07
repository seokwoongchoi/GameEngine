#pragma once
class HDR
{
public:
	HDR(uint width = 1280, uint height = 720);
	~HDR();

	void Pass(UINT val) { pass = val; }
	void PostProcessing(ID3D11ShaderResourceView* pHDRSRV, ID3D11RenderTargetView * oldTarget, ID3D11ShaderResourceView* dsv);
	void DownScale(ID3D11ShaderResourceView* srv);
	
	void DownScaleBlur();
	void Bloom();
	void ImGui();
	
	void BokehHightlightScan(ID3D11ShaderResourceView* pHDRSRV, ID3D11ShaderResourceView* pDepthSRV);

	void Blur(ID3D11ShaderResourceView* pInput, ID3D11UnorderedAccessView* pOutput);

	void FinalPass(ID3D11ShaderResourceView* srv, ID3D11ShaderResourceView* dsv);
	// Entry point for post processing

	void BokehRender();

	void SetParameters(float fMiddleGrey, float fWhite)
	{
		this->fMiddleGrey = fMiddleGrey;
		this->fWhite = fWhite;
	}
private:


	ID3D11Buffer* downScale1DBuffer;
	ID3D11UnorderedAccessView* downScale1DUAV;
	ID3D11ShaderResourceView* downScale1DSRV;


	ID3D11Texture2D* m_pDownScaleRT;
	ID3D11ShaderResourceView* m_pDownScaleSRV;
	ID3D11UnorderedAccessView* m_pDownScaleUAV;
		

	// Average luminance
	ID3D11Buffer* avgLumBuffer;
	ID3D11UnorderedAccessView* avgLumUAV;
	ID3D11ShaderResourceView* avgLumSRV;
	
	// Previous average luminance for adaptation
	ID3D11Buffer* m_pPrevAvgLumBuffer;
	ID3D11UnorderedAccessView* m_pPrevAvgLumUAV;
	ID3D11ShaderResourceView* m_pPrevAvgLumSRV;


	ID3D11DeviceContext* DC;
private:

	uint pass;
	uint width;
	uint height;

	uint downScaleGroups;

	float fMiddleGrey;
	float fWhite;
	float m_fAdaptation;
	float m_fBloomThreshold;
	float m_fBloomScale;
private:
	struct TDownScaleCB
	{
		UINT nWidth;
		UINT nHeight;
		UINT nTotalPixels;
		UINT nGroupSize;
		float fAdaptation;
		float fBloomThreshold;
		UINT pad[2];
	};


	ID3D11Buffer* m_pDownScaleCB;


	struct TFinalPassCB
	{

		float fMiddleGrey;
		float fLumWhiteSqr;
		float fBloomScale;
		UINT pad;
	};
	
	ID3D11Buffer* m_pFinalPassCB;

	 float DOFFarValues1 = 0.0f;
	 float DOFFarValues2 =  943.0f;

	 float m_fBokehLumThreshold= 7.65f;
	 float m_fBokehBlurThreshold = 0.43f;
	 float m_fBokehRadiusScale = 0.05;
	 float m_fBokehColorScale = 0.05f;

	
	 ID3D11SamplerState*         g_pSampLinear = NULL;
	 ID3D11SamplerState*         g_pSampPoint = NULL;
	// Bloom texture
	ID3D11Texture2D* m_pBloomRT;
	ID3D11ShaderResourceView* m_pBloomSRV;
	ID3D11UnorderedAccessView* m_pBloomUAV;
	
	// Temporary texture
	ID3D11Texture2D* m_pTempRT[2];
	ID3D11ShaderResourceView* m_pTempSRV[2];
	ID3D11UnorderedAccessView* m_pTempUAV[2];
private:

	ID3D11Buffer* m_pBokehBuffer;
	ID3D11UnorderedAccessView* m_pBokehUAV=nullptr;
	ID3D11ShaderResourceView* m_pBokehSRV;
	


	// Bokeh indirect draw buffer
	ID3D11Buffer* m_pBokehIndirectDrawBuffer=nullptr;

	// Bokeh highlight texture view and blend state
	ID3D11ShaderResourceView* m_pBokehTexView;
	ID3DX11EffectUnorderedAccessViewVariable* bokehUAVEffect;
	ID3DX11EffectShaderResourceVariable* bokehHDRTextureEffect;
	ID3DX11EffectShaderResourceVariable* bokehDepthTextureEffect;
	ID3DX11EffectShaderResourceVariable* bokehAvgLumEffect;

	struct TBokehHightlightScanCB
	{
		UINT nWidth;
		UINT nHeight;
		float ProjectionValues[2];
		float fDOFFarStart;

		float fDOFFarRangeRcp;
		float fMiddleGrey;
		float fLumWhiteSqr;
		float fBokehBlurThreshold;

		float fBokehLumThreshold;
		float fRadiusScale;
		float fColorScale;
		

	} ;
	TBokehHightlightScanCB BokehHightlightScanCB;
	ConstantBuffer* bokehCSBuffer;
	ID3DX11EffectConstantBuffer* sBokehCSBuffer;

	struct TBokehRenderCB
	{
		float AspectRatio[2];
		UINT pad[2];


	};
	TBokehRenderCB BokehRenderCB;
	ConstantBuffer* bokehRenderBuffer;
	ID3DX11EffectConstantBuffer* sBokehRenderBuffer;

	Matrix proj;
	bool bFirst = false;
	


	// Shaders
	ID3D11ComputeShader* m_pDownScaleFirstPassCS;
	ID3D11ComputeShader* m_pDownScaleSecondPassCS;
	ID3D11ComputeShader* m_pBloomRevealCS;
	ID3D11ComputeShader* m_HorizontalBlurCS;
	ID3D11ComputeShader* m_VerticalBlurCS;
	ID3D11VertexShader* m_pFullScreenQuadVS;
	ID3D11PixelShader* m_pFinalPassPS;
	
};
