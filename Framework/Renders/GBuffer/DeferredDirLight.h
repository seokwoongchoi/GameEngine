#pragma once

struct cascaedDesc
{

	Matrix cascadeProj[3];

};
class DeferredDirLight
{
public:
	explicit DeferredDirLight(class Shader* shader,uint width=1280,uint height=720);
	~DeferredDirLight();
	
	DeferredDirLight(const DeferredDirLight& rhs) = delete;
	DeferredDirLight& operator=(const DeferredDirLight& rhs) = delete;
	void InitShadowMap(int size);
	void Update();
	void UpdateShadowMatrix();
	void Render();
	void SetShadowMap();
	
	
	


private:

	// Extract the frustum corners for the given near and far values
	void ExtractFrustumPoints(float fNear, float fFar, D3DXVECTOR3* arrFrustumCorners);

	// Extract the frustum bounding sphere for the given near and far values
	void ExtractFrustumBoundSphere(float fNear, float fFar, D3DXVECTOR3& vBoundCenter, float& fBoundRadius);

	// Test if a cascade needs an update
	bool CascadeNeedsUpdate(const D3DXMATRIX& mShadowView, int iCascadeIdx, const D3DXVECTOR3& newCenter, D3DXVECTOR3& vOffset);
private:
	Vector3 offsetOutsave;
	ID3D11Texture2D* cascadedTexture;

	//ID3D11RenderTargetView* cascadedRTV;
	ID3D11DepthStencilView* cascadedDSV;
	ID3D11ShaderResourceView* cascadedSRV;



private:
	cascaedDesc cascaed;
	class Shader* shader;
	//class Shader* pixelCS;
	//class Shader* shadowShader;
	//class CascadedShadow* cascadedShadow;
protected:
	struct LightDesc
	{
		Color Ambient;
		Color Specular;

		Vector3 Direction;
		float Padding;

		Vector3 Position;
		float Padding1;
		

		Matrix ToShadowSpace;
		Vector4 ToCascadeSpace[3];

		
		
		
	}lightDesc;

	ConstantBuffer* LightBuffer;
	ID3DX11EffectConstantBuffer* sLightBuffer;

private:
	uint width, height;

	ID3DX11EffectMatrixVariable* sCascadedProjView;
	ID3D11SamplerState* samplerState;

private:
	

	//////////////////////////////calc matrix/////////////////////////////////////////////////////
private:

	float cascadeTotalRange;
	float arrCascadeRanges[4];

	Vector3 shadowBoundCenter;
	float shadowBoundRadius;
	Vector3 arrCascadeBoundCenter[3];
	float arrCascadeBoundRadius[3];
	Matrix worldToShadowSpace;
	

	Vector4 cascadeOffsetX;
	Vector4 cascadeOffsetY;
	Vector4 cascadeScale;

	//Vector3 vDirectionalDir;
private:
	Camera* camera;


	ID3DX11EffectShaderResourceVariable* sShadow;

	bool bFirst;
	

	Matrix view;
	Vector3 camPos;
	Vector3 camForward;
	Vector3 camUp;
	Vector3 camRight;

	Vector3 worldCenter;
	Vector3 pos	   ;
	Vector3 lookAt ;
	Vector3 right  ;
	Vector3 up;	   ;

	Matrix shadowView;
	D3DXMATRIX shadowViewInv;

	Matrix cascadeTrans;
	Matrix S;
	Vector3 newCenter;

	Vector3 offset;

	Vector3 cascadeCenterShadowSpace;

	D3D11_VIEWPORT vp[3];
	Vector3 oldCenterInCascade;
	Vector3 newCenterInCascade;
	Vector3 centerDiff;
	
	
	
	 D3DXVECTOR3 vBoundSpan;
	
 private:
	 struct CascadedShadowDesc
	 {
		 
		 Matrix arrWorldToCascadeProj[3];

	 }cascadedShadowDesc;

	 ConstantBuffer* CascadedShadowBuffer;
	 ID3DX11EffectConstantBuffer* sCascadedShadowBuffer;
};

