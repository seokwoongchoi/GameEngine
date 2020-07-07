#pragma once
//struct cascaedDesc
//{
//	
//	Matrix cascadeProj[3];
//
//};
class CascadedShadow
{
public:
	CascadedShadow(Shader* Shadowshader, uint width=1280,uint height= 720);
	~CascadedShadow();

	void Init(int shdowMapSize);
	
	void Set();
	void UpdateShadowMatrix();
	

	ID3D11ShaderResourceView* SRV() { return cascadedSRV; }
	ID3D11SamplerState* Sampler() { return samplerState; }
	void GetWorldToShadowSpace(Matrix* matrix);
	void GetToCascadeOffsetX(Vector4* vec);
	void GetToCascadeOffsetY(Vector4* vec);
	void GetToCascadeScale(Vector4* vec);
	 
private:

	// Extract the frustum corners for the given near and far values
	void ExtractFrustumPoints(float fNear, float fFar, D3DXVECTOR3* arrFrustumCorners);

	// Extract the frustum bounding sphere for the given near and far values
	void ExtractFrustumBoundSphere(float fNear, float fFar, D3DXVECTOR3& vBoundCenter, float& fBoundRadius
		, Vector3 camPos, Vector3 camRight, Vector3 camUp, Vector3 camForward);

	// Test if a cascade needs an update
	bool CascadeNeedsUpdate(const D3DXMATRIX& mShadowView, int iCascadeIdx, const D3DXVECTOR3& newCenter, D3DXVECTOR3& vOffset);
private:
	
//	cascaedDesc cascaed;
	Shader* shader;
	Shader*	deferredShader;
private:
	ID3D11Texture2D* cascadedTexture;
	
	//ID3D11RenderTargetView* cascadedRTV;
	ID3D11DepthStencilView* cascadedDSV;
	ID3D11ShaderResourceView* cascadedSRV;
private:
	uint width, height;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;
	ID3D11SamplerState* samplerState;

private:
	bool bNeedUpdate;
	
	//////////////////////////////calc matrix/////////////////////////////////////////////////////
private:
	int shadowMapSize;
	float cascadeTotalRange;
	float arrCascadeRanges[4];

	Vector3 shadowBoundCenter;
	float shadowBoundRadius;
	Vector3 arrCascadeBoundCenter[3];
	float arrCascadeBoundRadius[3];
	Matrix worldToShadowSpace;
	Matrix arrWorldToCascadeProj[3];

	Vector4 cascadeOffsetX;
	Vector4 cascadeOffsetY;
	Vector4 cascadeScale;

	Vector3 vDirectionalDir;
private:
	Camera* camera;

	

};

