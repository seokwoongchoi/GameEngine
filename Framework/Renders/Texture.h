#pragma once

class Texture
{
public:
	friend class Textures;
	Texture(const Texture& rhs) = delete;
	Texture& operator=(const Texture& rhs) = delete;
public:
	void SaveFile(wstring file);
	static void SaveFile(wstring file, ID3D11Texture2D* src);
	static D3D11_TEXTURE2D_DESC ReadPixels(ID3D11Texture2D* src, DXGI_FORMAT readFormat, vector<D3DXCOLOR>* pixels);

	D3D11_TEXTURE2D_DESC ReadPixel(DXGI_FORMAT format, vector<Color>*pixels);
	static D3D11_TEXTURE2D_DESC ReadPixel(ID3D11Texture2D* src,DXGI_FORMAT format, vector<Color>*pixels);
public:
	explicit Texture(wstring file, D3DX11_IMAGE_LOAD_INFO* loadInfo = NULL);

	~Texture();

	operator ID3D11ShaderResourceView*(){ return view; }


	wstring GetFile() { return file; }

	UINT GetWidth() { return metaData.width; }
	UINT GetHeight() { return metaData.height; }
	

	D3D11_TEXTURE2D_DESC ReadPixels(DXGI_FORMAT readFormat, vector<D3DXCOLOR>* pixels);
	

	void GetImageInfo(DirectX::TexMetadata* data)
	{
		*data = metaData;
	}

	inline ID3D11ShaderResourceView* SRV() { return view; }
	ID3D11Texture2D* GetTexture();

private:
	wstring file;

	DirectX::TexMetadata metaData;
	ID3D11ShaderResourceView* view;
	
};

struct TextureDesc
{
	wstring file;
	UINT width, height;
	DirectX::TexMetadata metaData;
	ID3D11ShaderResourceView* view;

	bool operator==(const TextureDesc& desc)
	{
		bool b = true;
		b &= file == desc.file;
		b &= width == desc.width;
		b &= height == desc.height;

		return b;
	}
};

class Textures
{
public:
	friend class Texture;

public:
	static void Create();
	static void Delete();

private:
	static void Load(Texture* texture, D3DX11_IMAGE_LOAD_INFO* loadInfo = NULL);

private:
	static vector<TextureDesc> descs;
};

class TextureArray
{
public:
	explicit	TextureArray(vector<wstring>& names, UINT width = 256, UINT height = 256, UINT mipLevels = 1);
	~TextureArray();
	TextureArray(const TextureArray& rhs) = delete;
	TextureArray& operator=(const TextureArray& rhs) = delete;
	ID3D11ShaderResourceView* SRV() { return srv; }

private:
	vector<ID3D11Texture2D *> CreateTextures(vector<wstring>& names, UINT width, UINT height, UINT mipLevels);

private:
	ID3D11ShaderResourceView* srv;
};

class TextureCube
{
public:
	TextureCube(Vector3& position, UINT width, UINT height);
	~TextureCube();

	
	void Set(Shader* shader);

	inline ID3D11ShaderResourceView* SRV() { return srv; }
	//Camera* GetCamera(UINT index) { return (Camera *)cameras[index]; }

	Perspective* GetPerspective() { return perspective; }

private:
	Vector3 position;

	UINT width, height;

	ID3D11Texture2D* rtvTexture;
	ID3D11RenderTargetView* rtv;
	ID3D11ShaderResourceView* srv;

	ID3D11Texture2D* dsvTexture;
	ID3D11DepthStencilView* dsv;

	Matrix view[6];
	//class Fixity* cameras[6];

	Perspective* perspective;
	Viewport* viewport;

	ID3DX11EffectMatrixVariable* sView;
	ID3DX11EffectMatrixVariable* sProjection;

private:
	struct CubeMapViewDesc
	{
		Matrix CubeViews[6];
		Matrix CubeProjection;
	};

	CubeMapViewDesc cubeMapViewDesc;

	ConstantBuffer* cubeMapBuffer = nullptr;
	ID3DX11EffectConstantBuffer* sCubeMapBuffer=nullptr;

	Vector3 up[6] =
	{
		Vector3(0,1,0),Vector3(0,1,0),
		Vector3(0,0,-1),Vector3(0,0,1),
		Vector3(0,1,0),Vector3(0,1,0)
	};
	
};