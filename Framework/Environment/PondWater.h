#pragma once
class PondWater:public Renderer
{
public:
	PondWater(Shader* shader,float height,float radius);
	~PondWater();

	void Update() override;
	void Render() override;

	void SetReflection();
	void SetRefraction();
private:
	struct Desc
	{
		Matrix ReflectionMatrix;
		Color RefrationColor = Color(0.2f, 0.3f, 1.0f, 1.0f);
		Vector2 NormalMapTile=Vector2(0.1f,0.2f);
		float WaveTranslation=0.0f;
		float WaveScale=0.03f;
		float Shininess=200.0f;
		
		float Padding[3];
	}desc;

private:
	float height, radius;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	Texture* waveMap;
	ID3DX11EffectShaderResourceVariable* sWaveMap;

	Vector2 mapSize;
	float waveSpeed;

	class Fixity* fixity;
	RenderTarget* rendertarget;
	RenderTarget*  refraction;

	DepthStencil* depthStencil;

	Viewport* viewport;

	ID3DX11EffectShaderResourceVariable* sReflectionMap;
	ID3DX11EffectVectorVariable* sClipPlane;

};

