#pragma once
struct VertexScale
{
	Vector3 position;
	Vector2 Scale;
	uint TextureNum;
};
class Billboard :public Renderer
{
public:
	Billboard(Shader* shader, vector<wstring> & textureNames);
	~Billboard();

	Billboard(const Billboard& rhs) = delete;
	Billboard& operator=(const Billboard& rhs) = delete;

	void Fixed(bool val) { bool fixedY = val; }


	void Add(Vector3& position, Vector2& scale, uint& TextureNum);

	void CreateBuffer();


	void Update();
	void Render();

	//Texture* GetTexture() { return texture; }
	VertexScale* GetVertex() { return vertices.data(); }


private:




	TextureArray* textures;
	
	ID3DX11EffectShaderResourceVariable* sTexture;
	

	vector< VertexScale> vertices;

};

