#include "Framework.h"
#include "OceanRenderer.h"

#define FRESNEL_TEX_SIZE			512
#define PERLIN_TEX_SIZE				256
#include "OceanSimulator.h"
#include "Viewer/Fixity.h"


OceanRenderer::OceanRenderer(Shader * shader, const Vector3 position)
	:shader(shader), pass(0),position(position)
{

	
	if (!this->shader)
	{
		
		this->shader = new Shader(L"Deferred/ocean_shading.fx");
	}
		
	
	g_MeshDim = 256;
	g_UpperGridCoverage = 64.0f;
	g_FurthestCover = 8;

	g_SkyColor = D3DXVECTOR3(0.38f, 0.45f, 0.56f);
	//32,40,42
	g_WaterbodyColor = D3DXVECTOR3(0.125f, 0.15625f, 0.1640625f);
	//g_WaterbodyColor = D3DXVECTOR3(0.38f, 0.45f, 0.56f);
	g_PerlinSize = 0.1f;
	g_PerlinSpeed = 0.06f;
	g_PerlinAmplitude = D3DXVECTOR3(35, 42, 57);
	//g_PerlinAmplitude = D3DXVECTOR3(70, 82, 114);
	g_PerlinGradient = D3DXVECTOR3(1.4f, 1.6f, 2.2f);
	g_PerlinOctave = D3DXVECTOR3(1.12f, 0.59f, 0.23f);
	//g_BendParam = D3DXVECTOR3(0.1f, -0.4f, 0.2f);
	g_BendParam = D3DXVECTOR3(8.0f, -0.4f, 0.3f);
	g_SunDir = Context::Get()->LightDirection();
	//g_SunColor = D3DXVECTOR3(0.2f, 0.2f, 0.2f);
	g_SunColor = Vector3(0.74f,0.59f,0.247f);
	g_Shineness = 400.0f;

	g_Lods = 0;
	
	sDisplacement = this->shader->AsSRV("g_texDisplacement");
	sPerlin = this->shader->AsSRV("g_texPerlin");
	sGradient = this->shader->AsSRV("g_texGradient");
	sFresnel = this->shader->AsSRV("g_texFresnel");
	sCube = this->shader->AsSRV("g_texReflectCube");

	OceanParameter ocean_param;
	ocean_param.dmap_dim = 128;
	// The side length (world space) of square patch
	ocean_param.patch_length = 10;
	// Adjust this parameter to control the simulation speed
	ocean_param.time_scale = 0.8f;
	// A scale to control the amplitude. Not the world space height
	ocean_param.wave_amplitude = 0.35f;
	// 2D wind direction. No need to be normalized
	ocean_param.wind_dir = D3DXVECTOR2(0.8f, 0.6f);
	// The bigger the wind speed, the larger scale of wave crest.
	// But the wave scale can be no larger than patch_length
	ocean_param.wind_speed = 100.0f;
	// Damp out the components opposite to wind direction.
	// The smaller the value, the higher wind dependency
	ocean_param.wind_dependency = 0.07f;
	// Control the scale of horizontal movement. Higher value creates
	ocean_param.choppy_scale = 1.3f;
	initRenderResource(ocean_param);
	//oceanBump = new Texture(L"Environment/water_bump.dds");
	
	
	//CreateCubeMap();
}

OceanRenderer::~OceanRenderer()
{

	
	SafeRelease(g_pFresnelMap);
	SafeRelease(g_pSRV_Fresnel);
	SafeRelease(g_pSRV_Perlin);
	//SafeRelease(g_pSRV_ReflectCube);

	SafeRelease(g_pHeightSampler);
	SafeRelease(g_pGradientSampler);
	SafeRelease(g_pFresnelSampler);
	SafeRelease(g_pPerlinSampler);
	SafeRelease(g_pCubeSampler);



	//for (UINT i = 0; i < 6; i++)
	//	SafeDelete(cameras[i]);

	
}

void OceanRenderer::Update()
{
	
}

void OceanRenderer::initRenderResource(const struct OceanParameter & ocean_param)
{
	g_PatchLength = ocean_param.patch_length;
	g_DisplaceMapDim = ocean_param.dmap_dim;
	g_WindDir = ocean_param.wind_dir;

	// D3D buffers
	createSurfaceMesh();
	createFresnelMap();
	loadTextures();
	
	constBuffer = new ConstantBuffer(&shading_data, sizeof(Const_Shading));
	sConstBuffer = shader->AsConstantBuffer("cbShading");

	perBuffer = new ConstantBuffer(&call_consts, sizeof(Const_Per_Call));
	sPerBuffer = shader->AsConstantBuffer("cbChangePerCall");
	
	// Grid side length * 2
	shading_data.g_TexelLength_x2 = g_PatchLength / g_DisplaceMapDim * 2;;
	// Color
	shading_data.g_SkyColor = g_SkyColor;
	shading_data.g_WaterbodyColor = g_WaterbodyColor;
	// Texcoord
	shading_data.g_UVScale = 1.0f / g_PatchLength;
	shading_data.g_UVOffset = 0.5f / g_DisplaceMapDim;
	// Perlin
	shading_data.g_PerlinSize = g_PerlinSize;
	shading_data.g_PerlinAmplitude = g_PerlinAmplitude;
	shading_data.g_PerlinGradient = g_PerlinGradient;
	shading_data.g_PerlinOctave = g_PerlinOctave;
	// Multiple reflection workaround
	shading_data.g_BendParam = g_BendParam;
	// Sun streaks
	shading_data.g_SunColor = g_SunColor;
	//shading_data.g_SunDir = g_SunDir;
	Vector3 lightDirection = Context::Get()->LightDirection();
	shading_data.g_SunDir = Vector3(-lightDirection.x, lightDirection.y, lightDirection.z);
	shading_data.g_Shineness = g_Shineness;

    // Samplers
    D3D11_SAMPLER_DESC sam_desc;
    sam_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sam_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sam_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sam_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sam_desc.MipLODBias = 0;
    sam_desc.MaxAnisotropy = 1;
    sam_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sam_desc.BorderColor[0] = 1.0f;
    sam_desc.BorderColor[1] = 1.0f;
    sam_desc.BorderColor[2] = 1.0f;
    sam_desc.BorderColor[3] = 1.0f;
    sam_desc.MinLOD = 0;
    sam_desc.MaxLOD = FLT_MAX;
    Check(D3D::GetDevice()->CreateSamplerState(&sam_desc, &g_pHeightSampler));
    
    
    sam_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    Check(D3D::GetDevice()->CreateSamplerState(&sam_desc, &g_pCubeSampler));
    
    
    sam_desc.Filter = D3D11_FILTER_ANISOTROPIC;
    sam_desc.MaxAnisotropy = 8;
    Check(D3D::GetDevice()->CreateSamplerState(&sam_desc, &g_pGradientSampler));
    
    sam_desc.MaxLOD = FLT_MAX;
    sam_desc.MaxAnisotropy = 4;
    Check(D3D::GetDevice()->CreateSamplerState(&sam_desc, &g_pPerlinSampler));
    
    sam_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sam_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sam_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sam_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    Check(D3D::GetDevice()->CreateSamplerState(&sam_desc, &g_pFresnelSampler));

    shader->AsSampler("g_samplerDisplacement")->SetSampler(0, g_pHeightSampler);
    shader->AsSampler("g_samplerPerlin")->SetSampler(0, g_pPerlinSampler);
    shader->AsSampler("g_samplerGradient")->SetSampler(0, g_pGradientSampler);
    shader->AsSampler("g_samplerFresnel")->SetSampler(0, g_pFresnelSampler);
    shader->AsSampler("g_samplerCube")->SetSampler(0, g_pCubeSampler);
}

void OceanRenderer::createSurfaceMesh()
{
	// --------------------------------- Vertex Buffer -------------------------------
	int num_verts = (g_MeshDim + 1) * (g_MeshDim + 1);
	Vertex* pV = new Vertex[num_verts];
	assert(pV);

	int i, j;
	for (i = 0; i <= g_MeshDim; i++)
	{
		for (j = 0; j <= g_MeshDim; j++)
		{
			pV[i * (g_MeshDim + 1) + j].Position.x = (float)j;
			pV[i * (g_MeshDim + 1) + j].Position.y = (float)i;
			pV[i * (g_MeshDim + 1) + j].Position.z = 0;
		}
	}

	vertexBuffer = new VertexBuffer(pV, num_verts, sizeof(Vertex));

	// --------------------------------- Index Buffer -------------------------------
	// The index numbers for all mesh LODs (up to 256x256)
	const int index_size_lookup[] = { 0, 0, 4284, 18828, 69444, 254412, 956916, 3689820, 14464836 };

	memset(&g_mesh_patterns[0][0][0][0][0], 0, sizeof(g_mesh_patterns));

	g_Lods = 0;
	for (i = g_MeshDim; i > 1; i >>= 1)
		g_Lods++;

	// Generate patch meshes. Each patch contains two parts: the inner mesh which is a regular
	// grids in a triangle strip. The boundary mesh is constructed w.r.t. the edge degrees to
	// meet water-tight requirement.
	DWORD* index_array = new DWORD[index_size_lookup[g_Lods]];
	assert(index_array);

	int offset = 0;
	int level_size = g_MeshDim;

	// Enumerate patterns
	for (int level = 0; level <= g_Lods - 2; level++)
	{
		int left_degree = level_size;

		for (int left_type = 0; left_type < 3; left_type++)
		{
			int right_degree = level_size;

			for (int right_type = 0; right_type < 3; right_type++)
			{
				int bottom_degree = level_size;

				for (int bottom_type = 0; bottom_type < 3; bottom_type++)
				{
					int top_degree = level_size;

					for (int top_type = 0; top_type < 3; top_type++)
					{
						QuadRenderParam* pattern = &g_mesh_patterns[level][left_type][right_type][bottom_type][top_type];

						// Inner mesh (triangle strip)
						RECT inner_rect;
						inner_rect.left = (left_degree == level_size) ? 0 : 1;
						inner_rect.right = (right_degree == level_size) ? level_size : level_size - 1;
						inner_rect.bottom = (bottom_degree == level_size) ? 0 : 1;
						inner_rect.top = (top_degree == level_size) ? level_size : level_size - 1;

						int num_new_indices = generateInnerMesh(inner_rect, index_array + offset);

						pattern->inner_start_index = offset;
						pattern->num_inner_verts = (level_size + 1) * (level_size + 1);
						pattern->num_inner_faces = num_new_indices - 2;
						offset += num_new_indices;

						// Boundary mesh (triangle list)
						int l_degree = (left_degree == level_size) ? 0 : left_degree;
						int r_degree = (right_degree == level_size) ? 0 : right_degree;
						int b_degree = (bottom_degree == level_size) ? 0 : bottom_degree;
						int t_degree = (top_degree == level_size) ? 0 : top_degree;

						RECT outer_rect = { 0, level_size, level_size, 0 };
						num_new_indices = generateBoundaryMesh(l_degree, r_degree, b_degree, t_degree, outer_rect, index_array + offset);

						pattern->boundary_start_index = offset;
						pattern->num_boundary_verts = (level_size + 1) * (level_size + 1);
						pattern->num_boundary_faces = num_new_indices / 3;
						offset += num_new_indices;

						top_degree /= 2;
					}
					bottom_degree /= 2;
				}
				right_degree /= 2;
			}
			left_degree /= 2;
		}
		level_size /= 2;
	}

	assert(offset == index_size_lookup[g_Lods]);
	indexBuffer = new IndexBuffer(index_array, index_size_lookup[g_Lods]);
	//
	SafeDeleteArray(pV);
	SafeDeleteArray(index_array);
}
int OceanRenderer::generateInnerMesh(RECT vert_rect, DWORD * output)
{

	int i, j;
	int counter = 0;
	int width = vert_rect.right - vert_rect.left;
	int height = vert_rect.top - vert_rect.bottom;

	bool reverse = false;
	for (i = 0; i < height; i++)
	{

		if (reverse == false)
		{

			output[counter++] = ((i)+vert_rect.bottom) * (g_MeshDim + 1) + (0) + vert_rect.left;
			output[counter++] = ((i + 1) + vert_rect.bottom) * (g_MeshDim + 1) + (0) + vert_rect.left;
			for (j = 0; j < width; j++)
			{
				output[counter++] = ((i)+vert_rect.bottom) * (g_MeshDim + 1) + (j + 1) + vert_rect.left;
				output[counter++] = ((i + 1) + vert_rect.bottom) * (g_MeshDim + 1) + (j + 1) + vert_rect.left;
			}
		}
		else
		{
			output[counter++] = ((i)+vert_rect.bottom) * (g_MeshDim + 1) + (width)+vert_rect.left;
			output[counter++] = ((i + 1) + vert_rect.bottom) * (g_MeshDim + 1) + (width)+vert_rect.left;
			for (j = width - 1; j >= 0; j--)
			{
				output[counter++] = ((i)+vert_rect.bottom) * (g_MeshDim + 1) + (j)+vert_rect.left;
				output[counter++] = ((i + 1) + vert_rect.bottom) * (g_MeshDim + 1) + (j)+vert_rect.left;
			}
		}

		reverse = !reverse;
	}

	return counter;
}

int OceanRenderer::generateBoundaryMesh(int left_degree, int right_degree, int bottom_degree, int top_degree, RECT vert_rect, DWORD * output)
{
	// Triangle list for bottom boundary
	int i, j;
	int counter = 0;
	int width = vert_rect.right - vert_rect.left;

	if (bottom_degree > 0)
	{
		int b_step = width / bottom_degree;

		for (i = 0; i < width; i += b_step)
		{
			output[counter++] = ((0) + vert_rect.bottom) * (g_MeshDim + 1) + (i)+vert_rect.left;
			output[counter++] = ((1) + vert_rect.bottom) * (g_MeshDim + 1) + (i + b_step / 2) + vert_rect.left;
			output[counter++] = ((0) + vert_rect.bottom) * (g_MeshDim + 1) + (i + b_step) + vert_rect.left;

			for (j = 0; j < b_step / 2; j++)
			{
				if (i == 0 && j == 0 && left_degree > 0)
					continue;

				output[counter++] = ((0) + vert_rect.bottom) * (g_MeshDim + 1) + (i)+vert_rect.left;
				output[counter++] = ((1) + vert_rect.bottom) * (g_MeshDim + 1) + (i + j) + vert_rect.left;
				output[counter++] = ((1) + vert_rect.bottom) * (g_MeshDim + 1) + (i + j + 1) + vert_rect.left;
			}

			for (j = b_step / 2; j < b_step; j++)
			{
				if (i == width - b_step && j == b_step - 1 && right_degree > 0)
					continue;

				output[counter++] = ((0) + vert_rect.bottom) * (g_MeshDim + 1) + (i + b_step) + vert_rect.left;
				output[counter++] = ((1) + vert_rect.bottom) * (g_MeshDim + 1) + (i + j) + vert_rect.left;
				output[counter++] = ((1) + vert_rect.bottom) * (g_MeshDim + 1) + (i + j + 1) + vert_rect.left;
			}
		}
	}

	// Right boundary
	int height = vert_rect.top - vert_rect.bottom;

	if (right_degree > 0)
	{
		int r_step = height / right_degree;

		for (i = 0; i < height; i += r_step)
		{
			output[counter++] = ((i)+vert_rect.bottom) * (g_MeshDim + 1) + (width)+vert_rect.left;
			output[counter++] = ((i + r_step / 2) + vert_rect.bottom) * (g_MeshDim + 1) + (width - 1) + vert_rect.left;
			output[counter++] = ((i + r_step) + vert_rect.bottom) * (g_MeshDim + 1) + (width)+vert_rect.left;

			for (j = 0; j < r_step / 2; j++)
			{
				if (i == 0 && j == 0 && bottom_degree > 0)
					continue;

				output[counter++] = ((i)+vert_rect.bottom) * (g_MeshDim + 1) + (width)+vert_rect.left;
				output[counter++] = ((i + j) + vert_rect.bottom) * (g_MeshDim + 1) + (width - 1) + vert_rect.left;
				output[counter++] = ((i + j + 1) + vert_rect.bottom) * (g_MeshDim + 1) + (width - 1) + vert_rect.left;
			}

			for (j = r_step / 2; j < r_step; j++)
			{
				if (i == height - r_step && j == r_step - 1 && top_degree > 0)
					continue;

				output[counter++] = ((i + r_step) + vert_rect.bottom) * (g_MeshDim + 1) + (width)+vert_rect.left;
				output[counter++] = ((i + j) + vert_rect.bottom) * (g_MeshDim + 1) + (width - 1) + vert_rect.left;
				output[counter++] = ((i + j + 1) + vert_rect.bottom) * (g_MeshDim + 1) + (width - 1) + vert_rect.left;
			}
		}
	}

	// Top boundary
	if (top_degree > 0)
	{
		int t_step = width / top_degree;

		for (i = 0; i < width; i += t_step)
		{
			output[counter++] = ((height)+vert_rect.bottom) * (g_MeshDim + 1) + (i)+vert_rect.left;
			output[counter++] = ((height - 1) + vert_rect.bottom) * (g_MeshDim + 1) + (i + t_step / 2) + vert_rect.left;
			output[counter++] = ((height)+vert_rect.bottom) * (g_MeshDim + 1) + (i + t_step) + vert_rect.left;

			for (j = 0; j < t_step / 2; j++)
			{
				if (i == 0 && j == 0 && left_degree > 0)
					continue;

				output[counter++] = ((height)+vert_rect.bottom) * (g_MeshDim + 1) + (i)+vert_rect.left;
				output[counter++] = ((height - 1) + vert_rect.bottom) * (g_MeshDim + 1) + (i + j) + vert_rect.left;
				output[counter++] = ((height - 1) + vert_rect.bottom) * (g_MeshDim + 1) + (i + j + 1) + vert_rect.left;
			}

			for (j = t_step / 2; j < t_step; j++)
			{
				if (i == width - t_step && j == t_step - 1 && right_degree > 0)
					continue;

				output[counter++] = ((height)+vert_rect.bottom) * (g_MeshDim + 1) + (i + t_step) + vert_rect.left;
				output[counter++] = ((height - 1) + vert_rect.bottom) * (g_MeshDim + 1) + (i + j) + vert_rect.left;
				output[counter++] = ((height - 1) + vert_rect.bottom) * (g_MeshDim + 1) + (i + j + 1) + vert_rect.left;
			}
		}
	}

	// Left boundary
	if (left_degree > 0)
	{
		int l_step = height / left_degree;

		for (i = 0; i < height; i += l_step)
		{
			output[counter++] = ((i)+vert_rect.bottom) * (g_MeshDim + 1) + (0) + vert_rect.left;
			output[counter++] = ((i + l_step / 2) + vert_rect.bottom) * (g_MeshDim + 1) + (1) + vert_rect.left;
			output[counter++] = ((i + l_step) + vert_rect.bottom) * (g_MeshDim + 1) + (0) + vert_rect.left;

			for (j = 0; j < l_step / 2; j++)
			{
				if (i == 0 && j == 0 && bottom_degree > 0)
					continue;

				output[counter++] = ((i)+vert_rect.bottom) * (g_MeshDim + 1) + (0) + vert_rect.left;
				output[counter++] = ((i + j) + vert_rect.bottom) * (g_MeshDim + 1) + (1) + vert_rect.left;
				output[counter++] = ((i + j + 1) + vert_rect.bottom) * (g_MeshDim + 1) + (1) + vert_rect.left;
			}

			for (j = l_step / 2; j < l_step; j++)
			{
				if (i == height - l_step && j == l_step - 1 && top_degree > 0)
					continue;

				output[counter++] = ((i + l_step) + vert_rect.bottom) * (g_MeshDim + 1) + (0) + vert_rect.left;
				output[counter++] = ((i + j) + vert_rect.bottom) * (g_MeshDim + 1) + (1) + vert_rect.left;
				output[counter++] = ((i + j + 1) + vert_rect.bottom) * (g_MeshDim + 1) + (1) + vert_rect.left;
			}
		}
	}

	return counter;
}

void OceanRenderer::createFresnelMap()
{
	DWORD* buffer = new DWORD[FRESNEL_TEX_SIZE];
		for (int i = 0; i < FRESNEL_TEX_SIZE; i++)
		{
			float cos_a = i / (FLOAT)FRESNEL_TEX_SIZE;
			// Using water's refraction index 1.33
			DWORD fresnel = (DWORD)(D3DXFresnelTerm(cos_a, 1.33f) * 255);
	
			DWORD sky_blend = (DWORD)(powf(1 / (1 + cos_a), 16.0f) * 255);
	
			buffer[i] = (sky_blend << 8) | fresnel;
		}
	
		D3D11_TEXTURE1D_DESC tex_desc;
		tex_desc.Width = FRESNEL_TEX_SIZE;
		tex_desc.MipLevels = 1;
		tex_desc.ArraySize = 1;
		tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		tex_desc.Usage = D3D11_USAGE_IMMUTABLE;
		tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		tex_desc.CPUAccessFlags = 0;
		tex_desc.MiscFlags = 0;
	
		D3D11_SUBRESOURCE_DATA init_data;
		init_data.pSysMem = buffer;
		init_data.SysMemPitch = 0;
		init_data.SysMemSlicePitch = 0;
	
		Check(D3D::GetDevice()->CreateTexture1D(&tex_desc, &init_data, &g_pFresnelMap));
		
		SafeDeleteArray(buffer);
		
	
		// Create shader resource
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
		srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
		srv_desc.Texture1D.MipLevels = 1;
		srv_desc.Texture1D.MostDetailedMip = 0;
	
		Check(D3D::GetDevice()->CreateShaderResourceView(g_pFresnelMap, &srv_desc, &g_pSRV_Fresnel));
		sFresnel->SetResource(g_pSRV_Fresnel);
	
}

void OceanRenderer::loadTextures()
{
	

		//wstring temp = L"../../_Textures/Environment/reflect_cube.dds";
	//wstring temp = L"../../_Textures/Environment/SunsetCube1024.dds";
	wstring temp = L"../../_Textures/Environment/SunsetCube1024.dds";
		/*D3DX11CreateShaderResourceViewFromFile
		(
			D3D::GetDevice(), temp.c_str(), NULL, NULL, &g_pSRV_ReflectCube, NULL
		);
*/
		temp = L"../../_Textures/Environment/perlin_noise.dds";

		D3DX11CreateShaderResourceViewFromFile
		(
			D3D::GetDevice(), temp.c_str(), NULL, NULL, &g_pSRV_Perlin, NULL
		);

		sPerlin->SetResource(g_pSRV_Perlin);
		
		//sCube->SetResource(g_pSRV_ReflectCube);

}

void OceanRenderer::renderShaded()
{
	
	
	g_render_list.clear();
	float ocean_extent = g_PatchLength * (1 << g_FurthestCover);
	root_node = { D3DXVECTOR2(-ocean_extent * 0.5f+ position.x, -ocean_extent * 0.5f+ position.y), ocean_extent,0, {-1,-1,-1,-1} };
	buildNodeList(root_node);

	matView = D3DXMATRIX(
		1, 0, 0, 0,
		0, 0, 1, 0,
		0, 1, 0, 0,
		0, 0, 0, 1)*Context::Get()->View();
	matProj = Context::Get()->Projection();

	//shader->AsSRV("ReflectionMap")->SetResource(rendertarget->SRV());
	
	
	vertexBuffer->Render();
	indexBuffer->Render();
	/*if (ImGui::Begin("Ocean"))
	{
		ImGui::ColorEdit3("oceanColor", (float*)&g_WaterbodyColor);
		shading_data.g_WaterbodyColor = g_WaterbodyColor;
		ImGui::ColorEdit3("_SunColor", (float*)&g_SunColor);
		lightDirection = Context::Get()->LightDirection();
	}
	ImGui::End();*/
	
    D3DXVec3Lerp(&shading_data.g_SunColor, &Vector3(1, 1, 1), &g_SunColor, lightDirection.x);

	shading_data.g_SunDir = Vector3(-lightDirection.x, lightDirection.y, lightDirection.z);
	shading_data.g_BendParam = g_BendParam;
	shading_data.g_Shineness = g_Shineness;

	if (sConstBuffer)
	{

		constBuffer -> Apply();
		sConstBuffer->SetConstantBuffer(constBuffer->Buffer());
	}
	
	
	for (int i = 0; i < (int)g_render_list.size(); i++)
	{
		QuadNode& node = g_render_list[i];

		if (!isLeaf(node))
			continue;

		QuadRenderParam& render_param = selectMeshPattern(node);
		
		int level_size = g_MeshDim;
		for (int lod = 0; lod < node.lod; lod++)
			level_size >>= 1;

	
	
		
		D3DXMatrixScaling(&matScale, node.length / level_size, node.length / level_size, 0);
		call_consts.g_matLocal =  matScale ;
		
		D3DXMatrixTranslation(&matWorld, node.bottom_left.x, node.bottom_left.y, position.z);
		
		call_consts.g_matWorldViewProj = matWorld * matView *matProj;
		
	
		D3DXVECTOR2 uv_base = node.bottom_left / g_PatchLength * g_PerlinSize;
		call_consts.g_UVBase = uv_base;

		
		D3DXVECTOR2 perlin_move = -g_WindDir * Time::Get()->Running() * g_PerlinSpeed;
		call_consts.g_PerlinMovement = perlin_move;
		
	
		matInvWV = matWorld  * matView;
		D3DXMatrixInverse(&matInvWV, NULL, &matInvWV);
		D3DXVECTOR3 vLocalEye(0, 0, 0);
		D3DXVec3TransformCoord(&vLocalEye, &vLocalEye, &matInvWV);
		call_consts.g_LocalEye = vLocalEye;

	
		
		if (sPerBuffer)
		{

			perBuffer->Apply();
			sPerBuffer->SetConstantBuffer(perBuffer->Buffer());
		}

		//transform->Render();
		// Perform draw call
		if (render_param.num_inner_faces > 0)
		{
			// Inner mesh of the patch
			D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			shader->DrawIndexed(0, pass,render_param.num_inner_faces + 2, render_param.inner_start_index, 0);
		}

		if (render_param.num_boundary_faces > 0)
		{
			// Boundary mesh of the patch
			D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			shader->DrawIndexed(0, pass,render_param.num_boundary_faces * 3, render_param.boundary_start_index, 0);
		}
	}
}

void OceanRenderer::SetCubeMap(ID3D11ShaderResourceView * srv)
{
	if (g_pSRV_ReflectCube != nullptr) return;
	g_pSRV_ReflectCube = srv;
	sCube->SetResource(g_pSRV_ReflectCube);
	
}


bool OceanRenderer::checkNodeVisibility(const QuadNode & quad_node)
{
	// Plane equation setup


	
	// Left plane
	//float fov_x = atan(0.5f / matProj.m[0][0]);
	float fov_x = atan(1.5f / matProj.m[0][0]);
	D3DXVECTOR4 plane_left(cos(fov_x), 0, sin(fov_x), 0);
	// Right plane
	D3DXVECTOR4 plane_right(-cos(fov_x), 0, sin(fov_x), 0);

	// Bottom plane
	//float fov_y = atan(0.5f / matProj.m[1][1]);
	float fov_y = atan(3.0f / matProj.m[1][1]);
	D3DXVECTOR4 plane_bottom(0, cos(fov_y), sin(fov_y), 0);
	// Top plane
	D3DXVECTOR4 plane_top(0, -cos(fov_y), sin(fov_y), 0);

	// Test quad corners against view frustum in view space
	D3DXVECTOR4 corner_verts[4];
	corner_verts[0] = D3DXVECTOR4(quad_node.bottom_left.x, quad_node.bottom_left.y, 0, 1);
	corner_verts[1] = corner_verts[0] + D3DXVECTOR4(quad_node.length, 0, 0, 0);
	corner_verts[2] = corner_verts[0] + D3DXVECTOR4(quad_node.length, quad_node.length, 0, 0);
	corner_verts[3] = corner_verts[0] + D3DXVECTOR4(0, quad_node.length, 0, 0);
	auto view = Context::Get()->View();

	D3DXMATRIX matView = D3DXMATRIX(1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1) * view;
	D3DXVec4Transform(&corner_verts[0], &corner_verts[0], &matView);
	D3DXVec4Transform(&corner_verts[1], &corner_verts[1], &matView);
	D3DXVec4Transform(&corner_verts[2], &corner_verts[2], &matView);
	D3DXVec4Transform(&corner_verts[3], &corner_verts[3], &matView);

	// Test against eye plane
	if (corner_verts[0].z < 0 && corner_verts[1].z < 0 && corner_verts[2].z < 0 && corner_verts[3].z < 0)
		return false;

	// Test against left plane
	float dist_0 = D3DXVec4Dot(&corner_verts[0], &plane_left);
	float dist_1 = D3DXVec4Dot(&corner_verts[1], &plane_left);
	float dist_2 = D3DXVec4Dot(&corner_verts[2], &plane_left);
	float dist_3 = D3DXVec4Dot(&corner_verts[3], &plane_left);
	if (dist_0 < 0 && dist_1 < 0 && dist_2 < 0 && dist_3 < 0)
		return false;

	// Test against right plane
	dist_0 = D3DXVec4Dot(&corner_verts[0], &plane_right);
	dist_1 = D3DXVec4Dot(&corner_verts[1], &plane_right);
	dist_2 = D3DXVec4Dot(&corner_verts[2], &plane_right);
	dist_3 = D3DXVec4Dot(&corner_verts[3], &plane_right);
	if (dist_0 < 0 && dist_1 < 0 && dist_2 < 0 && dist_3 < 0)
		return false;

	// Test against bottom plane
	dist_0 = D3DXVec4Dot(&corner_verts[0], &plane_bottom);
	dist_1 = D3DXVec4Dot(&corner_verts[1], &plane_bottom);
	dist_2 = D3DXVec4Dot(&corner_verts[2], &plane_bottom);
	dist_3 = D3DXVec4Dot(&corner_verts[3], &plane_bottom);
	if (dist_0 < 0 && dist_1 < 0 && dist_2 < 0 && dist_3 < 0)
		return false;

	// Test against top plane
	dist_0 = D3DXVec4Dot(&corner_verts[0], &plane_top);
	dist_1 = D3DXVec4Dot(&corner_verts[1], &plane_top);
	dist_2 = D3DXVec4Dot(&corner_verts[2], &plane_top);
	dist_3 = D3DXVec4Dot(&corner_verts[3], &plane_top);
	if (dist_0 < 0 && dist_1 < 0 && dist_2 < 0 && dist_3 < 0)
		return false;

	return true;
}

float OceanRenderer::estimateGridCoverage(const QuadNode & quad_node, float screen_area)
{
	// Estimate projected area

	// Test 16 points on the quad and find out the biggest one.
	const static float sample_pos[16][2] =
	{
		{0, 0},
		{0, 1},
		{1, 0},
		{1, 1},
		{0.5f, 0.333f},
		{0.25f, 0.667f},
		{0.75f, 0.111f},
		{0.125f, 0.444f},
		{0.625f, 0.778f},
		{0.375f, 0.222f},
		{0.875f, 0.556f},
		{0.0625f, 0.889f},
		{0.5625f, 0.037f},
		{0.3125f, 0.37f},
		{0.8125f, 0.704f},
		{0.1875f, 0.148f},
	};


	D3DXVECTOR3 eye_point=Vector3(0,0,0); 
	Context::Get()->GetCamera()->Position(&eye_point);
	eye_point = D3DXVECTOR3(eye_point.x, eye_point.z, eye_point.y);
	float grid_len_world = quad_node.length / g_MeshDim;

	float max_area_proj = 0;
	for (int i = 0; i < 16; i++)
	{
		D3DXVECTOR3 test_point(quad_node.bottom_left.x + quad_node.length * sample_pos[i][0], quad_node.bottom_left.y + quad_node.length * sample_pos[i][1], 0);
		D3DXVECTOR3 eye_vec = test_point - eye_point;
		float dist = D3DXVec3Length(&eye_vec);

		float area_world = grid_len_world * grid_len_world;// *abs(eye_point.z) / sqrt(nearest_sqr_dist);
		float area_proj = area_world * matProj(0, 0) * matProj(1, 1) / (dist * dist);

		if (max_area_proj < area_proj)
			max_area_proj = area_proj;
	}

	float pixel_coverage = max_area_proj * screen_area * 0.25f;

	return pixel_coverage;
}

bool OceanRenderer::isLeaf(const QuadNode & quad_node)
{
	return (quad_node.sub_node[0] == -1 && quad_node.sub_node[1] == -1 && quad_node.sub_node[2] == -1 && quad_node.sub_node[3] == -1);
}

int OceanRenderer::searchLeaf(const vector<QuadNode>& node_list, const D3DXVECTOR2 & point)
{
	return 0; int index = -1;

	int size = (int)node_list.size();
	QuadNode node = node_list[size - 1];

	while (!isLeaf(node))
	{
		bool found = false;

		for (int i = 0; i < 4; i++)
		{
			index = node.sub_node[i];
			if (index == -1)
				continue;

			QuadNode sub_node = node_list[index];
			if (point.x >= sub_node.bottom_left.x && point.x <= sub_node.bottom_left.x + sub_node.length &&
				point.y >= sub_node.bottom_left.y && point.y <= sub_node.bottom_left.y + sub_node.length)
			{
				node = sub_node;
				found = true;
				break;
			}
		}

		if (!found)
			return -1;
	}

	return index;
}

QuadRenderParam & OceanRenderer::selectMeshPattern(const QuadNode & quad_node)
{
	// Check 4 adjacent quad.
	D3DXVECTOR2 point_left = quad_node.bottom_left + D3DXVECTOR2(-g_PatchLength * 0.5f, quad_node.length * 0.5f);
	int left_adj_index = searchLeaf(g_render_list, point_left);

	D3DXVECTOR2 point_right = quad_node.bottom_left + D3DXVECTOR2(quad_node.length + g_PatchLength * 0.5f, quad_node.length * 0.5f);
	int right_adj_index = searchLeaf(g_render_list, point_right);

	D3DXVECTOR2 point_bottom = quad_node.bottom_left + D3DXVECTOR2(quad_node.length * 0.5f, -g_PatchLength * 0.5f);
	int bottom_adj_index = searchLeaf(g_render_list, point_bottom);

	D3DXVECTOR2 point_top = quad_node.bottom_left + D3DXVECTOR2(quad_node.length * 0.5f, quad_node.length + g_PatchLength * 0.5f);
	int top_adj_index = searchLeaf(g_render_list, point_top);

	int left_type = 0;
	if (left_adj_index != -1 && g_render_list[left_adj_index].length > quad_node.length * 0.999f)
	{
		QuadNode adj_node = g_render_list[left_adj_index];
		float scale = adj_node.length / quad_node.length * (g_MeshDim >> quad_node.lod) / (g_MeshDim >> adj_node.lod);
		if (scale > 3.999f)
			left_type = 2;
		else if (scale > 1.999f)
			left_type = 1;
	}

	int right_type = 0;
	if (right_adj_index != -1 && g_render_list[right_adj_index].length > quad_node.length * 0.999f)
	{
		QuadNode adj_node = g_render_list[right_adj_index];
		float scale = adj_node.length / quad_node.length * (g_MeshDim >> quad_node.lod) / (g_MeshDim >> adj_node.lod);
		if (scale > 3.999f)
			right_type = 2;
		else if (scale > 1.999f)
			right_type = 1;
	}

	int bottom_type = 0;
	if (bottom_adj_index != -1 && g_render_list[bottom_adj_index].length > quad_node.length * 0.999f)
	{
		QuadNode adj_node = g_render_list[bottom_adj_index];
		float scale = adj_node.length / quad_node.length * (g_MeshDim >> quad_node.lod) / (g_MeshDim >> adj_node.lod);
		if (scale > 3.999f)
			bottom_type = 2;
		else if (scale > 1.999f)
			bottom_type = 1;
	}

	int top_type = 0;
	if (top_adj_index != -1 && g_render_list[top_adj_index].length > quad_node.length * 0.999f)
	{
		QuadNode adj_node = g_render_list[top_adj_index];
		float scale = adj_node.length / quad_node.length * (g_MeshDim >> quad_node.lod) / (g_MeshDim >> adj_node.lod);
		if (scale > 3.999f)
			top_type = 2;
		else if (scale > 1.999f)
			top_type = 1;
	}

	// Check lookup table, [L][R][B][T]
	return g_mesh_patterns[quad_node.lod][left_type][right_type][bottom_type][top_type];
}

int OceanRenderer::buildNodeList(QuadNode & quad_node)
{
	// Check against view frustum
	if (!checkNodeVisibility(quad_node))
		return -1;
//
	// Estimate the min grid coverage
	UINT num_vps = 1;
	D3D11_VIEWPORT vp;
	D3D::GetDC()->RSGetViewports(&num_vps, &vp);
	float min_coverage = estimateGridCoverage(quad_node, (float)vp.Width * vp.Height);

	// Recursively attatch sub-nodes.
	bool visible = true;
	if (min_coverage > g_UpperGridCoverage && quad_node.length > g_PatchLength)
	{
		// Recursive rendering for sub-quads.
		QuadNode sub_node_0 = { quad_node.bottom_left, quad_node.length / 2, 0, {-1, -1, -1, -1} };
		quad_node.sub_node[0] = buildNodeList(sub_node_0);

		QuadNode sub_node_1 = { quad_node.bottom_left + D3DXVECTOR2(quad_node.length / 2, 0), quad_node.length / 2, 0, {-1, -1, -1, -1} };
		quad_node.sub_node[1] = buildNodeList(sub_node_1);

		QuadNode sub_node_2 = { quad_node.bottom_left + D3DXVECTOR2(quad_node.length / 2, quad_node.length / 2), quad_node.length / 2, 0, {-1, -1, -1, -1} };
		quad_node.sub_node[2] = buildNodeList(sub_node_2);

		QuadNode sub_node_3 = { quad_node.bottom_left + D3DXVECTOR2(0, quad_node.length / 2), quad_node.length / 2, 0, {-1, -1, -1, -1} };
		quad_node.sub_node[3] = buildNodeList(sub_node_3);

		visible = !isLeaf(quad_node);
	}

	if (visible)
	{
		// Estimate mesh LOD
		int lod = 0;
		for (lod = 0; lod < g_Lods - 1; lod++)
		{
			if (min_coverage > g_UpperGridCoverage)
				break;
			min_coverage *= 4;
		}

		// We don't use 1x1 and 2x2 patch. So the highest level is g_Lods - 2.
		quad_node.lod = min(lod, g_Lods - 2);
	}
	else
		return -1;

	// Insert into the list
	int position = (int)g_render_list.size();
	g_render_list.push_back(quad_node);

	return position;
}

