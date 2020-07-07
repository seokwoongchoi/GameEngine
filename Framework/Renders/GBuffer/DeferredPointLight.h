#pragma once

struct PointLight
{
	
	Color Diffuse;
	Color Specular;

	Vector3 Position;
	float Range;

	float Intensity;
	Vector3 Projection;

	
	
	
};
struct PointLightDesc
{
   Matrix WolrdViewProj;
   PointLight Light;
};
class DeferredPointLight
{
public:
	explicit DeferredPointLight(class Shader* shader);
	~DeferredPointLight();

	
		DeferredPointLight(const DeferredPointLight&) = delete;
	DeferredPointLight& operator=(const DeferredPointLight&) = delete;
	void Render();
	//void Pass(uint pass) { this->pass = pass; }
	PointLight& GetPointLight(UINT index);
	uint GetPointLightCount() { return pointLights.size(); }
	//uint PointLights(OUT PointLight* lights);
	void AddPointLight(PointLight& light);
	//PointLight& GetPointLight(uint index);
	//uint GetPointLightCount() { return pointLightCount; }

	
private:
	
	Matrix View;
	Matrix Proj;
	Matrix lightScaleMatrix;
	Matrix lightTransMatrix;
	//uint pointLightCount;

	Shader* shader;

	PointLightDesc pointLightDesc;
	//vector< PointLight> pointLights;
	vector< PointLight> pointLights;
private:
	

	ConstantBuffer* pointLightBuffer;
	ID3DX11EffectConstantBuffer* sPointLightBuffer;

	
};

