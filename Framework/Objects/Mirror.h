#pragma once
class Mirror :public Renderer
{
public:
	Mirror(Shader* shader, float height, float radius);
	~Mirror();

	void Update() override;
	void Render() override;

	void SetReflection();
	void SetRefraction();
private:
	struct Desc
	{
		Matrix ReflectionMatrix;
		Color RefrationColor = Color(0.2f, 0.3f, 1.0f, 1.0f);
		
		float Shininess = 200.0f;
		float Padding[3];
	}desc;

private:
	float height, radius;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	
	Vector2 mapSize;
	

	class Fixity* fixity;
	RenderTarget* rendertarget;
	

	DepthStencil* depthStencil;

	Viewport* viewport;

	ID3DX11EffectShaderResourceVariable* sReflectionMap;
	

};

