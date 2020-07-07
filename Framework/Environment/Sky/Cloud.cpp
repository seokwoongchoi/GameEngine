#include "Framework.h"
#include "Cloud.h"

Cloud::Cloud(Shader * shader)
	: Renderer(shader),shader(shader)
{
	cloudBuffer = new ConstantBuffer(&cloudDesc, sizeof(CloudDesc));
	sCloudBuffer = shader->AsConstantBuffer("CB_Cloud");
	sSRV = shader->AsSRV("CloudMap");

	sCloud1 = shader->AsSRV("Cloud1");
	sCloud2 = shader->AsSRV("Cloud2");

	cloudTexture1 = new Texture(L"Environment/cloud001.dds");
	cloudTexture2 = new Texture(L"Environment/cloud002.dds");

	/*VertexTexture vertices[6];

	vertices[0].Position = Vector3(-1.0f, -1.0f, 0.0f);
	vertices[1].Position = Vector3(-1.0f, +1.0f, 0.0f);
	vertices[2].Position = Vector3(+1.0f, -1.0f, 0.0f);
	vertices[3].Position = Vector3(+1.0f, -1.0f, 0.0f);
	vertices[4].Position = Vector3(-1.0f, +1.0f, 0.0f);
	vertices[5].Position = Vector3(+1.0f, +1.0f, 0.0f);

	vertices[0].Uv = Vector2(0, 1);
	vertices[1].Uv = Vector2(0, 0);
	vertices[2].Uv = Vector2(1, 1);
	vertices[3].Uv = Vector2(1, 1);
	vertices[4].Uv = Vector2(0, 0);
	vertices[5].Uv = Vector2(1, 0);

	vertexBuffer = new VertexBuffer(vertices, 6, sizeof(VertexTexture));*/

	Create();
	//CreateDome();
	int perm[] =
	{
	   151,160,137,91,90,15,
	   131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	   190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	   88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	   77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	   102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	   135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	   5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	   223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	   129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	   251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	   49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};

	int gradValues[] =
	{
	   +1, +1, +0, -1, +1, +0, +1, -1, +0, -1, -1, +0,
	   +1, +0, +1, -1, +0, +1, +1, +0, -1, -1, +0, -1,
	   +0, +1, +1, +0, -1, +1, +0, +1, -1, +0, -1, -1,
	   +1, +1, +0, +0, -1, +1, -1, +1, +0, +0, -1, -1
	};


	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = 256;
	desc.Height = 256;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	Color* pixels = new Color[256 * 256];
	for (int y = 0; y < 256; y++)
	{
		for (int x = 0; x < 256; x++)
		{
			int value = perm[(x + perm[y]) & 0xFF];

			Color color;
			color.r = (float)(gradValues[value & 0x0F] * 64 + 64);
			color.g = (float)(gradValues[value & 0x0F + 1] * 64 + 64);
			color.b = (float)(gradValues[value & 0x0F + 2] * 64 + 64);
			color.a = (float)value;


			UINT index = desc.Width * y + x;
			pixels[index] = color;
		}
	}

	D3D11_SUBRESOURCE_DATA subResource = { 0 };
	subResource.pSysMem = pixels;
	subResource.SysMemPitch = 256 * 4;

	Check(D3D::GetDevice()->CreateTexture2D(&desc, &subResource, &texture));

	D3DX11SaveTextureToFile(D3D::GetDC(), texture, D3DX11_IFF_PNG, L"Noise.png");


	//Create SRV
	{
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format = desc.Format;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

		Check(D3D::GetDevice()->CreateShaderResourceView(texture, &srvDesc, &srv));
	}
	sSRV->SetResource(srv);

	sCloud1->SetResource(cloudTexture1->SRV());
	sCloud2->SetResource(cloudTexture2->SRV());
	SafeDeleteArray(pixels);
}

Cloud::~Cloud()
{
	
	SafeDelete(cloudBuffer);
}


void Cloud::Render(bool bGlow)
{
	//UINT stride = sizeof(VertexTexture);
	//UINT offset = 0;
	cloudBuffer->Apply();
	sCloudBuffer->SetConstantBuffer(cloudBuffer->Buffer());


	//GetTransform()->Scale(10, 1,10);
	//GetTransform()->Rotation(0,0, 0);
	
	//GetTransform()->Scale(250, 250, 250);
	Super::Render();
	
	cloudVertexBuffer->Render();
	cloudIndexBuffer->Render();
	//vertexBuffer->Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	shader->DrawIndexed(0, 4, cloudIndexCount);

	//shader->Draw(0, 4, 6);
}

void Cloud::Update()
{
	Context::Get()->GetCamera()->Position(&position);
	GetTransform()->Position(position.x, position.y - 50.5f, position.z);

	
	GetTransform()->Scale(scale, scale, scale);
	/*ImGui::SliderFloat("Resolution", (float*)&skyPlaneResolution, 0, 1000);
	ImGui::SliderFloat("Width", (float*)&skyPlaneWidth, 0, 1000);
	ImGui::SliderFloat("skyPlaneTop", (float*)&skyPlaneTop, 0, 100);
	ImGui::SliderFloat("skyPlaneBottom", (float*)&skyPlaneBottom,-100, 100);*/
}

void Cloud::Create()
{
	

	float quadSize = skyPlaneWidth / static_cast<float>(skyPlaneResolution);
	float radius = skyPlaneWidth / 2.0f;
	float constant = (skyPlaneTop - skyPlaneBottom) / (radius*radius);
	float textureDelta = (float)textureRepeat / (float)skyPlaneResolution;


	struct skyPlane
	{
		Vector3 position;
		Vector3 uv;
	};

	skyPlane* plane = new skyPlane[(skyPlaneResolution + 1)*(skyPlaneResolution + 1)];


	for (uint i = 0; i <= skyPlaneResolution; i++)
		for (uint j = 0; j <= skyPlaneResolution; j++)
		{
			Vector3 position;
			Vector3 uv;

			position.x = (-0.5f*skyPlaneWidth) + ((float)j*quadSize);
			position.z = (-0.5f*skyPlaneWidth) + ((float)i*quadSize);
			position.y = skyPlaneTop - (constant*((position.x*position.x) + (position.z*position.z)));

			uv.x = (float)j*textureDelta;
			uv.y = (float)i*textureDelta;

			uint index = i * (skyPlaneResolution + 1) + j;
			plane[index].position = position;
			plane[index].uv = uv;
		}

	UINT index = 0;
	cloudVertexCount = ((skyPlaneResolution + 1)*(skyPlaneResolution + 1)) * 6;
	cloudIndexCount = cloudVertexCount;
	VertexTexture* vertices = new VertexTexture[cloudVertexCount];
	UINT* indices = new UINT[cloudVertexCount];
	for (uint j = 0; j < skyPlaneResolution; j++)
	{
		for (uint i = 0; i < skyPlaneResolution; i++)
		{
			uint index1 = j * (skyPlaneResolution + 1) + i;
			uint index2 = j * (skyPlaneResolution + 1) + (i + 1);
			uint index3 = (j + 1) * (skyPlaneResolution + 1) + i;
			uint index4 = (j + 1) * (skyPlaneResolution + 1) + (i + 1);

			// Triangle 1 - Upper Left
			vertices[index].Position =plane[index1].position;
			vertices[index].Uv = D3DXVECTOR2(plane[index1].uv.x, plane[index1].uv.y);
			indices[index] = index;
			index++;

			// Triangle 1 - Upper Right
			vertices[index].Position = plane[index2].position;
			vertices[index].Uv = D3DXVECTOR2(plane[index2].uv.x, plane[index2].uv.y);
			indices[index] = index;
			index++;

			// Triangle 1 - Bottom Left
			vertices[index].Position = plane[index3].position;
			vertices[index].Uv = D3DXVECTOR2(plane[index3].uv.x, plane[index3].uv.y);
			indices[index] = index;
			index++;

			// Triangle 2 - Bottom Left
			vertices[index].Position = plane[index3].position;
			vertices[index].Uv = D3DXVECTOR2(plane[index3].uv.x, plane[index3].uv.y);
			indices[index] = index;
			index++;

			// Triangle 2 - Upper Right
			vertices[index].Position = plane[index2].position;
			vertices[index].Uv = D3DXVECTOR2(plane[index2].uv.x, plane[index2].uv.y);
			indices[index] = index;
			index++;

			// Triangle 2 - Bottom Right
			vertices[index].Position = plane[index4].position;
			vertices[index].Uv = D3DXVECTOR2(plane[index4].uv.x, plane[index4].uv.y);
			indices[index] = index;
			index++;
		}
	}
	cloudVertexBuffer = new VertexBuffer(vertices, cloudVertexCount, sizeof(VertexTexture), 0);
	cloudIndexBuffer = new IndexBuffer(indices, cloudIndexCount);

	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);
}

void Cloud::ImGui()
{
	
	ImGui::InputFloat("Cloud scale", (float*)&scale, 0, 1000);
	ImGui::SliderFloat("Cloud Tiles", (float*)&cloudDesc.Tiles, 0, 5);
	ImGui::SliderFloat("Cloud Cover", (float*)&cloudDesc.Cover, 0, 1.0f);
	ImGui::SliderFloat("Cloud Sharpness", (float*)&cloudDesc.Sharpness, 0, 2.0f);
	ImGui::SliderFloat("Cloud Speed", (float*)&cloudDesc.Speed, 0, 5);
	//static float temp[2] = { cloudDesc.FirstOffset.x,cloudDesc.FirstOffset.y };
	ImGui::SliderFloat("Cloud FirstOffsetX", (float*)&cloudDesc.FirstOffset.x, 0, 1.0f);
	ImGui::SliderFloat("Cloud FirstOffsetY", (float*)&cloudDesc.FirstOffset.y, 0, 1.0f);


	//static float temp2[2] = { cloudDesc.SecondOffset.x,cloudDesc.SecondOffset.y };
	ImGui::SliderFloat("Cloud SecondOffsetX", (float*)&cloudDesc.SecondOffset.x, 0, 1.0f);
	ImGui::SliderFloat("Cloud SecondOffsetY", (float*)&cloudDesc.SecondOffset.y, 0, 1.0f);
	ImGui::End();
}

