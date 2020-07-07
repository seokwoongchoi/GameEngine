#pragma once

class Material
{
public:
	Material();
	explicit	Material(Shader* shader);
	~Material();

	Shader* GetShader() { return shader; }
	void SetShader(Shader* shader);


	void Name(wstring val) { name = val; }
	wstring Name() { return name; }

	Color Ambient() { return colorDesc.Ambient; }
	void Ambient(Color& color);
	void Ambient(float r, float g, float b, float a = 1.0f);

	Color Diffuse() { return colorDesc.Diffuse; }
	void Diffuse(Color& color);
	void Diffuse(float r, float g, float b, float a = 1.0f);

	Color Specular() { return colorDesc.Specular; }
	void Specular(Color& color);
	void Specular(float r, float g, float b, float a = 1.0f);



	float Roughness() { return colorDesc.Roughness; }
	void Roughness(float val);

	float Matallic() { return colorDesc.Matallic; }
	void Matallic(float val);


	Texture* DiffuseMap() { return diffuseMap; }
	void DiffuseMap(string file);
	void DiffuseMap(wstring file);

	void DiffuseMaps(wstring file);

	Texture* SpecularMap() { return specularMap; }
	void SpecularMap(string file);
	void SpecularMap(wstring file);

	void SpecularMaps(wstring file);

	Texture* NormalMap() { return normalMap; }
	void NormalMap(string file);
	void NormalMap(wstring file);

	void NormalMaps(wstring file);

	Texture* RoughnessMap() { return roughnessMap; }
	void RoughnessMap(string file);
	void RoughnessMap(wstring file);

	void RoughnessMaps(wstring file);

	Texture* MatallicMap() { return matallicMap; }
	void MatallicMap(string file);
	void MatallicMap(wstring file);

	void MatallicMaps(wstring file);

	void Render();

	void Renders();

private:
	void Initialize();

private:
	struct ColorDesc
	{
		Color Ambient = Color(1, 1,1, 1);
		Color Diffuse = Color(1, 1, 1, 1);
		Color Specular = Color(0, 0, 0, 1);

	    float Roughness = 1;
		float Matallic = 1;
		float Padding[2];
	} colorDesc;

private:
	Shader* shader;

	wstring name;

	Texture* diffuseMap;
	
	Texture* specularMap;
	
	Texture* normalMap;
	
	Texture* roughnessMap;
	
	Texture* matallicMap;

	TextureArray* diffuseMaps;
	vector<wstring> diffuseNames;

	TextureArray* specularMaps;
	vector<wstring> specularNames;

	TextureArray* normalMaps;
	vector<wstring> normalNames;

	TextureArray* roughnessMaps;
	vector<wstring> roughnessNames;

	TextureArray* matallicMaps;
	vector<wstring> matallicNames;

	ConstantBuffer* buffer;

	ID3DX11EffectConstantBuffer* sBuffer;
	ID3DX11EffectShaderResourceVariable* sDiffuseMap;
	ID3DX11EffectShaderResourceVariable* sSpecularMap;
	ID3DX11EffectShaderResourceVariable* sNormalMap;
	ID3DX11EffectShaderResourceVariable* sRoughnessMap;
	ID3DX11EffectShaderResourceVariable* sMatallicMap;

	ID3DX11EffectShaderResourceVariable* sDiffuseMaps;
	ID3DX11EffectShaderResourceVariable* sSpecularMaps;
	ID3DX11EffectShaderResourceVariable* sNormalMaps;
	ID3DX11EffectShaderResourceVariable* sRoughnessMaps;
	ID3DX11EffectShaderResourceVariable* sMatallicMaps;


	bool isDiffuseMap = false;
	bool isSpecularMap = false;
	bool isNormalMap = false;
	bool isRoughnessMap = false;
	bool isMatallicMap = false;
};
