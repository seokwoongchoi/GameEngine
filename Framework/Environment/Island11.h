#pragma once



#define terrain_gridpoints					512
#define terrain_numpatches_1d				64
#define terrain_geometry_scale				2.0f
#define terrain_maxheight					30.0f 
#define terrain_minheight					-30.0f 
#define terrain_fractalfactor				0.68f;
#define terrain_fractalinitialvalue			100.0f
#define terrain_smoothfactor1				0.99f
#define terrain_smoothfactor2				0.10f
#define terrain_rockfactor					0.95f
#define terrain_smoothsteps					40

#define terrain_height_underwater_start		-100.0f
#define terrain_height_underwater_end		-8.0f
#define terrain_height_sand_start			-30.0f
#define terrain_height_sand_end				1.7f
#define terrain_height_grass_start			1.7f
#define terrain_height_grass_end			30.0f
#define terrain_height_rocks_start			-2.0f
#define terrain_height_trees_start			4.0f
#define terrain_height_trees_end			30.0f
#define terrain_slope_grass_start			0.96f
#define terrain_slope_rocks_start			0.85f

#define terrain_far_range terrain_gridpoints*terrain_geometry_scale

#define shadowmap_resource_buffer_size_xy				4096
#define water_normalmap_resource_buffer_size_xy			2048
#define terrain_layerdef_map_texture_size				1024
#define terrain_depth_shadow_map_texture_size			512

#define sky_gridpoints						10
#define sky_texture_angle					0.425f

#define main_buffer_size_multiplier			1.0f
#define reflection_buffer_size_multiplier   1.0f
#define refraction_buffer_size_multiplier   1.0f

#define scene_z_near						0.1f
#define scene_z_far							1000.0f
#define camera_fov							45//110.0f


struct VertexIsland
{
	Vector2 origin;
	Vector2 size;
};
class Island11
{
public:
	explicit Island11(Shader* shader);
	~Island11();

	
		Island11(const Island11&) = delete;
	Island11& operator=(const Island11&) = delete;
	
	void ReCreateBuffers();
	void LoadTextures();
	void Reflection();
	void Update();
	void Terrain();
	void Water();
	
	void Refraction(ID3D11ShaderResourceView* srv, ID3D11Texture2D* texture);
	void FinalPass();
	void CreateTerrain();

private:
	class Shader* shader;
	
private:

	
	float g_BackBufferWidth = 1280.0f;
	float g_BackBufferHeight = 720.0f;
	float g_LightPosition[3] = { -10000.0f,6500.0f,10000.0f };

	// Statistics
	int g_CurrentFrame = 0;
	float g_FrameTime = 0;
	float g_TotalTime = 0;

	// Control variables
	bool g_RenderWireframe = false;
	bool g_QueryStats = false;
	bool g_RenderHUD = true;
	bool g_UseDynamicLOD = true;
	bool g_RenderCaustics = true;
	bool g_FrustumCullInHS = true;
	bool g_CycleViewPoints = false;


	float g_DynamicTessellationFactor = 50.0f;
	//float g_StaticTessellationFactor = 12.0f;

	class  Fixity* reflectionCam;
	Matrix rflectionMatrix;
private:
	float DynamicTesselationFactor;
	float StaticTesselationFactor;
	void SetupNormalView();
	void SetupReflectionView();
	void SetupRefractionView();
	void SetupLightView();
	float BackbufferWidth;
	float BackbufferHeight;

	UINT MultiSampleCount=1;
	UINT MultiSampleQuality=0;

	ID3D11Texture2D		*rock_bump_texture;
	ID3D11ShaderResourceView *rock_bump_textureSRV;

	ID3D11Texture2D		*rock_microbump_texture;
	ID3D11ShaderResourceView *rock_microbump_textureSRV;

	ID3D11Texture2D		*rock_diffuse_texture;
	ID3D11ShaderResourceView *rock_diffuse_textureSRV;

	ID3D11Texture2D		*sand_bump_texture;
	ID3D11ShaderResourceView *sand_bump_textureSRV;

	ID3D11Texture2D		*sand_microbump_texture;
	ID3D11ShaderResourceView *sand_microbump_textureSRV;

	ID3D11Texture2D		*sand_diffuse_texture;
	ID3D11ShaderResourceView *sand_diffuse_textureSRV;

	ID3D11Texture2D		*grass_diffuse_texture;
	ID3D11ShaderResourceView *grass_diffuse_textureSRV;

	ID3D11Texture2D		*slope_diffuse_texture;
	ID3D11ShaderResourceView *slope_diffuse_textureSRV;

	ID3D11Texture2D		*water_bump_texture;
	ID3D11ShaderResourceView *water_bump_textureSRV;
	
	ID3D11Texture2D			 *reflection_color_resource;
	ID3D11ShaderResourceView *reflection_color_resourceSRV;
	ID3D11RenderTargetView   *reflection_color_resourceRTV;

	ID3D11Texture2D			 *refraction_color_resource;
	ID3D11ShaderResourceView *refraction_color_resourceSRV;
	ID3D11RenderTargetView   *refraction_color_resourceRTV;

	ID3D11Texture2D			 *shadowmap_resource;
	ID3D11ShaderResourceView *shadowmap_resourceSRV;
	ID3D11DepthStencilView   *shadowmap_resourceDSV;

	ID3D11Texture2D			 *reflection_depth_resource;
	ID3D11DepthStencilView   *reflection_depth_resourceDSV;


	ID3D11Texture2D			 *refraction_depth_resource;
	ID3D11RenderTargetView   *refraction_depth_resourceRTV;
	ID3D11ShaderResourceView *refraction_depth_resourceSRV;

	ID3D11Texture2D			 *water_normalmap_resource;
	ID3D11ShaderResourceView *water_normalmap_resourceSRV;
	ID3D11RenderTargetView   *water_normalmap_resourceRTV;

	ID3D11Texture2D			 *main_color_resource;
	ID3D11ShaderResourceView *main_color_resourceSRV;
	ID3D11RenderTargetView   *main_color_resourceRTV;
	ID3D11Texture2D			 *main_depth_resource;
	ID3D11DepthStencilView   *main_depth_resourceDSV;
	ID3D11ShaderResourceView *main_depth_resourceSRV;
	ID3D11Texture2D			 *main_color_resource_resolved;
	ID3D11ShaderResourceView *main_color_resource_resolvedSRV;

	
	

	float				height[terrain_gridpoints + 1][terrain_gridpoints + 1];
	D3DXVECTOR3			normal[terrain_gridpoints + 1][terrain_gridpoints + 1];
	D3DXVECTOR3			tangent[terrain_gridpoints + 1][terrain_gridpoints + 1];
	D3DXVECTOR3			binormal[terrain_gridpoints + 1][terrain_gridpoints + 1];

	ID3D11Texture2D		*heightmap_texture;
	ID3D11ShaderResourceView *heightmap_textureSRV;

	ID3D11Texture2D		*layerdef_texture;
	ID3D11ShaderResourceView *layerdef_textureSRV;

	ID3D11Texture2D		*depthmap_texture;
	ID3D11ShaderResourceView *depthmap_textureSRV;

	ID3D11Buffer		*heightfield_vertexbuffer;
	//ID3D11Buffer		*sky_vertexbuffer;

	/*ID3D11InputLayout   *heightfield_inputlayout;
	ID3D11InputLayout   *trianglestrip_inputlayout;*/


	Matrix viewIndex;
	uint  camIndex;
	Vector3 position, rotation;

	Vector3 LookAtPoint;
	Vector3 EyePoint;
	

	D3DXVECTOR3 cameraPosition;
	class Camera* camera;

	Matrix view;
	Matrix proj;
	D3DXVECTOR3 direction;
	
private:
		struct ViewDesc
		{
			Matrix View;
			Matrix VP;
			Matrix VPInv;

			Vector3 camPos;
			float halfSpaceCullSign;

			Vector3 camDirection;
			float halfSpaceCullPosition;
		} viewDesc;


		ConstantBuffer* viewBuffer;
		ID3DX11EffectConstantBuffer* sViewBuffer;

		struct LightDesc
		{
			
			Matrix VP;
			Matrix VPInv;

			Vector3 Pos;
			float halfSpaceCullSign;

		} lightDesc;


		ConstantBuffer* lightBuffer;
		ID3DX11EffectConstantBuffer* sLightBuffer;

		D3DXVECTOR3 lookUp = D3DXVECTOR3(0, 1, 0);
		float nr, fr;
private:

		
			D3D11_VIEWPORT reflection_Viewport;
			D3D11_VIEWPORT water_normalmap_resource_viewport;
			D3D11_VIEWPORT main_Viewport;


			float origin[2] = { 0,0 };
			UINT stride = sizeof(float) * 4;
			UINT offset = 0;
			UINT cRT = 1;

			float ClearColor[4] = { 0.8f, 0.8f, 1.0f, 1.0f };
			float RefractionClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
			float cullSign = 1.0f;
			float halfSpaceCullPosition=2.0f;
};

float bilinear_interpolation(float fx, float fy, float a, float b, float c, float d);

