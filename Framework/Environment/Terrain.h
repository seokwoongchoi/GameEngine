#pragma once
enum class BrushType
{
	None,
	Raise,
	Smooth,
	Splatting,
};
class Terrain :public Renderer
{
public:
	typedef VertexColorTextureNormal TerrainVertex;

public:

	Terrain(Shader* shader,wstring heightMap);
	 ~Terrain();

	 void Update();
	 void Render();

	 TerrainVertex* GetVertices() { return vertices; }
	 uint* GetIndices() { return indices; }

	 Vector3* GetPoints() { return points.data(); }
	 uint GetPointsCount() { return points.size(); }

	 void BaseMap(wstring file);
	 void LayerMap(wstring layer, wstring alpha);
	 void LayerMap1(wstring layer);
	 void LayerMap2(wstring layer);
	 void LayerMap3(wstring layer);
	 void LayerMap4(wstring layer);
	 float GetHeight(Vector3& position, float terrainSpacing = 1.0f);
	 Vector3 GetTerrainNormal(Vector3& position,Vector3& org);

	 Vector3 GetPickedPosition();

	 void SavePixel();
	 void SaveMap(wstring name);
	 void OnenFile(wstring name,uint layerNum);
	

	 void SplattingDetail(const char* name, const char* InputName,const string& layerName,Texture* layerMap,uint num);
	
	 
public:
	void RaiseHeight(Vector3& position,uint type,uint range );
	void SmoothBrush(Vector3& position, uint type, uint range,float factor);
	void Platting(Vector3& position, uint type, uint range);
	void Splatting(Vector3& position, uint type, uint range,uint alpha);
private:
	void CreateVertexData();
	void CreateIndexData();
	void CreateNormalData();

	void CreateTestTexture();

	void ReadHeightMap(vector<Color>* colors);
private:
	struct BrushDesc
	{
		Color Color=D3DXCOLOR(0,1,0,1);
		Vector3 Location;
		uint Type = 0; //그리지 않음
		uint Range = 10;
		float Padding[3];

	}brushDesc;

	struct GridDesc
	{
		
		Color GridLineColor = D3DXCOLOR(1, 1, 1, 1);;

		uint VisibleGridLine; //0 off, 1 on
		float GridLineThickness=-0.5f;
		float GridLineSize=1;
		uint LayerNum=0;

	}gridDesc;


private:
	
	BrushType brushtype;
	float terrainSpacing;
	

	uint width, height;
	Vector2 spacing; //texture의 간격
	
	Texture* heightMap;

	
	TerrainVertex* vertices;
	

	
	uint* indices;
	
	vector<D3DXVECTOR3> points;
	D3DXVECTOR3 normalize;

	bool bCheck;
	bool bGrid;

	Texture* baseMap;

	ID3DX11EffectShaderResourceVariable* SBaseMap;

	Texture* layerMap;
	ID3DX11EffectShaderResourceVariable* SLayerMap;
	Texture* layerMap2;
	ID3DX11EffectShaderResourceVariable* SLayer2Map;
	Texture* layerMap3;
	ID3DX11EffectShaderResourceVariable* SLayer3Map;
	Texture* layerMap4;
	ID3DX11EffectShaderResourceVariable* SLayer4Map;

	Texture* alphaMap;
	ID3DX11EffectShaderResourceVariable* SAlphaMap;

	//normal 용
	Vertex* nv;
	ID3D11Buffer* nvertexBuffer;
	vector<Vector3> normals;
	class Shader* nShader;

	//constantbuffer

	ConstantBuffer* brushBuffer;
	ID3DX11EffectConstantBuffer* sBrushBuffer;
	ConstantBuffer* gridBuffer;
	ID3DX11EffectConstantBuffer* sGridBuffer;
	
	
	
	D3D11_TEXTURE2D_DESC temp;

	uint alpha;

	bool bUpdate;

	string layer1name;
	string layer2name;
	string layer3name;
	string layer4name;

	vector<function<void(wstring, uint)>> funcs;

	bool bLayer1;
	bool bLayer2;
	bool bLayer3;
	bool bLayer4;
	Vector3 mouse;
};

