#include "Framework.h"
#include "OceanSimulator.h"
#include "CS/OceanFFT.h"

#define HALF_SQRT_2	0.7071068f
#define GRAV_ACCEL	981.0f	// The acceleration of gravity, cm/s^2

#define BLOCK_SIZE_X 16
#define BLOCK_SIZE_Y 16


OceanSimulator::OceanSimulator()
{
	
	csShader = new Shader(L"Deferred/ocean_simulator_cs.fxo");
	shader= new Shader(L"Deferred/ocean_simulator_vs_ps.fxo");
	params.dmap_dim = 64;
	// The side length (world space) of square patch
	params.patch_length = 3;
	// Adjust this parameter to control the simulation speed
	params.time_scale = 0.8f;
	// A scale to control the amplitude. Not the world space height
	params.wave_amplitude =0.35f;
	// 2D wind direction. No need to be normalized
	params.wind_dir = D3DXVECTOR2(0.8f, 0.6f);
	// The bigger the wind speed, the larger scale of wave crest.
	// But the wave scale can be no larger than patch_length
	params.wind_speed =100.0f;
	// Damp out the components opposite to wind direction.
	// The smaller the value, the higher wind dependency
	params.wind_dependency = 0.07f;
	// Control the scale of horizontal movement. Higher value creates
	params.choppy_scale =1.3f;

	int heightmapSize = (params.dmap_dim + 4)*(params.dmap_dim + 1);
	Vector2* h0data = new Vector2[heightmapSize * sizeof(Vector2)];
	float* omegadata = new float[heightmapSize * sizeof(float)];
	initHeightMap(params, h0data, omegadata);
	
	int hmap = params.dmap_dim;
	int inputFullSize = (hmap + 4)*(hmap + 1);
	int inputHalfSize = hmap * hmap;
	int outputSize = hmap * hmap;

	char* zeroData = new char[3 * outputSize * sizeof(float) * 2];
	memset(zeroData, 0, 3 * outputSize * sizeof(float) * 2);

	uint float2_stride = 2 * sizeof(float);
	CreateBufferAndUAV(h0data, inputFullSize*float2_stride, float2_stride, &m_pBuffer_Float2_H0, &m_pUAV_H0, &m_pSRV_H0);
	
	CreateBufferAndUAV(zeroData, 3 * inputHalfSize * float2_stride, float2_stride, &m_pBuffer_Float2_Ht, &m_pUAV_Ht, &m_pSRV_Ht);

	// omega

	CreateBufferAndUAV(omegadata, inputFullSize * sizeof(float), sizeof(float), &m_pBuffer_Float_Omega, &m_pUAV_Omega, &m_pSRV_Omega);
	CreateBufferAndUAV( zeroData, 3 * outputSize * float2_stride, float2_stride, &m_pBuffer_Float_Dxyz, &m_pUAV_Dxyz, &m_pSRV_Dxyz);
	
	SafeDeleteArray(zeroData);
	SafeDeleteArray(h0data);
	SafeDeleteArray(omegadata);

	CreateTextureAndViews(hmap, hmap, DXGI_FORMAT_R32G32B32A32_FLOAT, &m_pDisplacementMap, &m_pDisplacementSRV, &m_pDisplacementRTV);
	CreateTextureAndViews(hmap, hmap, DXGI_FORMAT_R16G16B16A16_FLOAT, &m_pGradientMap, &m_pGradientSRV, &m_pGradientRTV);
	

	//// Samplers
	//D3D11_SAMPLER_DESC sam_desc1;
	//sam_desc1.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	//sam_desc1.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	//sam_desc1.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	//sam_desc1.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//sam_desc1.MipLODBias = 0;
	//sam_desc1.MaxAnisotropy = 1;
	//sam_desc1.ComparisonFunc = D3D11_COMPARISON_NEVER;
	//sam_desc1.BorderColor[0] = 1.0f;
	//sam_desc1.BorderColor[1] = 1.0f;
	//sam_desc1.BorderColor[2] = 1.0f;
	//sam_desc1.BorderColor[3] = 1.0f;
	//sam_desc1.MinLOD = -FLT_MAX;
	//sam_desc1.MaxLOD = FLT_MAX;
	//Check(D3D::GetDevice()->CreateSamplerState(&sam_desc1, &m_pPointSamplerState));
	//


	//csShader->AsSampler("LinearSampler")->SetSampler(0, m_pPointSamplerState);


	Vertex vertices[4];
	/*vertices[0].Position = Vector3(-1.0f, -1.0f, 0.0f);
	vertices[1].Position = Vector3(-1.0f, 1.0f, 0.0f );
	vertices[2].Position = Vector3(+1.0f, -1.0f, 0.0f);
	vertices[3].Position = Vector3(+1.0f, +1.0f, 0.0f);*/
	vertices[0].Position = Vector3(1.0f, -1.0f, 0.0f);
	vertices[1].Position = Vector3(1.0f, 1.0f, 0.0f);
	vertices[2].Position = Vector3(-1.0f, 1.0f, 0.0f);
	vertices[3].Position = Vector3(-1.0f, -1.0f, 0.0f); 

	//float quad_verts[] =
	//{
	//	-1, -1, 0, 1,
	//	-1,  1, 0, 1,
	//	 1, -1, 0, 1,
	//	 1,  1, 0, 1,
	//};
	//
	

	vertexBuffer = new VertexBuffer(vertices, 4, sizeof(Vertex));

	uint indexCount = 6;
	uint* indices = new uint[indexCount]
	{
		0,1,2,2,1,3
	};
	indexBuffer = new IndexBuffer(indices, indexCount);
	
	
	immutableBuffer = new ConstantBuffer(&immutableCB, sizeof(CB_Immutable));
	sImmutableBufferCS = csShader->AsConstantBuffer("CB_Immutable");
	sImmutableBuffer = shader->AsConstantBuffer("CB_Immutable");
	immutableCB.actualDim = params.dmap_dim;
	immutableCB.inputWidth = immutableCB.actualDim + 4;
	immutableCB.outputWidth = immutableCB.actualDim;
	immutableCB.outputHeight = immutableCB.actualDim;
	immutableCB.dtxOffset = immutableCB.actualDim*immutableCB.actualDim;
	immutableCB.dtyOffset = immutableCB.actualDim*immutableCB.actualDim * 2;
	
	

	perframeBuffer = new ConstantBuffer(&perframeCB, sizeof(CB_PerFrame));
	sPerframeBufferCS = csShader->AsConstantBuffer("CB_Perframe");
	sPerframeBuffer = shader->AsConstantBuffer("CB_Perframe");
	
	FFT = new OceanFFT(10);
	

}

OceanSimulator::~OceanSimulator()
{
}

void OceanSimulator::updateDisplacementMap(float time)
{
	csShader->AsSRV("g_InputH0")->SetResource(m_pSRV_H0);
	csShader->AsSRV("g_InputOmega")->SetResource(m_pSRV_Omega);
	csShader->AsUAV("g_OutputHt")->SetUnorderedAccessView(m_pUAV_Ht);

	perframeCB.Time = time * params.time_scale;
	perframeCB.ChoppyScale = params.choppy_scale;
	perframeCB.GridLen = params.dmap_dim / params.patch_length;


	if (sPerframeBufferCS)
	{

		perframeBuffer->Apply();
		sPerframeBufferCS->SetConstantBuffer(perframeBuffer->Buffer());
	}
	if (sImmutableBufferCS)
	{

		immutableBuffer->Apply();
		sImmutableBufferCS->SetConstantBuffer(immutableBuffer->Buffer());
	}
	UINT group_count_x = (params.dmap_dim + BLOCK_SIZE_X - 1) / BLOCK_SIZE_X;
	UINT group_count_y = (params.dmap_dim + BLOCK_SIZE_Y - 1) / BLOCK_SIZE_Y;
	
	csShader->Dispatch(0,0,group_count_x, group_count_y, 1);

	//FFT->fft512x512Update(m_pUAV_Dxyz, m_pSRV_Dxyz, m_pSRV_Ht);
	//FFT->fft_512x512_c2c(m_pUAV_Dxyz, m_pSRV_Dxyz, m_pSRV_Ht);
	
	// Push RT
	ID3D11RenderTargetView* old_target;
	ID3D11DepthStencilView* old_depth;
	D3D::GetDC()->OMGetRenderTargets(1, &old_target, &old_depth);
	D3D11_VIEWPORT old_viewport;
	UINT num_viewport = 1;
	D3D::GetDC()->RSGetViewports(&num_viewport, &old_viewport);

	D3D11_VIEWPORT new_vp = { 0, 0, D3D::Get()->Width(), D3D::Get()->Height(), 0.0f, 1.0f };
	D3D::GetDC()->RSSetViewports(1, &new_vp);

	// Set RT
	//ID3D11RenderTargetView* rt_views[1] = { m_pDisplacementRTV };
	D3D::GetDC()->OMSetRenderTargets(1, &m_pDisplacementRTV, nullptr);
	D3D::Get()->Clear(Color(0, 0, 0, 1), m_pDisplacementRTV, nullptr);
	

	if (sPerframeBuffer)
	{

		perframeBuffer->Apply();
		sPerframeBuffer->SetConstantBuffer(perframeBuffer->Buffer());
	}
	
	if (sImmutableBuffer)
	{

		immutableBuffer->Apply();
		sImmutableBuffer->SetConstantBuffer(immutableBuffer->Buffer());
	}
	// Buffer resourcesS
	
	shader->AsSRV("g_InputDxyz")->SetResource(m_pSRV_Dxyz);


	vertexBuffer->Render();
	//indexBuffer->Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// Perform draw call
	shader->Draw(0,0,4);

	


	// ----------------------------------- Generate Normal ----------------------------------------
	// Set RT
	/*rt_views[0] = m_pGradientRTV;
	D3D::GetDC()->OMSetRenderTargets(1, rt_views, NULL);*/

	D3D::GetDC()->OMSetRenderTargets(1, &m_pGradientRTV, nullptr);
	D3D::Get()->Clear(Color(0, 0, 0, 1), m_pGradientRTV, nullptr);


	// Texture resource and sampler
	shader->AsSRV("g_samplerDisplacementMap")->SetResource(m_pDisplacementSRV);
	//vertexBuffer->Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	// Perform draw call
	shader->Draw(0,1,4);

	

	//// Pop RT
	D3D::GetDC()->RSSetViewports(1, &old_viewport);
	D3D::GetDC()->OMSetRenderTargets(1, &old_target, old_depth);
	SafeRelease(old_target);
	SafeRelease(old_depth);

	D3D::GetDC()->GenerateMips(m_pGradientSRV);

}

void OceanSimulator::initHeightMap(OceanParameter & params, D3DXVECTOR2 * out_h0, float * out_omega)
{
	int i, j;
	D3DXVECTOR2 K, Kn;

	D3DXVECTOR2 wind_dir;
	D3DXVec2Normalize(&wind_dir, &params.wind_dir);
	float a = params.wave_amplitude * 1e-7f;	// It is too small. We must scale it for editing.
	float v = params.wind_speed;
	float dir_depend = params.wind_dependency;

	int height_map_dim = params.dmap_dim;
	float patch_length = params.patch_length;

	// initialize random generator.
	srand(0);

	for (i = 0; i <= height_map_dim; i++)
	{
		// K is wave-vector, range [-|DX/W, |DX/W], [-|DY/H, |DY/H]
		K.y = static_cast<float>((-height_map_dim / 2.0f + i) * (2 * D3DX_PI / patch_length));

		for (j = 0; j <= height_map_dim; j++)
		{
			K.x = static_cast<float>((-height_map_dim / 2.0f + j) * (2 * D3DX_PI / patch_length));

			float phil = (K.x == 0 && K.y == 0) ? 0 : sqrtf(Phillips(K, wind_dir, v, a, dir_depend));

			out_h0[i * (height_map_dim + 4) + j].x = float(phil * Gauss() * HALF_SQRT_2);
			out_h0[i * (height_map_dim + 4) + j].y = float(phil * Gauss() * HALF_SQRT_2);

			// The angular frequency is following the dispersion relation:
			//            out_omega^2 = g*k
			// The equation of Gerstner wave:
			//            x = x0 - K/k * A * sin(dot(K, x0) - sqrt(g * k) * t), x is a 2D vector.
			//            z = A * cos(dot(K, x0) - sqrt(g * k) * t)
			// Gerstner wave shows that a point on a simple sinusoid wave is doing a uniform circular
			// motion with the center (x0, y0, z0), radius A, and the circular plane is parallel to
			// vector K.
			out_omega[i * (height_map_dim + 4) + j] = sqrtf(GRAV_ACCEL * sqrtf(K.x * K.x + K.y * K.y));
		}
	}
}

void OceanSimulator::CreateBufferAndUAV(void * data, uint byteWidth, uint byteStride, ID3D11Buffer ** ppBuffer, ID3D11UnorderedAccessView ** ppUAV, ID3D11ShaderResourceView ** ppSRV)
{
	// Create buffer
	D3D11_BUFFER_DESC buf_desc;
	buf_desc.ByteWidth = byteWidth;
	buf_desc.Usage = D3D11_USAGE_DEFAULT;
	buf_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	buf_desc.CPUAccessFlags = 0;
	buf_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	buf_desc.StructureByteStride = byteStride;
		
	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = data;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	Check(D3D::GetDevice()->CreateBuffer(&buf_desc, data != NULL ? &init_data : NULL, ppBuffer));
	//Check(D3D::GetDevice()->CreateBuffer(&buf_desc, nullptr, ppBuffer));
	// Create undordered access view
	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
	uav_desc.Format = DXGI_FORMAT_UNKNOWN;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uav_desc.Buffer.FirstElement = 0;
	uav_desc.Buffer.NumElements = byteWidth / byteStride;
	uav_desc.Buffer.Flags = 0;

	Check(D3D::GetDevice()->CreateUnorderedAccessView(*ppBuffer, &uav_desc, ppUAV));
	// Create shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	srv_desc.Format = DXGI_FORMAT_UNKNOWN;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srv_desc.Buffer.FirstElement = 0;
	srv_desc.Buffer.NumElements = byteWidth / byteStride;

	Check(D3D::GetDevice()->CreateShaderResourceView(*ppBuffer, &srv_desc, ppSRV));
}

void OceanSimulator::CreateTextureAndViews(uint width, uint height, DXGI_FORMAT format, ID3D11Texture2D ** ppTex, ID3D11ShaderResourceView ** ppSRV, ID3D11RenderTargetView ** ppRTV)
{
	// Create 2D texture
	D3D11_TEXTURE2D_DESC tex_desc;
	tex_desc.Width = width;
	tex_desc.Height = height;
	tex_desc.MipLevels = 0;
	tex_desc.ArraySize = 1;
	tex_desc.Format = format;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	Check(D3D::GetDevice()->CreateTexture2D(&tex_desc, NULL, ppTex));

	// Create shader resource view
	(*ppTex)->GetDesc(&tex_desc);
	if (ppSRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
		srv_desc.Format = format;
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels = tex_desc.MipLevels;
		srv_desc.Texture2D.MostDetailedMip = 0;

		Check(D3D::GetDevice()->CreateShaderResourceView(*ppTex, &srv_desc, ppSRV));
		
	}

	// Create render target view
	if (ppRTV)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
		rtv_desc.Format = format;
		rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtv_desc.Texture2D.MipSlice = 0;

		Check(D3D::GetDevice()->CreateRenderTargetView(*ppTex, &rtv_desc, ppRTV));
		
	}
}

const OceanParameter & OceanSimulator::getParameters()
{
	return params;
}



float OceanSimulator::Gauss()
{
	float u1 = rand() / (float)RAND_MAX;
	float u2 = rand() / (float)RAND_MAX;
	if (u1 < 1e-6f)
		u1 = 1e-6f;
	return sqrtf(-2 * logf(u1)) * cosf(2 * D3DX_PI * u2);
}

float OceanSimulator::Phillips(Vector2 K, Vector2 W, float v, float a, float dir_depend)
{
	// largest possible wave from constant wind of velocity v
	float l = v * v / GRAV_ACCEL;
	// damp out waves with very small length w << l
	float w = l / 1000;

	float Ksqr = K.x * K.x + K.y * K.y;
	float Kcos = K.x * W.x + K.y * W.y;
	float phillips = a * expf(-1 / (l * l * Ksqr)) / (Ksqr * Ksqr * Ksqr) * (Kcos * Kcos);

	// filter out waves moving opposite to wind
	if (Kcos < 0)
		phillips *= dir_depend;

	// damp out waves with very small length w << l
	return phillips * expf(-Ksqr * w * w);
}

ID3D11ShaderResourceView * OceanSimulator::getD3D11DisplacementMap()
{
	return m_pDisplacementSRV;
}

ID3D11ShaderResourceView * OceanSimulator::getD3D11GradientMap()
{
	return m_pGradientSRV;
}


