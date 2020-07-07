#pragma once

class Cloud :public Renderer
{
public:
	explicit Cloud(Shader* shader);
	~Cloud();

	
	Cloud(const Cloud&) = delete;
	Cloud& operator=(const Cloud&) = delete;

	void Pass(uint i) {
		this->pass
			= i;
	}
	void Render(bool bGlow = false);
	void Update();
	void Create();
	void ImGui();
private:

	Vector3 position = Vector3(0, 0, 0);
	uint skyPlaneResolution = 10;
	uint textureRepeat = 1;
	float skyPlaneWidth = 10.0f;
	float skyPlaneTop = 1.5f;
	float skyPlaneBottom = 0.0f;
	Shader* shader;

	//VertexBuffer * vertexBuffer;

	ID3D11Texture2D* texture;


	Texture* cloudTexture1;
	Texture* cloudTexture2;
	//ID3D11Texture2D* cloudTexture1;
	//ID3D11Texture2D* cloudTexture2;

	ID3D11ShaderResourceView* srv;

	ID3DX11EffectShaderResourceVariable* sSRV;

	ID3DX11EffectShaderResourceVariable* sCloud1;
	ID3DX11EffectShaderResourceVariable* sCloud2;

	VertexBuffer* cloudVertexBuffer;
	IndexBuffer* cloudIndexBuffer;
	UINT cloudVertexCount;
	UINT cloudIndexCount;

	uint pass;

	struct CloudDesc
	{
		float Tiles = 3.323f;
		float Cover = 0.00f;
		float Sharpness = 1.214f;
		float Speed = 0.01f;
		Vector2 FirstOffset = Vector2(0.0f, 0.0f);
		Vector2 SecondOffset = Vector2(0.5f, 0.2f);
	} cloudDesc;
	ConstantBuffer* cloudBuffer;
	ID3DX11EffectConstantBuffer* sCloudBuffer;
	float scale = 150.0f;
};