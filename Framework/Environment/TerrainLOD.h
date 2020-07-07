#pragma once
#define BillboardTextureCount 8
enum class BrushType
{
	None,
	Raise,
	Smooth,
	Splatting,
	Billboard,
};
struct QuadTreeNode
{
	float minX;
	float maxX;
	float minZ;
	float maxZ;

	Vector3 boundsMin;
	Vector3 boundsMax;

	Vector2 minMaxY;
	vector< QuadTreeNode*>childs;
	vector<QuadTreeNode*> hittedChilds;

	bool Intersection(const Vector3& org, const Vector3& dir,  Vector3& Pos);

	
	bool IntersectionAABB(const Vector3& org, const Vector3& dir,  Vector3& Pos,float& d);
	bool hitted=false;

	float dist=-1.0f;



};



struct VertexTerrain
{
	Vector3 Position;
	Vector2 Uv;
	Vector2 BoundsY;
	Color Alpha;
};
struct BufferDesc
{
	float MinDistance = 1.0f;
	float MaxDistance = 500.0f;
	float MinTessellation = 1.0f;
	float MaxTessellation = 64.0f;

	float TexelCellSpaceU;
	float TexelCellSpaceV;
	float WorldCellSpace = 1.0f;
	float HeightRatio = 20.0f;

	Vector2 TexScale = D3DXVECTOR2(8,8);
	float mainActorYPos = 0.0f;
	float Padding2;

	Plane WorldFrustumPlanes[6];
};

class TerrainLOD : public Renderer
{
public:
	 struct InitializeInfo;
public:
	 struct InitializeInfo
	{
		Shader* shader;

		wstring heightMap;
		float CellSpacing;
		UINT CellsPerPatch;
		float HeightRatio;
	};

	// ID3D11ShaderResourceView* mipSrv;
public:
	TerrainLOD(InitializeInfo& info);
	~TerrainLOD();

	void SetStart(bool bStart)
	{
		this->bStart = bStart;
	}
	void BaseTexture(wstring file);
	void PBRTextures(wstring normal, wstring rough, wstring display);

	void NormalTextures(wstring normal);

	void RoughnessTextures( wstring rough);

	Texture* HeightMap() { return heightMap; }

	void Update();
	void Render();

	bool Intersection(Vector3& Pos);

	void Intersection( Matrix* matrix);
	
	
	void RenderBox(QuadTreeNode* node);
	
private://ImGui
	void ImGui();
private:
	void ReadPixel();
	void UpdateHeightMap();

private:
	void RaiseHeight();
	void SmoothBrush();
	void Platting();
	void Splatting(Vector3& position, uint type, uint range, uint alpha, float factor);
	void BillboardBrush(Vector3& position, uint type, uint range, uint count,Vector2 scale);
	void BillboardFinder(wstring name);
	
	
private:
	float raiseSpeed;
	BrushType brushType;
	struct BrushDesc
	{
		Color Color = D3DXCOLOR(1, 0.2f, 0, 1);
		Vector3 Location;
		uint BrushShape = 0; //그리지 않음

		uint Range = 10;
		float Padding[3];
		

	}brushDesc;
	
	ConstantBuffer* brushBuffer;
	ID3DX11EffectConstantBuffer* sBrushBuffer;

	struct UpdateDesc
	{
		
		Vector3 Location;
		uint Range = 10;
		uint BrushShape;
		float raiseSpeed=0.1f;
		float Padding[2];
		

	}updateDesc;

	ConstantBuffer* updateBuffer;
	ID3DX11EffectConstantBuffer* sUpdateBuffer;

	ID3D11Texture2D* updateTexture;
	ID3D11Buffer* updateStructuerdBuffer;
	ID3D11UnorderedAccessView* updateUAV=nullptr;
	ID3D11ShaderResourceView* updateSRV;
	

	ID3D11ShaderResourceView* tempSRV;

private:
	class Shader* billboardShader;
	class Shader* UpdateShader;
	class Billboard* billboard;
	uint billbaordIndex;
	uint billboardCount;
	float billboardScale[2];
	Texture* billboardTexture[BillboardTextureCount];
	wstring currentPath;
	wstring selectedBillboard;

	vector<wstring> billboardTextures;
	vector<wstring> billboarfiles;
	vector<Billboard*> billboards;

private:
	Vector3 mousePos;
	Matrix world; //= transform->World();
	BufferDesc bufferDesc;
private:
	bool InBounds(UINT row, UINT col);
	void CalcBoundY();
	void CalcPatchBounds(UINT row, UINT col);

	void CreateVertexData();
	void CreateIndexData();
	void CreateQuadTree(QuadTreeNode* node, InitializeInfo info, Vector2 leftTop, Vector2 rightBottom);

	Vector2 GetMinMaxY(Vector2 tl, Vector2 br);

	//void UpdateQuadTree();
	//
	float Height(float px, float pz);
	
	
private:
	QuadTreeNode* root;
	const float tolerance = 0.01f;
	UINT faceCount;
	UINT patchVertexRows;
	UINT patchVertexCols;

private:
	InitializeInfo info;

	
	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	

	Texture* heightMap;

	

	UINT width, height;

	VertexTerrain* vertices;
	UINT* indices;


	vector<Vector2> bounds;

	vector<D3DXCOLOR> heightMapPixel;
	
	Texture* baseTexture;
	ID3DX11EffectShaderResourceVariable* sBaseTexture;

	Texture* normalTexture;
	ID3DX11EffectShaderResourceVariable* sNormalTexture;



	Texture* bumpTexture;
	ID3DX11EffectShaderResourceVariable* sBumpTexture;

	Texture* roughnessTexture;
	ID3DX11EffectShaderResourceVariable* sRoughnessTexture;

	

	ID3DX11EffectShaderResourceVariable* sHeightTexture;
	Texture* displaymentTexture;
	ID3DX11EffectShaderResourceVariable* sDisplaymentTexture;

	ID3DX11EffectShaderResourceVariable* sUpdateTexture;

	class Frustum* frustum;
	
	private:
	Vector3 org, dir;
	Matrix V;
	Matrix P; 

	Vector3 pos;

	bool bClicked = false;
	bool bStart = false;
	ID3DX11EffectUnorderedAccessViewVariable* sUpdate;

	/*struct RayDesc
	{
		Vector3 org;
		float pad1;
		Vector3 dir;
		float pad2;

	};
	RayDesc rayDesc;
	ConstantBuffer* rayBuffer; 
	ID3DX11EffectConstantBuffer* sRayBuffer;*/
	
	uint count;
	Vector3 lerp;
};



