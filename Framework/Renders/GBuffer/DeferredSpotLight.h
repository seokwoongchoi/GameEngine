#pragma once

struct SpotLight
{
	
	Color Diffuse;
	Color Specular;

	Vector3 Position;
	Vector3 Direction;

	float Range;

	float Intensity;

	float InnerAngle;
	float OuterAngle;

	
	
//////////////////////////////////////////////////

};
struct DomainDesc
{
	
	Matrix WolrdViewProj;
	float fSinAngle;
	float fCosAngle;

	float Pading1;
	float Pading2;
	
};
struct PixelDesc
{
	Color Diffuse;
	Color Specular;

	Vector3 Position;
	float Range;

	Vector3 vDirToLight;//-dir
	float Intensity;

	float SpotCosConeAttRange;
	float SpotCosOuterCone;
	float pading1;
	float pading2;
	
};

class DeferredSpotLight
{
public:
	explicit DeferredSpotLight(class Shader* shader);
	~DeferredSpotLight();

	
		DeferredSpotLight(const DeferredSpotLight&) = delete;
	DeferredSpotLight& operator=(const DeferredSpotLight&) = delete;
	
	void Render();
	

	
	void AddSpotLight(SpotLight& light);
	SpotLight& GetSpotLight(UINT index);
	uint GetSpotLightCount() { return spotLights.size(); }
private:

	Matrix View;
	Matrix Proj;
	Matrix lightScaleMatrix;
	Matrix lightTransMatrix;
	//uint pointLightCount;

	Shader* shader;

	//SpotLightDesc spotLightDesc;
	vector< SpotLight> spotLights;
	DomainDesc domainDesc;
	PixelDesc pixelDesc;

	UINT spotLightCount;

private:

	ID3D11DepthStencilState* m_pNoDepthWriteGreatherStencilMaskState;
	ConstantBuffer* DomainBuffer;
	ID3DX11EffectConstantBuffer* sDomainBuffer;

	ConstantBuffer* pixelBuffer;
	ID3DX11EffectConstantBuffer* sPixelLightBuffer;

	D3DXMATRIX m_LightWorldTransRotate;
	D3DXMATRIX mWorldViewProjection;
	D3DXVECTOR3 vUp;
	D3DXVECTOR3 vRight;
	D3DXVECTOR3 vAt;
	Vector3 vDir;
};

