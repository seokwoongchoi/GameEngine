#include "Framework.h"
#include "Island11.h"
#include "Viewer/Fixity.h"

extern bool g_RenderWireframe;
extern bool g_RenderCaustics;
#define PI 3.14159265358979323846f
int gp_wrap(int a)
{
	if (a < 0) return (a + terrain_gridpoints);
	if (a >= terrain_gridpoints) return (a - terrain_gridpoints);
	return a;
}


Island11::Island11(Shader* shader)
	:shader(shader)
{


	viewBuffer = new ConstantBuffer(&viewDesc, sizeof(ViewDesc));
	sViewBuffer = shader->AsConstantBuffer("CB_ViewMatrix");

	lightBuffer = new ConstantBuffer(&lightDesc, sizeof(LightDesc));
	sLightBuffer = shader->AsConstantBuffer("CB_LightMatrix");


	reflectionCam = new Fixity();
	rock_bump_texture = nullptr;
	rock_bump_textureSRV = nullptr;

	rock_microbump_texture = nullptr;
	rock_microbump_textureSRV = nullptr;

	rock_diffuse_texture = nullptr;
	rock_diffuse_textureSRV = nullptr;

	sand_bump_texture = nullptr;
	sand_bump_textureSRV = nullptr;

	sand_microbump_texture = nullptr;
	sand_microbump_textureSRV = nullptr;

	sand_diffuse_texture = nullptr;
	sand_diffuse_textureSRV = nullptr;

	grass_diffuse_texture = nullptr;
	grass_diffuse_textureSRV = nullptr;

	slope_diffuse_texture = nullptr;
	slope_diffuse_textureSRV = nullptr;

	water_bump_texture = nullptr;
	water_bump_textureSRV = nullptr;

	//sky_texture = nullptr;
	//sky_textureSRV = nullptr;

	reflection_color_resource = nullptr;
	reflection_color_resourceSRV = nullptr;
	reflection_color_resourceRTV = nullptr;

	refraction_color_resource = nullptr;
	refraction_color_resourceSRV = nullptr;
	refraction_color_resourceRTV = nullptr;

	shadowmap_resource = nullptr;
	shadowmap_resourceSRV = nullptr;
	shadowmap_resourceDSV = nullptr;

	reflection_depth_resource = nullptr;
	reflection_depth_resourceDSV = nullptr;


	refraction_depth_resource = nullptr;
	refraction_depth_resourceRTV = nullptr;
	refraction_depth_resourceSRV = nullptr;

	water_normalmap_resource = nullptr;
	water_normalmap_resourceSRV = nullptr;
	water_normalmap_resourceRTV = nullptr;

	main_color_resource = nullptr;
	main_color_resourceSRV = nullptr;
	main_color_resourceRTV = nullptr;
	main_depth_resource = nullptr;
	main_depth_resourceDSV = nullptr;
	main_depth_resourceSRV = nullptr;
	main_color_resource_resolved = nullptr;
	main_color_resource_resolvedSRV=nullptr;
	
	//shader = new Shader(L"Deferred/Island11.fx");
	//Context::Get()->SetShader(shader);
	

	CreateTerrain();
	LoadTextures();
	BackbufferWidth = 1280.0f;
	BackbufferHeight = 720.0f;
	ReCreateBuffers();

//	depthStencil = new DepthStencil(static_cast<uint>(BackbufferWidth), static_cast<uint>(BackbufferHeight));

	shader->AsSRV("g_HeightfieldTexture")->SetResource(heightmap_textureSRV);

	shader->AsSRV("g_LayerdefTexture")->SetResource(layerdef_textureSRV);
	shader->AsSRV("g_RockBumpTexture")->SetResource(rock_bump_textureSRV);

	shader->AsSRV("g_RockMicroBumpTexture")->SetResource(rock_microbump_textureSRV);

	shader->AsSRV("g_RockDiffuseTexture")->SetResource(rock_diffuse_textureSRV);

	shader->AsSRV("g_SandBumpTexture")->SetResource(sand_bump_textureSRV);

	shader->AsSRV("g_SandMicroBumpTexture")->SetResource(sand_microbump_textureSRV);

	shader->AsSRV("g_SandDiffuseTexture")->SetResource(sand_diffuse_textureSRV);

	shader->AsSRV("g_GrassDiffuseTexture")->SetResource(grass_diffuse_textureSRV);

	shader->AsSRV("g_SlopeDiffuseTexture")->SetResource(slope_diffuse_textureSRV);

	shader->AsSRV("g_WaterBumpTexture")->SetResource(water_bump_textureSRV);

	//shader->AsSRV("g_SkyTexture")->SetResource(sky_textureSRV);

	shader->AsSRV("g_DepthMapTexture")->SetResource(depthmap_textureSRV);

	shader->AsVector("g_HeightFieldOrigin")->SetFloatVector(origin);
	shader->AsScalar("g_HeightFieldSize")->SetFloat(terrain_gridpoints*terrain_geometry_scale);



	reflection_Viewport.Width = (float)BackbufferWidth*reflection_buffer_size_multiplier;
	reflection_Viewport.Height = (float)BackbufferHeight*reflection_buffer_size_multiplier;
	reflection_Viewport.MaxDepth = 1;
	reflection_Viewport.MinDepth = 0;
	reflection_Viewport.TopLeftX = 0;
	reflection_Viewport.TopLeftY = 0;

	

	main_Viewport.Width = (float)BackbufferWidth*main_buffer_size_multiplier;
	main_Viewport.Height = (float)BackbufferHeight*main_buffer_size_multiplier;
	main_Viewport.MaxDepth = 1;
	main_Viewport.MinDepth = 0;
	main_Viewport.TopLeftX = 0;
	main_Viewport.TopLeftY = 0;

	

	water_normalmap_resource_viewport.Width = water_normalmap_resource_buffer_size_xy;
	water_normalmap_resource_viewport.Height = water_normalmap_resource_buffer_size_xy;
	water_normalmap_resource_viewport.MaxDepth = 1;
	water_normalmap_resource_viewport.MinDepth = 0;
	water_normalmap_resource_viewport.TopLeftX = 0;
	water_normalmap_resource_viewport.TopLeftY = 0;
}

Island11::~Island11()
{
	SafeRelease(heightmap_texture);
	SafeRelease(heightmap_textureSRV);

	SafeRelease(rock_bump_texture);
	SafeRelease(rock_bump_textureSRV);

	SafeRelease(rock_microbump_texture);
	SafeRelease(rock_microbump_textureSRV);

	SafeRelease(rock_diffuse_texture);
	SafeRelease(rock_diffuse_textureSRV);


	SafeRelease(sand_bump_texture);
	SafeRelease(sand_bump_textureSRV);

	SafeRelease(sand_microbump_texture);
	SafeRelease(sand_microbump_textureSRV);

	SafeRelease(sand_diffuse_texture);
	SafeRelease(sand_diffuse_textureSRV);

	SafeRelease(slope_diffuse_texture);
	SafeRelease(slope_diffuse_textureSRV);

	SafeRelease(grass_diffuse_texture);
	SafeRelease(grass_diffuse_textureSRV);

	SafeRelease(layerdef_texture);
	SafeRelease(layerdef_textureSRV);

	SafeRelease(water_bump_texture);
	SafeRelease(water_bump_textureSRV);

	SafeRelease(depthmap_texture);
	SafeRelease(depthmap_textureSRV);
/*
	SafeRelease(sky_texture);
	SafeRelease(sky_textureSRV);

*/

	SafeRelease(main_color_resource);
	SafeRelease(main_color_resourceSRV);
	SafeRelease(main_color_resourceRTV);

	SafeRelease(main_color_resource_resolved);
	SafeRelease(main_color_resource_resolvedSRV);

	SafeRelease(main_depth_resource);
	SafeRelease(main_depth_resourceDSV);
	SafeRelease(main_depth_resourceSRV);

	SafeRelease(reflection_color_resource);
	SafeRelease(reflection_color_resourceSRV);
	SafeRelease(reflection_color_resourceRTV);
	SafeRelease(refraction_color_resource);
	SafeRelease(refraction_color_resourceSRV);
	SafeRelease(refraction_color_resourceRTV);

	SafeRelease(reflection_depth_resource);
	SafeRelease(reflection_depth_resourceDSV);
	SafeRelease(refraction_depth_resource);
	SafeRelease(refraction_depth_resourceRTV);
	SafeRelease(refraction_depth_resourceSRV);

	SafeRelease(shadowmap_resource);
	SafeRelease(shadowmap_resourceDSV);
	SafeRelease(shadowmap_resourceSRV);

	//SafeRelease(sky_vertexbuffer);
	//SafeRelease(trianglestrip_inputlayout);

	SafeRelease(heightfield_vertexbuffer);
	//SafeRelease(heightfield_inputlayout);

	SafeRelease(water_normalmap_resource);
	SafeRelease(water_normalmap_resourceSRV);
	SafeRelease(water_normalmap_resourceRTV);
}



void Island11::ReCreateBuffers()
{
	D3D11_TEXTURE2D_DESC tex_desc;
	D3D11_SHADER_RESOURCE_VIEW_DESC textureSRV_desc;
	D3D11_DEPTH_STENCIL_VIEW_DESC DSV_desc;



	SafeRelease(main_color_resource);
	SafeRelease(main_color_resourceSRV);
	SafeRelease(main_color_resourceRTV);

	SafeRelease(main_color_resource_resolved);
	SafeRelease(main_color_resource_resolvedSRV);

	SafeRelease(main_depth_resource);
	SafeRelease(main_depth_resourceDSV);
	SafeRelease(main_depth_resourceSRV);

	SafeRelease(reflection_color_resource);
	SafeRelease(reflection_color_resourceSRV);
	SafeRelease(reflection_color_resourceRTV);
	SafeRelease(refraction_color_resource);
	SafeRelease(refraction_color_resourceSRV);
	SafeRelease(refraction_color_resourceRTV);

	SafeRelease(reflection_depth_resource);
	SafeRelease(reflection_depth_resourceDSV);
	SafeRelease(refraction_depth_resource);
	SafeRelease(refraction_depth_resourceSRV);
	SafeRelease(refraction_depth_resourceRTV);

	SafeRelease(shadowmap_resource);
	SafeRelease(shadowmap_resourceDSV);
	SafeRelease(shadowmap_resourceSRV);

	SafeRelease(water_normalmap_resource);
	SafeRelease(water_normalmap_resourceSRV);
	SafeRelease(water_normalmap_resourceRTV);

	// recreating main color buffer

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	ZeroMemory(&tex_desc, sizeof(tex_desc));

	tex_desc.Width = (UINT)(BackbufferWidth*main_buffer_size_multiplier);
	tex_desc.Height = (UINT)(BackbufferHeight*main_buffer_size_multiplier);
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = MultiSampleCount;
	tex_desc.SampleDesc.Quality = MultiSampleQuality;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	textureSRV_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	Check(D3D::GetDevice()->CreateTexture2D(&tex_desc, NULL, &main_color_resource));
    Check(D3D::GetDevice()->CreateShaderResourceView(main_color_resource, &textureSRV_desc, &main_color_resourceSRV));
	Check(D3D::GetDevice()->CreateRenderTargetView(main_color_resource, NULL, &main_color_resourceRTV));


	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	ZeroMemory(&tex_desc, sizeof(tex_desc));

	tex_desc.Width = (UINT)(BackbufferWidth*main_buffer_size_multiplier);
	tex_desc.Height = (UINT)(BackbufferHeight*main_buffer_size_multiplier);
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	textureSRV_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	Check(D3D::GetDevice()->CreateTexture2D(&tex_desc, NULL, &main_color_resource_resolved));
	Check(D3D::GetDevice()->CreateShaderResourceView(main_color_resource_resolved, &textureSRV_desc, &main_color_resource_resolvedSRV));

	// recreating main depth buffer

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	ZeroMemory(&tex_desc, sizeof(tex_desc));

	tex_desc.Width = (UINT)(BackbufferWidth*main_buffer_size_multiplier);
	tex_desc.Height = (UINT)(BackbufferHeight*main_buffer_size_multiplier);
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R32_TYPELESS;
	tex_desc.SampleDesc.Count = MultiSampleCount;
	tex_desc.SampleDesc.Quality = MultiSampleQuality;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	DSV_desc.Format = DXGI_FORMAT_D32_FLOAT;
	DSV_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	DSV_desc.Flags = 0;
	DSV_desc.Texture2D.MipSlice = 0;

	textureSRV_desc.Format = DXGI_FORMAT_R32_FLOAT;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	textureSRV_desc.Texture2D.MipLevels = 1;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	Check(D3D::GetDevice()->CreateTexture2D(&tex_desc, NULL, &main_depth_resource));
	Check(D3D::GetDevice()->CreateDepthStencilView(main_depth_resource, &DSV_desc, &main_depth_resourceDSV));
	Check(D3D::GetDevice()->CreateShaderResourceView(main_depth_resource, &textureSRV_desc, &main_depth_resourceSRV));

	// recreating reflection and refraction color buffers

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	ZeroMemory(&tex_desc, sizeof(tex_desc));

	tex_desc.Width = (UINT)(BackbufferWidth*reflection_buffer_size_multiplier);
	tex_desc.Height = (UINT)(BackbufferHeight*reflection_buffer_size_multiplier);
	tex_desc.MipLevels = (UINT)max(1, log(max((float)tex_desc.Width, (float)tex_desc.Height)) / (float)log(2.0f));
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	textureSRV_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	Check(D3D::GetDevice()->CreateTexture2D(&tex_desc, NULL, &reflection_color_resource));
	Check(D3D::GetDevice()->CreateShaderResourceView(reflection_color_resource, &textureSRV_desc, &reflection_color_resourceSRV));
	Check(D3D::GetDevice()->CreateRenderTargetView(reflection_color_resource, NULL, &reflection_color_resourceRTV));


	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	ZeroMemory(&tex_desc, sizeof(tex_desc));

	tex_desc.Width = (UINT)(BackbufferWidth*refraction_buffer_size_multiplier);
	tex_desc.Height = (UINT)(BackbufferHeight*refraction_buffer_size_multiplier);
	tex_desc.MipLevels = (UINT)max(1, log(max((float)tex_desc.Width, (float)tex_desc.Height)) / (float)log(2.0f));
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	textureSRV_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	Check(D3D::GetDevice()->CreateTexture2D(&tex_desc, NULL, &refraction_color_resource));
	Check(D3D::GetDevice()->CreateShaderResourceView(refraction_color_resource, &textureSRV_desc, &refraction_color_resourceSRV));
	Check(D3D::GetDevice()->CreateRenderTargetView(refraction_color_resource, NULL, &refraction_color_resourceRTV));

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	ZeroMemory(&tex_desc, sizeof(tex_desc));

	// recreating reflection and refraction depth buffers

	tex_desc.Width = (UINT)(BackbufferWidth*reflection_buffer_size_multiplier);
	tex_desc.Height = (UINT)(BackbufferHeight*reflection_buffer_size_multiplier);
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R32_TYPELESS;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	DSV_desc.Format = DXGI_FORMAT_D32_FLOAT;
	DSV_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSV_desc.Flags = 0;
	DSV_desc.Texture2D.MipSlice = 0;

	Check(D3D::GetDevice()->CreateTexture2D(&tex_desc, NULL, &reflection_depth_resource));
	Check(D3D::GetDevice()->CreateDepthStencilView(reflection_depth_resource, &DSV_desc, &reflection_depth_resourceDSV));


	tex_desc.Width = (UINT)(BackbufferWidth*refraction_buffer_size_multiplier);
	tex_desc.Height = (UINT)(BackbufferHeight*refraction_buffer_size_multiplier);
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R32_FLOAT;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	textureSRV_desc.Format = DXGI_FORMAT_R32_FLOAT;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = 1;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	Check(D3D::GetDevice()->CreateTexture2D(&tex_desc, NULL, &refraction_depth_resource));
	Check(D3D::GetDevice()->CreateRenderTargetView(refraction_depth_resource, NULL, &refraction_depth_resourceRTV));
	Check(D3D::GetDevice()->CreateShaderResourceView(refraction_depth_resource, &textureSRV_desc, &refraction_depth_resourceSRV));

	// recreating shadowmap resource
	tex_desc.Width = shadowmap_resource_buffer_size_xy;
	tex_desc.Height = shadowmap_resource_buffer_size_xy;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R32_TYPELESS;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	DSV_desc.Format = DXGI_FORMAT_D32_FLOAT;
	DSV_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSV_desc.Flags = 0;
	DSV_desc.Texture2D.MipSlice = 0;

	textureSRV_desc.Format = DXGI_FORMAT_R32_FLOAT;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = 1;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	Check(D3D::GetDevice()->CreateTexture2D(&tex_desc, NULL, &shadowmap_resource));
	Check(D3D::GetDevice()->CreateShaderResourceView(shadowmap_resource, &textureSRV_desc, &shadowmap_resourceSRV));
	Check(D3D::GetDevice()->CreateDepthStencilView(shadowmap_resource, &DSV_desc, &shadowmap_resourceDSV));

	// recreating water normalmap buffer

	tex_desc.Width = water_normalmap_resource_buffer_size_xy;
	tex_desc.Height = water_normalmap_resource_buffer_size_xy;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	textureSRV_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;
	
	Check(D3D::GetDevice()->CreateTexture2D(&tex_desc, NULL, &water_normalmap_resource));
	Check(D3D::GetDevice()->CreateShaderResourceView(water_normalmap_resource, &textureSRV_desc, &water_normalmap_resourceSRV));
	Check(D3D::GetDevice()->CreateRenderTargetView(water_normalmap_resource, NULL, &water_normalmap_resourceRTV));


	
}

void Island11::LoadTextures()
{

	D3D11_SHADER_RESOURCE_VIEW_DESC textureSRV_desc;

	
	// Load images
	D3DX11_IMAGE_LOAD_INFO imageLoadInfo;
	D3DX11_IMAGE_INFO imageInfo;
	wstring path= L"../../_Textures/TerrainTextures/rock_bump6.dds";

	//DXUTFindDXSDKMediaFileCch(path.c_str(), MAX_PATH, L"TerrainTextures/rock_bump6.dds");
	D3DX11GetImageInfoFromFile(path.c_str(), NULL, &imageInfo, NULL);
	memset(&imageLoadInfo, 0, sizeof(imageLoadInfo));
	imageLoadInfo.Width = imageInfo.Width;
	imageLoadInfo.Height = imageInfo.Height;
	imageLoadInfo.MipLevels = imageInfo.MipLevels;
	imageLoadInfo.Format = imageInfo.Format;
	imageLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3DX11CreateTextureFromFile(D3D::GetDevice(), path.c_str(), &imageLoadInfo, NULL, (ID3D11Resource**)&rock_bump_texture, NULL);

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = imageLoadInfo.Format;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = imageLoadInfo.MipLevels;
	D3D::GetDevice()->CreateShaderResourceView(rock_bump_texture, &textureSRV_desc, &rock_bump_textureSRV);

	path = L"../../_Textures/TerrainTextures/terrain_rock4.dds";
	//DXUTFindDXSDKMediaFileCch(path.c_str(), MAX_PATH, L"TerrainTextures/terrain_rock4.dds");
	D3DX11GetImageInfoFromFile(path.c_str(), NULL, &imageInfo, NULL);
	memset(&imageLoadInfo, 0, sizeof(imageLoadInfo));
	imageLoadInfo.Width = imageInfo.Width;
	imageLoadInfo.Height = imageInfo.Height;
	imageLoadInfo.MipLevels = imageInfo.MipLevels;
	imageLoadInfo.Format = imageInfo.Format;
	imageLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3DX11CreateTextureFromFile(D3D::Get()->GetDevice(), path.c_str(), &imageLoadInfo, NULL, (ID3D11Resource**)&rock_diffuse_texture, NULL);

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = imageLoadInfo.Format;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = imageLoadInfo.MipLevels;
	D3D::Get()->GetDevice()->CreateShaderResourceView(rock_diffuse_texture, &textureSRV_desc, &rock_diffuse_textureSRV);

	path = L"../../_Textures/TerrainTextures/sand_diffuse.dds";
	//DXUTFindDXSDKMediaFileCch(path.c_str(), MAX_PATH, L"TerrainTextures/sand_diffuse.dds");
	D3DX11GetImageInfoFromFile(path.c_str(), NULL, &imageInfo, NULL);
	memset(&imageLoadInfo, 0, sizeof(imageLoadInfo));
	imageLoadInfo.Width = imageInfo.Width;
	imageLoadInfo.Height = imageInfo.Height;
	imageLoadInfo.MipLevels = imageInfo.MipLevels;
	imageLoadInfo.Format = imageInfo.Format;
	imageLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3DX11CreateTextureFromFile(D3D::Get()->GetDevice(), path.c_str(), &imageLoadInfo, NULL, (ID3D11Resource**)&sand_diffuse_texture, NULL);

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = imageLoadInfo.Format;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = imageLoadInfo.MipLevels;
	D3D::Get()->GetDevice()->CreateShaderResourceView(sand_diffuse_texture, &textureSRV_desc, &sand_diffuse_textureSRV);

	path = L"../../_Textures/TerrainTextures/rock_bump4.dds";
	//DXUTFindDXSDKMediaFileCch(path.c_str(), MAX_PATH, L"TerrainTextures/rock_bump4.dds");
	D3DX11GetImageInfoFromFile(path.c_str(), NULL, &imageInfo, NULL);
	memset(&imageLoadInfo, 0, sizeof(imageLoadInfo));
	imageLoadInfo.Width = imageInfo.Width;
	imageLoadInfo.Height = imageInfo.Height;
	imageLoadInfo.MipLevels = imageInfo.MipLevels;
	imageLoadInfo.Format = imageInfo.Format;
	imageLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3DX11CreateTextureFromFile(D3D::Get()->GetDevice(), path.c_str(), &imageLoadInfo, NULL, (ID3D11Resource**)&sand_bump_texture, NULL);

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = imageLoadInfo.Format;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = imageLoadInfo.MipLevels;
	D3D::Get()->GetDevice()->CreateShaderResourceView(sand_bump_texture, &textureSRV_desc, &sand_bump_textureSRV);

	path = L"../../_Textures/TerrainTextures/terrain_grass.dds";
	//DXUTFindDXSDKMediaFileCch(path.c_str(), MAX_PATH, L"TerrainTextures/terrain_grass.dds");
	D3DX11GetImageInfoFromFile(path.c_str(), NULL, &imageInfo, NULL);
	memset(&imageLoadInfo, 0, sizeof(imageLoadInfo));
	imageLoadInfo.Width = imageInfo.Width;
	imageLoadInfo.Height = imageInfo.Height;
	imageLoadInfo.MipLevels = imageInfo.MipLevels;
	imageLoadInfo.Format = imageInfo.Format;
	imageLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3DX11CreateTextureFromFile(D3D::Get()->GetDevice(), path.c_str(), &imageLoadInfo, NULL, (ID3D11Resource**)&grass_diffuse_texture, NULL);

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = imageLoadInfo.Format;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = imageLoadInfo.MipLevels;
	D3D::Get()->GetDevice()->CreateShaderResourceView(grass_diffuse_texture, &textureSRV_desc, &grass_diffuse_textureSRV);


	path = L"../../_Textures/TerrainTextures/terrain_slope.dds";
	//DXUTFindDXSDKMediaFileCch(path.c_str(), MAX_PATH, L"TerrainTextures/terrain_slope.dds");
	D3DX11GetImageInfoFromFile(path.c_str(), NULL, &imageInfo, NULL);
	memset(&imageLoadInfo, 0, sizeof(imageLoadInfo));
	imageLoadInfo.Width = imageInfo.Width;
	imageLoadInfo.Height = imageInfo.Height;
	imageLoadInfo.MipLevels = imageInfo.MipLevels;
	imageLoadInfo.Format = imageInfo.Format;
	imageLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3DX11CreateTextureFromFile(D3D::Get()->GetDevice(), path.c_str(), &imageLoadInfo, NULL, (ID3D11Resource**)&slope_diffuse_texture, NULL);

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = imageLoadInfo.Format;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = imageLoadInfo.MipLevels;
	D3D::Get()->GetDevice()->CreateShaderResourceView(slope_diffuse_texture, &textureSRV_desc, &slope_diffuse_textureSRV);

	path = L"../../_Textures/TerrainTextures/lichen1_normal.dds";
	//DXUTFindDXSDKMediaFileCch(path.c_str(), MAX_PATH, L"TerrainTextures/lichen1_normal.dds");
	D3DX11GetImageInfoFromFile(path.c_str(), NULL, &imageInfo, NULL);
	memset(&imageLoadInfo, 0, sizeof(imageLoadInfo));
	imageLoadInfo.Width = imageInfo.Width;
	imageLoadInfo.Height = imageInfo.Height;
	imageLoadInfo.MipLevels = imageInfo.MipLevels;
	imageLoadInfo.Format = imageInfo.Format;
	imageLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3DX11CreateTextureFromFile(D3D::Get()->GetDevice(), path.c_str(), &imageLoadInfo, NULL, (ID3D11Resource**)&sand_microbump_texture, NULL);

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = imageLoadInfo.Format;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = imageLoadInfo.MipLevels;
	D3D::Get()->GetDevice()->CreateShaderResourceView(sand_microbump_texture, &textureSRV_desc, &sand_microbump_textureSRV);


	path = L"../../_Textures/TerrainTextures/rock_bump4.dds";
	//DXUTFindDXSDKMediaFileCch(path.c_str(), MAX_PATH, L"TerrainTextures/rock_bump4.dds");
	D3DX11GetImageInfoFromFile(path.c_str(), NULL, &imageInfo, NULL);
	memset(&imageLoadInfo, 0, sizeof(imageLoadInfo));
	imageLoadInfo.Width = imageInfo.Width;
	imageLoadInfo.Height = imageInfo.Height;
	imageLoadInfo.MipLevels = imageInfo.MipLevels;
	imageLoadInfo.Format = imageInfo.Format;
	imageLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3DX11CreateTextureFromFile(D3D::Get()->GetDevice(), path.c_str(), &imageLoadInfo, NULL, (ID3D11Resource**)&rock_microbump_texture, NULL);

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = imageLoadInfo.Format;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = imageLoadInfo.MipLevels;
	D3D::Get()->GetDevice()->CreateShaderResourceView(rock_microbump_texture, &textureSRV_desc, &rock_microbump_textureSRV);

	path = L"../../_Textures/TerrainTextures/water_bump.dds";
	//DXUTFindDXSDKMediaFileCch(path.c_str(), MAX_PATH, L"TerrainTextures/water_bump.dds");
	D3DX11GetImageInfoFromFile(path.c_str(), NULL, &imageInfo, NULL);
	memset(&imageLoadInfo, 0, sizeof(imageLoadInfo));
	imageLoadInfo.Width = imageInfo.Width;
	imageLoadInfo.Height = imageInfo.Height;
	imageLoadInfo.MipLevels = imageInfo.MipLevels;
	imageLoadInfo.Format = imageInfo.Format;
	imageLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3DX11CreateTextureFromFile(D3D::Get()->GetDevice(), path.c_str(), &imageLoadInfo, NULL, (ID3D11Resource**)&water_bump_texture, NULL);

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = imageLoadInfo.Format;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = imageLoadInfo.MipLevels;
	D3D::Get()->GetDevice()->CreateShaderResourceView(water_bump_texture, &textureSRV_desc, &water_bump_textureSRV);

//	path = L"../../_Textures/TerrainTextures/sky.dds";
//	//DXUTFindDXSDKMediaFileCch(path.c_str(), MAX_PATH, L"TerrainTextures/sky.dds");
//	D3DX11GetImageInfoFromFile(path.c_str(), NULL, &imageInfo, NULL);
//	memset(&imageLoadInfo, 0, sizeof(imageLoadInfo));
//	imageLoadInfo.Width = imageInfo.Width;
//	imageLoadInfo.Height = imageInfo.Height;
//	imageLoadInfo.MipLevels = imageInfo.MipLevels;
//	imageLoadInfo.Format = imageInfo.Format;
//	imageLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
//	D3DX11CreateTextureFromFile(D3D::Get()->GetDevice(), path.c_str(), &imageLoadInfo, NULL, (ID3D11Resource**)&sky_texture, NULL);
//
//	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
//	textureSRV_desc.Format = imageLoadInfo.Format;
//	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
//	textureSRV_desc.Texture2D.MipLevels = imageLoadInfo.MipLevels;
//	D3D::Get()->GetDevice()->CreateShaderResourceView(sky_texture, &textureSRV_desc, &sky_textureSRV);
}

void Island11::Reflection()
{
	

	


	
	// selecting shadowmap_resource rendertarget

	 //D3D::GetDC()->RSSetViewports(1, &shadowmap_resource_viewport);
	 //D3D::GetDC()->OMSetRenderTargets(0, NULL, shadowmap_resourceDSV);
	 //D3D::GetDC()->ClearDepthStencilView(shadowmap_resourceDSV, D3D11_CLEAR_DEPTH, 1.0, 0);

	//drawing terrain to depth buffer
   // SetupLightView();
	/*shader->AsScalar("g_TerrainBeingRendered")->SetFloat(1.0f);
	shader->AsScalar("g_SkipCausticsCalculation")->SetFloat(1.0f);
	 D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
	stride = sizeof(float) * 4;
	 D3D::GetDC()->IASetVertexBuffers(0, 1, &heightfield_vertexbuffer, &stride, &offset);
	shader->Draw(0,17, terrain_numpatches_1d*terrain_numpatches_1d, 0);*/
	//ImGui::Checkbox("RenderCaustics", &g_RenderCaustics);
	if (g_RenderCaustics)
	{
		// selecting water_normalmap_resource rendertarget
		 D3D::GetDC()->RSSetViewports(1, &water_normalmap_resource_viewport);
		 D3D::GetDC()->OMSetRenderTargets(1, &water_normalmap_resourceRTV, NULL);
		 D3D::GetDC()->ClearRenderTargetView(water_normalmap_resourceRTV, ClearColor);

		//rendering water normalmap
		SetupNormalView(); // need it just to provide shader with camera position
		 D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		stride = sizeof(float) * 6;
		 D3D::GetDC()->IASetVertexBuffers(0, 1, &heightfield_vertexbuffer, &stride, &offset);
		shader->Draw(0, 18, 4, 0);
	}



	// setting up reflection rendertarget	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	 D3D::GetDC()->RSSetViewports(1, &reflection_Viewport);
	 D3D::GetDC()->OMSetRenderTargets(1, &reflection_color_resourceRTV, reflection_depth_resourceDSV);
	 D3D::GetDC()->ClearRenderTargetView(reflection_color_resourceRTV, RefractionClearColor);
	 D3D::GetDC()->ClearDepthStencilView(reflection_depth_resourceDSV, D3D11_CLEAR_DEPTH, 1.0, 0);
	SetupReflectionView();
	shader->AsSRV("g_DepthTexture")->SetResource(shadowmap_resourceSRV);
	shader->AsScalar("g_SkipCausticsCalculation")->SetFloat(1.0f);
	 D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
	stride = sizeof(float) * 4;
	 D3D::GetDC()->IASetVertexBuffers(0, 1, &heightfield_vertexbuffer, &stride, &offset);
	shader->Draw(0, 19, terrain_numpatches_1d*terrain_numpatches_1d, 0);
		
}

void Island11::Update()
{

	ImGui::InputFloat("halfSpaceCullPosition", (float*)&halfSpaceCullPosition);
	ImGui::InputFloat("CullSign", (float*)&cullSign);
	g_TotalTime += Time::Get()->Delta();
	//g_FrameTime = Time::Get()->Get()->FPS();

	//float viewpoints_slide_speed_factor = 0.01f;
	//float viewpoints_slide_speed_damp = 0.97f;
	//float scaled_time = (1.0f + g_TotalTime / 25.0f);
	//int viewpoint_index = (int)(scaled_time) % 6;
	//D3DXVECTOR3 predicted_camera_position;
	//float dh;

	

	/*D3D11_VIEWPORT defaultViewport;
	UINT viewPortsNum = 1;
	D3D::GetDC()->RSGetViewports(&viewPortsNum, &defaultViewport);*/


	D3DXVECTOR2 WaterTexcoordShift(g_TotalTime*1.5f, g_TotalTime*0.75f);
	D3DXVECTOR2 ScreenSizeInv(1.0f / (g_BackBufferWidth*main_buffer_size_multiplier), 1.0f / (g_BackBufferHeight*main_buffer_size_multiplier));

	/*shader->AsScalar("g_ZNear")->SetFloat(scene_z_near);
	shader->AsScalar("g_ZFar")->SetFloat(scene_z_far);*/
	
	shader->AsVector("g_LightPosition")->SetFloatVector(Context::Get()->LightPosition());
	shader->AsVector("g_WaterBumpTexcoordShift")->SetFloatVector(WaterTexcoordShift);
	shader->AsVector("g_ScreenSizeInv")->SetFloatVector(ScreenSizeInv);

	
	shader->AsScalar("g_RenderCaustics")->SetFloat(g_RenderCaustics ? 1.0f : 0.0f);

	camera = Context::Get()->GetCamera();
}

void Island11::Terrain()
{
	SetupNormalView();

	shader->AsSRV("g_WaterNormalMapTexture")->SetResource(water_normalmap_resourceSRV);
	// drawing terrain to main buffer
	//shader->AsSRV("g_DepthTexture")->SetResource(shadowmap_resourceSRV);
	shader->AsScalar("g_TerrainBeingRendered")->SetFloat(1.0f);
	shader->AsScalar("g_SkipCausticsCalculation")->SetFloat(0.0f);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
	stride = sizeof(float) * 4;
	D3D::GetDC()->IASetVertexBuffers(0, 1, &heightfield_vertexbuffer, &stride, &offset);
	shader->Draw(0, 20, terrain_numpatches_1d*terrain_numpatches_1d, 0);
}

void Island11::Water()
{
	//D3D::GetDC()->RSSetViewports(1, &main_Viewport);
	//D3D::GetDC()->OMSetRenderTargets(1, &rtv, dsv);
	//D3D::GetDC()->ClearRenderTargetView(rtv, ClearColor);
	//D3D::GetDC()->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0, 0);
	

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	// drawing water surface to main buffer
	shader->AsSRV("g_DepthTexture")->SetResource(shadowmap_resourceSRV);
	shader->AsSRV("g_ReflectionTexture")->SetResource(reflection_color_resourceSRV);
	shader->AsSRV("g_RefractionTexture")->SetResource(refraction_color_resourceSRV);
	shader->AsSRV("g_RefractionDepthTextureResolved")->SetResource(refraction_depth_resourceSRV);
	shader->AsSRV("g_WaterNormalMapTexture")->SetResource(water_normalmap_resourceSRV);
	shader->AsScalar("g_TerrainBeingRendered")->SetFloat(0.0f);


	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
	stride = sizeof(float) * 4;
	D3D::GetDC()->IASetVertexBuffers(0, 1, &heightfield_vertexbuffer, &stride, &offset);
	
	shader->Draw(0, 21, terrain_numpatches_1d*terrain_numpatches_1d, 0);



}


void Island11::Refraction(ID3D11ShaderResourceView* srv, ID3D11Texture2D* texture)
{

	
	//SetupRefractionView();
	// resolving main buffer color to refraction color resource
	D3D::GetDC()->ResolveSubresource(refraction_color_resource, 0, texture, 0, DXGI_FORMAT_R8G8B8A8_UNORM);

	// resolving main buffer depth to refraction depth resource manually
	D3D::GetDC()->RSSetViewports(1, &main_Viewport);
	D3D::GetDC()->OMSetRenderTargets(1, &refraction_depth_resourceRTV, nullptr);
	D3D::GetDC()->ClearRenderTargetView(refraction_depth_resourceRTV, Color(1, 1, 1, 1));
	

	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	shader->AsSRV("g_RefractionDepthTextureMS1")->SetResource(srv);
	stride = sizeof(float) * 6;
	D3D::GetDC()->IASetVertexBuffers(0, 1, &heightfield_vertexbuffer, &stride, &offset);

	shader->Draw(0, 23, 4, 0);

}

void Island11::FinalPass()
{

	//D3D::GetDC()->OMSetRenderTargets(1, &colorBuffer, backBuffer);
	//D3D::GetDC()->RSSetViewports(1, &currentViewport);

//resolving main buffer 
D3D::GetDC()->ResolveSubresource(main_color_resource_resolved, 0, main_color_resource, 0, DXGI_FORMAT_R8G8B8A8_UNORM);

//drawing main buffer to back 
shader->AsSRV("g_MainTexture")->SetResource(main_color_resource_resolvedSRV);
//shader->AsScalar("g_MainBufferSizeMultiplier")->SetFloat(main_buffer_size_multiplier);


D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//pEffect->GetTechniqueByName("MainToBackBuffer")->GetPassByIndex(0)->Apply(0,  D3D::GetDC());
stride = sizeof(float) * 6;
D3D::GetDC()->IASetVertexBuffers(0, 1, &heightfield_vertexbuffer, &stride, &offset);
shader->Draw(0,22, 4, 0); // just need to pass 4 vertices to shader

shader->AsSRV("g_MainTexture")->SetResource(NULL);

//
//SafeRelease(colorBuffer);
//SafeRelease(backBuffer);


}



void Island11::CreateTerrain()
{
	int i, j, k, l;
	float x, z;
	int ix, iz;
	float * backterrain;
	D3DXVECTOR3 vec1, vec2, vec3;
	int currentstep = terrain_gridpoints;
	float mv, rm;
	float offset = 0, yscale = 0, maxheight = 0, minheight = 0;

	float *height_linear_array;
	float *patches_rawdata;
	HRESULT result;
	D3D11_SUBRESOURCE_DATA subresource_data;
	D3D11_TEXTURE2D_DESC tex_desc;
	D3D11_SHADER_RESOURCE_VIEW_DESC textureSRV_desc;

	backterrain = (float *)malloc((terrain_gridpoints + 1)*(terrain_gridpoints + 1) * sizeof(float));
	rm = terrain_fractalinitialvalue;
	backterrain[0] = 0;
	backterrain[0 + terrain_gridpoints * terrain_gridpoints] = 0;
	backterrain[terrain_gridpoints] = 0;
	backterrain[terrain_gridpoints + terrain_gridpoints * terrain_gridpoints] = 0;
	currentstep = terrain_gridpoints;
	srand(12);

	// generating fractal terrain using square-diamond method
	while (currentstep > 1)
	{
		//square step;
		i = 0;
		j = 0;


		while (i < terrain_gridpoints)
		{
			j = 0;
			while (j < terrain_gridpoints)
			{

				mv = backterrain[i + terrain_gridpoints * j];
				mv += backterrain[(i + currentstep) + terrain_gridpoints * j];
				mv += backterrain[(i + currentstep) + terrain_gridpoints * (j + currentstep)];
				mv += backterrain[i + terrain_gridpoints * (j + currentstep)];
				mv /= 4.0;
				backterrain[i + currentstep / 2 + terrain_gridpoints * (j + currentstep / 2)] = (float)(mv + rm * ((rand() % 1000) / 1000.0f - 0.5f));
				j += currentstep;
			}
			i += currentstep;
		}

		//diamond step;
		i = 0;
		j = 0;

		while (i < terrain_gridpoints)
		{
			j = 0;
			while (j < terrain_gridpoints)
			{

				mv = 0;
				mv = backterrain[i + terrain_gridpoints * j];
				mv += backterrain[(i + currentstep) + terrain_gridpoints * j];
				mv += backterrain[(i + currentstep / 2) + terrain_gridpoints * (j + currentstep / 2)];
				mv += backterrain[i + currentstep / 2 + terrain_gridpoints * gp_wrap(j - currentstep / 2)];
				mv /= 4;
				backterrain[i + currentstep / 2 + terrain_gridpoints * j] = (float)(mv + rm * ((rand() % 1000) / 1000.0f - 0.5f));

				mv = 0;
				mv = backterrain[i + terrain_gridpoints * j];
				mv += backterrain[i + terrain_gridpoints * (j + currentstep)];
				mv += backterrain[(i + currentstep / 2) + terrain_gridpoints * (j + currentstep / 2)];
				mv += backterrain[gp_wrap(i - currentstep / 2) + terrain_gridpoints * (j + currentstep / 2)];
				mv /= 4;
				backterrain[i + terrain_gridpoints * (j + currentstep / 2)] = (float)(mv + rm * ((rand() % 1000) / 1000.0f - 0.5f));

				mv = 0;
				mv = backterrain[i + currentstep + terrain_gridpoints * j];
				mv += backterrain[i + currentstep + terrain_gridpoints * (j + currentstep)];
				mv += backterrain[(i + currentstep / 2) + terrain_gridpoints * (j + currentstep / 2)];
				mv += backterrain[gp_wrap(i + currentstep / 2 + currentstep) + terrain_gridpoints * (j + currentstep / 2)];
				mv /= 4;
				backterrain[i + currentstep + terrain_gridpoints * (j + currentstep / 2)] = (float)(mv + rm * ((rand() % 1000) / 1000.0f - 0.5f));

				mv = 0;
				mv = backterrain[i + currentstep + terrain_gridpoints * (j + currentstep)];
				mv += backterrain[i + terrain_gridpoints * (j + currentstep)];
				mv += backterrain[(i + currentstep / 2) + terrain_gridpoints * (j + currentstep / 2)];
				mv += backterrain[i + currentstep / 2 + terrain_gridpoints * gp_wrap(j + currentstep / 2 + currentstep)];
				mv /= 4;
				backterrain[i + currentstep / 2 + terrain_gridpoints * (j + currentstep)] = (float)(mv + rm * ((rand() % 1000) / 1000.0f - 0.5f));
				j += currentstep;
			}
			i += currentstep;
		}
		//changing current step;
		currentstep /= 2;
		rm *= terrain_fractalfactor;
	}

	// scaling to minheight..maxheight range
	for (i = 0; i < terrain_gridpoints + 1; i++)
		for (j = 0; j < terrain_gridpoints + 1; j++)
		{
			height[i][j] = backterrain[i + terrain_gridpoints * j];
		}
	maxheight = height[0][0];
	minheight = height[0][0];
	for (i = 0; i < terrain_gridpoints + 1; i++)
		for (j = 0; j < terrain_gridpoints + 1; j++)
		{
			if (height[i][j] > maxheight) maxheight = height[i][j];
			if (height[i][j] < minheight) minheight = height[i][j];
		}
	offset = minheight - terrain_minheight;
	yscale = (terrain_maxheight - terrain_minheight) / (maxheight - minheight);

	for (i = 0; i < terrain_gridpoints + 1; i++)
		for (j = 0; j < terrain_gridpoints + 1; j++)
		{
			height[i][j] -= minheight;
			height[i][j] *= yscale;
			height[i][j] += terrain_minheight;
		}

	// moving down edges of heightmap	
	for (i = 0; i < terrain_gridpoints + 1; i++)
		for (j = 0; j < terrain_gridpoints + 1; j++)
		{
			mv = (float)((i - terrain_gridpoints / 2.0f)*(i - terrain_gridpoints / 2.0f) + (j - terrain_gridpoints / 2.0f)*(j - terrain_gridpoints / 2.0f));
			rm = (float)((terrain_gridpoints*0.8f)*(terrain_gridpoints*0.8f) / 4.0f);
			if (mv > rm)
			{
				height[i][j] -= ((mv - rm) / 1000.0f)*terrain_geometry_scale;
			}
			if (height[i][j] < terrain_minheight)
			{
				height[i][j] = terrain_minheight;
			}
		}


	// terrain banks
	for (k = 0; k < 10; k++)
	{
		for (i = 0; i < terrain_gridpoints + 1; i++)
			for (j = 0; j < terrain_gridpoints + 1; j++)
			{
				mv = height[i][j];
				if ((mv) > 0.02f)
				{
					mv -= 0.02f;
				}
				if (mv < -0.02f)
				{
					mv += 0.02f;
				}
				height[i][j] = mv;
			}
	}

	// smoothing 
	for (k = 0; k < terrain_smoothsteps; k++)
	{
		for (i = 0; i < terrain_gridpoints + 1; i++)
			for (j = 0; j < terrain_gridpoints + 1; j++)
			{

				vec1.x = 2 * terrain_geometry_scale;
				vec1.y = terrain_geometry_scale * (height[gp_wrap(i + 1)][j] - height[gp_wrap(i - 1)][j]);
				vec1.z = 0;
				vec2.x = 0;
				vec2.y = -terrain_geometry_scale * (height[i][gp_wrap(j + 1)] - height[i][gp_wrap(j - 1)]);
				vec2.z = -2 * terrain_geometry_scale;

				D3DXVec3Cross(&vec3, &vec1, &vec2);
				D3DXVec3Normalize(&vec3, &vec3);


				if (((vec3.y > terrain_rockfactor) || (height[i][j] < 1.2f)))
				{
					rm = terrain_smoothfactor1;
					mv = height[i][j] * (1.0f - rm) + rm * 0.25f*(height[gp_wrap(i - 1)][j] + height[i][gp_wrap(j - 1)] + height[gp_wrap(i + 1)][j] + height[i][gp_wrap(j + 1)]);
					backterrain[i + terrain_gridpoints * j] = mv;
				}
				else
				{
					rm = terrain_smoothfactor2;
					mv = height[i][j] * (1.0f - rm) + rm * 0.25f*(height[gp_wrap(i - 1)][j] + height[i][gp_wrap(j - 1)] + height[gp_wrap(i + 1)][j] + height[i][gp_wrap(j + 1)]);
					backterrain[i + terrain_gridpoints * j] = mv;
				}

			}
		for (i = 0; i < terrain_gridpoints + 1; i++)
			for (j = 0; j < terrain_gridpoints + 1; j++)
			{
				height[i][j] = (backterrain[i + terrain_gridpoints * j]);
			}
	}
	for (i = 0; i < terrain_gridpoints + 1; i++)
		for (j = 0; j < terrain_gridpoints + 1; j++)
		{
			rm = 0.5f;
			mv = height[i][j] * (1.0f - rm) + rm * 0.25f*(height[gp_wrap(i - 1)][j] + height[i][gp_wrap(j - 1)] + height[gp_wrap(i + 1)][j] + height[i][gp_wrap(j + 1)]);
			backterrain[i + terrain_gridpoints * j] = mv;
		}
	for (i = 0; i < terrain_gridpoints + 1; i++)
		for (j = 0; j < terrain_gridpoints + 1; j++)
		{
			height[i][j] = (backterrain[i + terrain_gridpoints * j]);
		}


	free(backterrain);

	//calculating normals
	for (i = 0; i < terrain_gridpoints + 1; i++)
		for (j = 0; j < terrain_gridpoints + 1; j++)
		{
			vec1.x = 2 * terrain_geometry_scale;
			vec1.y = terrain_geometry_scale * (height[gp_wrap(i + 1)][j] - height[gp_wrap(i - 1)][j]);
			vec1.z = 0;
			vec2.x = 0;
			vec2.y = -terrain_geometry_scale * (height[i][gp_wrap(j + 1)] - height[i][gp_wrap(j - 1)]);
			vec2.z = -2 * terrain_geometry_scale;
			D3DXVec3Cross(&normal[i][j], &vec1, &vec2);
			D3DXVec3Normalize(&normal[i][j], &normal[i][j]);
		}


	// buiding layerdef 
	byte* temp_layerdef_map_texture_pixels = (byte *)malloc(terrain_layerdef_map_texture_size*terrain_layerdef_map_texture_size * 4);
	byte* layerdef_map_texture_pixels = (byte *)malloc(terrain_layerdef_map_texture_size*terrain_layerdef_map_texture_size * 4);
	for (i = 0; i < terrain_layerdef_map_texture_size; i++)
		for (j = 0; j < terrain_layerdef_map_texture_size; j++)
		{
			x = (float)(terrain_gridpoints)*((float)i / (float)terrain_layerdef_map_texture_size);
			z = (float)(terrain_gridpoints)*((float)j / (float)terrain_layerdef_map_texture_size);
			ix = (int)floor(x);
			iz = (int)floor(z);
			rm = bilinear_interpolation(x - ix, z - iz, height[ix][iz], height[ix + 1][iz], height[ix + 1][iz + 1], height[ix][iz + 1])*terrain_geometry_scale;

			temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 0] = 0;
			temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 1] = 0;
			temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 2] = 0;
			temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 3] = 0;

			if ((rm > terrain_height_underwater_start) && (rm <= terrain_height_underwater_end))
			{
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 0] = 255;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 1] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 2] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 3] = 0;
			}

			if ((rm > terrain_height_sand_start) && (rm <= terrain_height_sand_end))
			{
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 0] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 1] = 255;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 2] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 3] = 0;
			}

			if ((rm > terrain_height_grass_start) && (rm <= terrain_height_grass_end))
			{
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 0] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 1] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 2] = 255;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 3] = 0;
			}

			mv = bilinear_interpolation(x - ix, z - iz, normal[ix][iz].y, normal[ix + 1][iz].y, normal[ix + 1][iz + 1].y, normal[ix][iz + 1].y);

			if ((mv < terrain_slope_grass_start) && (rm > terrain_height_sand_end))
			{
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 0] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 1] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 2] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 3] = 0;
			}

			if ((mv < terrain_slope_rocks_start) && (rm > terrain_height_rocks_start))
			{
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 0] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 1] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 2] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 3] = 255;
			}

		}
	for (i = 0; i < terrain_layerdef_map_texture_size; i++)
		for (j = 0; j < terrain_layerdef_map_texture_size; j++)
		{
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 0] = temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 0];
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 1] = temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 1];
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 2] = temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 2];
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 3] = temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 3];
		}


	for (i = 2; i < terrain_layerdef_map_texture_size - 2; i++)
		for (j = 2; j < terrain_layerdef_map_texture_size - 2; j++)
		{
			int n1 = 0;
			int n2 = 0;
			int n3 = 0;
			int n4 = 0;
			for (k = -2; k < 3; k++)
				for (l = -2; l < 3; l++)
				{
					n1 += temp_layerdef_map_texture_pixels[((j + k)*terrain_layerdef_map_texture_size + i + l) * 4 + 0];
					n2 += temp_layerdef_map_texture_pixels[((j + k)*terrain_layerdef_map_texture_size + i + l) * 4 + 1];
					n3 += temp_layerdef_map_texture_pixels[((j + k)*terrain_layerdef_map_texture_size + i + l) * 4 + 2];
					n4 += temp_layerdef_map_texture_pixels[((j + k)*terrain_layerdef_map_texture_size + i + l) * 4 + 3];
				}
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 0] = (byte)(n1 / 25);
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 1] = (byte)(n2 / 25);
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 2] = (byte)(n3 / 25);
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 3] = (byte)(n4 / 25);
		}

	// putting the generated data to textures

	subresource_data.pSysMem = layerdef_map_texture_pixels;
	subresource_data.SysMemPitch = terrain_layerdef_map_texture_size * 4;
	subresource_data.SysMemSlicePitch = 0;

	tex_desc.Width = terrain_layerdef_map_texture_size;
	tex_desc.Height = terrain_layerdef_map_texture_size;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;
	Check(D3D::Get()->GetDevice()->CreateTexture2D(&tex_desc, &subresource_data, &layerdef_texture));

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = tex_desc.Format;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;
	Check(D3D::Get()->GetDevice()->CreateShaderResourceView(layerdef_texture, &textureSRV_desc, &layerdef_textureSRV));

	free(temp_layerdef_map_texture_pixels);
	free(layerdef_map_texture_pixels);

	height_linear_array = new float[terrain_gridpoints*terrain_gridpoints * 4];
	patches_rawdata = new float[terrain_numpatches_1d*terrain_numpatches_1d * 4];

	for (int i = 0; i < terrain_gridpoints; i++)
		for (int j = 0; j < terrain_gridpoints; j++)
		{
			height_linear_array[(i + j * terrain_gridpoints) * 4 + 0] = normal[i][j].x;
			height_linear_array[(i + j * terrain_gridpoints) * 4 + 1] = normal[i][j].y;
			height_linear_array[(i + j * terrain_gridpoints) * 4 + 2] = normal[i][j].z;
			height_linear_array[(i + j * terrain_gridpoints) * 4 + 3] = height[i][j];
		}
	subresource_data.pSysMem = height_linear_array;
	subresource_data.SysMemPitch = terrain_gridpoints * 4 * sizeof(float);
	subresource_data.SysMemSlicePitch = 0;

	tex_desc.Width = terrain_gridpoints;
	tex_desc.Height = terrain_gridpoints;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;
	Check(D3D::Get()->GetDevice()->CreateTexture2D(&tex_desc, &subresource_data, &heightmap_texture));

	free(height_linear_array);

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = tex_desc.Format;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	Check(D3D::Get()->GetDevice()->CreateShaderResourceView(heightmap_texture, &textureSRV_desc, &heightmap_textureSRV));

	//building depthmap
	byte * depth_shadow_map_texture_pixels = (byte *)malloc(terrain_depth_shadow_map_texture_size*terrain_depth_shadow_map_texture_size * 4);
	for (i = 0; i < terrain_depth_shadow_map_texture_size; i++)
		for (j = 0; j < terrain_depth_shadow_map_texture_size; j++)
		{
			x = (float)(terrain_gridpoints)*((float)i / (float)terrain_depth_shadow_map_texture_size);
			z = (float)(terrain_gridpoints)*((float)j / (float)terrain_depth_shadow_map_texture_size);
			ix = (int)floor(x);
			iz = (int)floor(z);
			rm = bilinear_interpolation(x - ix, z - iz, height[ix][iz], height[ix + 1][iz], height[ix + 1][iz + 1], height[ix][iz + 1])*terrain_geometry_scale;

			if (rm > 0)
			{
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size + i) * 4 + 0] = 0;
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size + i) * 4 + 1] = 0;
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size + i) * 4 + 2] = 0;
			}
			else
			{
				float no = (1.0f*255.0f*(rm / (terrain_minheight*terrain_geometry_scale))) - 1.0f;
				if (no > 255) no = 255;
				if (no < 0) no = 0;
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size + i) * 4 + 0] = (byte)no;

				no = (10.0f*255.0f*(rm / (terrain_minheight*terrain_geometry_scale))) - 40.0f;
				if (no > 255) no = 255;
				if (no < 0) no = 0;
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size + i) * 4 + 1] = (byte)no;

				no = (100.0f*255.0f*(rm / (terrain_minheight*terrain_geometry_scale))) - 300.0f;
				if (no > 255) no = 255;
				if (no < 0) no = 0;
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size + i) * 4 + 2] = (byte)no;
			}
			depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size + i) * 4 + 3] = 0;
		}

	subresource_data.pSysMem = depth_shadow_map_texture_pixels;
	subresource_data.SysMemPitch = terrain_depth_shadow_map_texture_size * 4;
	subresource_data.SysMemSlicePitch = 0;

	tex_desc.Width = terrain_depth_shadow_map_texture_size;
	tex_desc.Height = terrain_depth_shadow_map_texture_size;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;
	Check(D3D::Get()->GetDevice()->CreateTexture2D(&tex_desc, &subresource_data, &depthmap_texture));

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = tex_desc.Format;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	Check(D3D::Get()->GetDevice()->CreateShaderResourceView(depthmap_texture, &textureSRV_desc, &depthmap_textureSRV));

	free(depth_shadow_map_texture_pixels);

	// creating terrain vertex buffer
	for (int i = 0; i < terrain_numpatches_1d; i++)
		for (int j = 0; j < terrain_numpatches_1d; j++)
		{
			patches_rawdata[(i + j * terrain_numpatches_1d) * 4 + 0] = i * terrain_geometry_scale*terrain_gridpoints / terrain_numpatches_1d;
			patches_rawdata[(i + j * terrain_numpatches_1d) * 4 + 1] = j * terrain_geometry_scale*terrain_gridpoints / terrain_numpatches_1d;
			patches_rawdata[(i + j * terrain_numpatches_1d) * 4 + 2] = terrain_geometry_scale * terrain_gridpoints / terrain_numpatches_1d;
			patches_rawdata[(i + j * terrain_numpatches_1d) * 4 + 3] = terrain_geometry_scale * terrain_gridpoints / terrain_numpatches_1d;
		}

	D3D11_BUFFER_DESC buf_desc;
	memset(&buf_desc, 0, sizeof(buf_desc));

	buf_desc.ByteWidth = terrain_numpatches_1d * terrain_numpatches_1d * 4 * sizeof(float);
	buf_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buf_desc.Usage = D3D11_USAGE_DEFAULT;

	subresource_data.pSysMem = patches_rawdata;
	subresource_data.SysMemPitch = 0;
	subresource_data.SysMemSlicePitch = 0;
	
	Check(D3D::Get()->GetDevice()->CreateBuffer(&buf_desc, &subresource_data, &heightfield_vertexbuffer));
	free(patches_rawdata);
	
}

void Island11::SetupNormalView()
{
	
	
	view = Context::Get()->View();
	proj = Context::Get()->Projection();

	viewDesc.View = view;
	viewDesc.VP = view * proj;
	D3DXMatrixInverse(&viewDesc.VPInv, NULL, &viewDesc.VP);
	camera->Position(&viewDesc.camPos);
	direction = camera->Forward();
	D3DXVec3Normalize(&viewDesc.camDirection, &direction);

	viewDesc.halfSpaceCullSign = cullSign;
	
	viewDesc.halfSpaceCullPosition = terrain_minheight* halfSpaceCullPosition;
	viewBuffer->Apply();
	sViewBuffer->SetConstantBuffer(viewBuffer->Buffer());

		


}

void Island11::SetupReflectionView()
{
	
	Context::Get()->GetCamera()->Position(&position);
	

	Context::Get()->GetCamera()->Position(&EyePoint);
	LookAtPoint = Context::Get()->GetCamera()->GetLookAtPt();
	EyePoint.y = -1.0f*EyePoint.y + 1.0f;
	LookAtPoint.y = -1.0f*LookAtPoint.y + 1.0f;

	camera->GetMatrix(&view);
	view._41 = 0; view._42 = 0; view._43 = 0;
	D3DXMatrixTranspose(&view, &view);
	view._41 = position.x;
	view._42 = position.y;

	view._42 = -view._42 - 1.0f;

	view._43 = position.z;

	view._21 *= -1.0f;
	view._23 *= -1.0f;

	view._32 *= -1.0f;
	D3DXMatrixInverse(&rflectionMatrix, nullptr, &view);

	/*rotation.x *= -1.0;
	reflectionCam->Rotation(rotation);
	position.y = -position.y + 1.0f;
	reflectionCam->Position(position);
	
	reflectionCam->GetMatrix(&rflectionMatrix);*/

	


	D3DXMatrixIdentity(&viewDesc.View);
	viewDesc.VP = rflectionMatrix * proj;
	D3DXMatrixIdentity(&viewDesc.VPInv);
	viewDesc.camPos= EyePoint;
	direction = LookAtPoint - EyePoint;
	D3DXVec3Normalize(&viewDesc.camDirection, &direction);
	viewDesc.halfSpaceCullSign = 1.0f;
	viewDesc.halfSpaceCullPosition = -0.6;
	viewBuffer->Apply();
	sViewBuffer->SetConstantBuffer(viewBuffer->Buffer());

	Context::Get()->PushViewMatrix(rflectionMatrix);

}

void Island11::SetupRefractionView()
{
	shader->AsScalar("g_HalfSpaceCullSign")->SetFloat(-1.0f);
	shader->AsScalar("g_HalfSpaceCullPosition")->SetFloat(terrain_minheight);
	
}

void Island11::SetupLightView()
{ 

 //    EyePoint = Context::Get()->LightPosition();
	////D3DXVECTOR3 LookAtPoint = D3DXVECTOR3((terrain_far_range / 2.0f), 0.0f,- (terrain_far_range / 2.0f));
 //  LookAtPoint = EyePoint + (10000* terrain_geometry_scale) * Context::Get()->LightDirection();
	//
 //  cameraPosition;
	//camera->Position(&cameraPosition);
	//
	//
	//nr = sqrt(EyePoint.x*EyePoint.x + EyePoint.y*EyePoint.y + EyePoint.z*EyePoint.z) - terrain_far_range * 0.7f;
	//fr = sqrt(EyePoint.x*EyePoint.x + EyePoint.y*EyePoint.y + EyePoint.z*EyePoint.z) + terrain_far_range * 0.7f;

	// view = *D3DXMatrixLookAtLH(&view, &EyePoint, &LookAtPoint, &lookUp);
	// proj = *D3DXMatrixOrthoLH(&proj, terrain_far_range*1.5, terrain_far_range, nr, fr);
	//lightDesc.VP = view * proj;
	//D3DXMatrixInverse(&lightDesc.VPInv, NULL, &lightDesc.VP);

	//shader->AsMatrix("g_LightModelViewProjectionMatrix")->SetMatrix(lightDesc.VP);
	//shader->AsMatrix("g_LightModelViewProjectionMatrixInv")->SetMatrix(lightDesc.VPInv);


	//shader->AsMatrix("g_ModelViewProjectionMatrix")->SetMatrix(lightDesc.VP);
	//shader->AsVector("g_CameraPosition")->SetFloatVector(cameraPosition);
	//D3DXVECTOR3 direction = camera->Forward();
	//D3DXVECTOR3 normalized_direction = *D3DXVec3Normalize(&normalized_direction, &direction);
	//shader->AsVector("g_CameraDirection")->SetFloatVector(normalized_direction);


	//shader->AsScalar("g_HalfSpaceCullSign")->SetFloat(1.0);
	//shader->AsScalar("g_HalfSpaceCullPosition")->SetFloat(terrain_minheight * 2);
}

float bilinear_interpolation(float fx, float fy, float a, float b, float c, float d)
{
	float s1, s2, s3, s4;
	s1 = fx * fy;
	s2 = (1 - fx)*fy;
	s3 = (1 - fx)*(1 - fy);
	s4 = fx * (1 - fy);
	return((a*s3 + b * s4 + c * s1 + d * s2));
}
