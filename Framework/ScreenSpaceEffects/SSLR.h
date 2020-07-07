#pragma once
class SSLR
{
public:
	SSLR(uint widht=1280,uint height=720);
	~SSLR();
	// Render the screen space light rays on top of the scene
	void Render(ID3D11ShaderResourceView* ssaoSRV, ID3D11RenderTargetView* pLightAccumRTV);
	void ImGui();
private:

	// Prepare the occlusion map
	void PrepareOcclusion(ID3D11ShaderResourceView* ssaoSRV);

	// Ray trace the occlusion map to generate the rays
	void RayTrace( const D3DXVECTOR2& vSunPosSS, const D3DXVECTOR3& vSunColor);

	// Combine the rays with the scene
	void Combine( ID3D11RenderTargetView* pLightAccumRTV);
	
private:
	// Shaders
	ID3D11Buffer*  OcclusionCB;
	ID3D11ComputeShader*  OcclusionCS;
	ID3D11Buffer*  RayTraceCB;
	ID3D11VertexShader*  FullScreenVS;
	ID3D11PixelShader*  RayTracePS;
	ID3D11PixelShader*  CombinePS;


	uint downScaleGroups;
private:

	struct CB_OCCLUSSION
	{
		UINT nWidth;
		UINT nHeight;
		UINT pad[2];
	};
	
	struct CB_LIGHT_RAYS
	{
		D3DXVECTOR2 vSunPos;
		float fInitDecay;
		float fDistDecay;
		D3DXVECTOR3 vRayColor;
		float fMaxDeltaLen;
	};
	
private:
	bool ShowRayTraceRes=false;
	uint width;
	uint height;

	ID3D11Texture2D* OcclusionTex;
	ID3D11UnorderedAccessView* OcclusionUAV; 
	ID3D11ShaderResourceView* OcclusionSRV;

	ID3D11Texture2D* LightRaysTex;
	ID3D11RenderTargetView* LightRaysRTV;
	ID3D11ShaderResourceView* LightRaysSRV;
private:

//private:
//	ID3DX11EffectShaderResourceVariable* sDepthMap;
//	ID3DX11EffectShaderResourceVariable* sNormalMap;
//	
	ID3D11BlendState* AdditiveBlendState;
	

	Vector3 dir;
	Vector3 vSunPos;
	Vector3 vEyePos;
	Matrix mView;
	Matrix mProj;
	Matrix mViewProjection;
	Vector3 vSunPosSS;
	float fMaxSunDist = 5.641f;
	float intensity = 0.023f;
	float decay = 0.2f;
	//float ddecay = 0.8f;
	float ddecay = 0.3f;

 float dist = 1000.0f;
	float fMaxDeltaLen = 0.005f;

	Vector3 white = Vector3(1, 1, 1);
 Vector3 yellow = Vector3(0.081f, 0.043f, 0.017f);
	D3DXVECTOR3 vSunColorAtt = Vector3(0.1f, 0.087f, 0.06f);

	D3D11_VIEWPORT oldvp;
	UINT num = 1;

	Vector3 sunColor = Vector3(0.1f, 0.087f, 0.06f);
	float factor;
};

