#pragma once
struct OceanParameter1
{
	// Must be power of 2.
	int dmap_dim;
	// Typical value is 1000 ~ 2000
	float patch_length;

	// Adjust the time interval for simulation.
	float time_scale;
	// Amplitude for transverse wave. Around 1.0
	float wave_amplitude;
	// Wind direction. Normalization not required.
	D3DXVECTOR2 wind_dir;
	// Around 100 ~ 1000
	float wind_speed;
	// This value damps out the waves against the wind direction.
	// Smaller value means higher wind dependency.
	float wind_dependency;
	// The amplitude for longitudinal wave. Must be positive.
	float choppy_scale;
};
struct ocean_vertex
{
	float index_x;
	float index_y;
};

// Quadtree structures & routines
struct QuadNode
{
	D3DXVECTOR2 bottom_left;
	float length;
	int lod;

	int sub_node[4];
};

struct QuadRenderParam
{
	UINT num_inner_verts;
	UINT num_inner_faces;
	UINT inner_start_index;

	UINT num_boundary_verts;
	UINT num_boundary_faces;
	UINT boundary_start_index;
};

class OceanRenderer
{
public:
	OceanRenderer(class Shader* shader,const Vector3 position);
	~OceanRenderer();
	
	void Update();
	// Rendering routines
	void renderShaded();

	void SetCubeMap(ID3D11ShaderResourceView* srv);
	
private:
	int generateInnerMesh(RECT vert_rect, DWORD * output);
	int generateBoundaryMesh(int left_degree, int right_degree, int bottom_degree, int top_degree,
		RECT vert_rect, DWORD* output);
	QuadRenderParam& selectMeshPattern(const QuadNode& quad_node);
	// init & cleanup
	void initRenderResource(const struct OceanParameter& ocean_param);

	// create a triangle strip mesh for ocean surface.
	void createSurfaceMesh();
	// create color/fresnel lookup table.
	void createFresnelMap();
	// create perlin noise texture for far-sight rendering
	void loadTextures();
	bool checkNodeVisibility(const QuadNode& quad_node);
	float estimateGridCoverage(const QuadNode& quad_node, float screen_area);
	bool isLeaf(const QuadNode& quad_node);
	int searchLeaf(const vector<QuadNode>& node_list, const D3DXVECTOR2& point);
	int buildNodeList(QuadNode& quad_node);


	
private:
	// Displacement map
	ID3D11Texture2D* m_pDisplacementMap = NULL;	// (RGBA32F)
	ID3D11ShaderResourceView* m_pDisplacementSRV = NULL;
	ID3D11RenderTargetView* m_pDisplacementRTV = NULL;

	// Gradient field
	ID3D11Texture2D* m_pGradientMap = NULL;			// (RGBA16F)
	ID3D11ShaderResourceView* m_pGradientSRV = NULL;
	ID3D11RenderTargetView* m_pGradientRTV = NULL;

	// Samplers
	ID3D11SamplerState* m_pPointSamplerState;
private:
	uint pass;
	VertexBuffer* vertexBuffer;
	IndexBuffer* indexBuffer;
	class Shader* shader;
	int g_MeshDim;
	float g_PatchLength;
	int g_DisplaceMapDim;
	float g_UpperGridCoverage;
	int g_FurthestCover;

	D3DXVECTOR3 g_SkyColor;
	D3DXVECTOR3 g_WaterbodyColor;

	// Perlin wave parameters
	float g_PerlinSize;
	float g_PerlinSpeed;
	D3DXVECTOR3 g_PerlinAmplitude;
	D3DXVECTOR3 g_PerlinGradient;
	D3DXVECTOR3 g_PerlinOctave;
	D3DXVECTOR2 g_WindDir;

	D3DXVECTOR3 g_BendParam;

	// Sunspot parameters
	D3DXVECTOR3 g_SunDir;
	D3DXVECTOR3 g_SunColor;
	float g_Shineness;

	int g_Lods;
	// Pattern lookup array. Filled at init time.
	QuadRenderParam g_mesh_patterns[9][3][3][3][3];
	// Pick a proper mesh pattern according to the adjacent patches.
	
	

	// Rendering list
	vector<QuadNode> g_render_list;
private:
	ID3D11Texture1D* g_pFresnelMap = NULL;
	ID3D11ShaderResourceView* g_pSRV_Fresnel = NULL;

	// Distant perlin wave
	ID3D11ShaderResourceView* g_pSRV_Perlin = NULL;

	// Environment maps
	ID3D11ShaderResourceView* g_pSRV_ReflectCube = NULL;
private:
	// D3D11 buffers and layout
	ID3D11Buffer* g_pMeshVB = NULL;
	ID3D11Buffer* g_pMeshIB = NULL;
	ID3D11InputLayout* g_pMeshLayout = NULL;

	// Environment maps
private:
	// Samplers
	ID3D11SamplerState* g_pHeightSampler = NULL;
	ID3D11SamplerState* g_pGradientSampler = NULL;
	ID3D11SamplerState* g_pFresnelSampler = NULL;
	ID3D11SamplerState* g_pPerlinSampler = NULL;
	ID3D11SamplerState* g_pCubeSampler = NULL;




private:
	// Constant buffer
	struct Const_Per_Call
	{
		D3DXMATRIX	g_matLocal;
		D3DXMATRIX	g_matWorldViewProj;	
		
		
		D3DXVECTOR2 g_UVBase;
		D3DXVECTOR2 g_PerlinMovement;

		D3DXVECTOR3	g_LocalEye;
		float Padding;

		
	};
	
	Const_Per_Call call_consts;
	ConstantBuffer* perBuffer;
	ID3DX11EffectConstantBuffer* sPerBuffer;

	struct Const_Shading
	{
		// Water-reflected sky color
		D3DXVECTOR3		g_SkyColor;
		float			unused0;
		// The color of bottomless water body
		D3DXVECTOR3		g_WaterbodyColor;
		float			g_Shineness;

		D3DXVECTOR3		g_SunDir;
		float			unused1;

		D3DXVECTOR3		g_SunColor;
		float			unused2;

		// The parameter is used for fixing an artifact
		D3DXVECTOR3		g_BendParam;
		float			g_PerlinSize;

		D3DXVECTOR3		g_PerlinAmplitude;
		float			unused3;

		D3DXVECTOR3		g_PerlinOctave;
		float			unused4;

		D3DXVECTOR3		g_PerlinGradient;
		float			g_TexelLength_x2;

		float			g_UVScale;
		float			g_UVOffset;
		float Padding[2];
	};
	Const_Shading shading_data;

	ConstantBuffer* constBuffer;
	ID3DX11EffectConstantBuffer* sConstBuffer;

	private:
		
		ID3DX11EffectShaderResourceVariable* sDisplacement;
		ID3DX11EffectShaderResourceVariable* sPerlin;
		ID3DX11EffectShaderResourceVariable* sGradient;
		ID3DX11EffectShaderResourceVariable* sFresnel;
		ID3DX11EffectShaderResourceVariable* sCube;

		//Texture* oceanBump;

		private:
			QuadNode root_node;
			D3DXMATRIX matView;
			D3DXMATRIX matProj;
			Vector3 lightDirection;
			Matrix T;
			D3DXMATRIX matScale;
			D3DXMATRIX matWorld;
			
			D3DXMATRIX matInvWV;

	

			Matrix ReflectionView;
			Vector3 position;
		
};

