#include "Framework.h"
#include "Scattering.h"

Scattering::Scattering(Shader * shader)
	: shader(shader), width(128), height(64), domeCount(32)
{

	//CreateQuad();
	CreateDoom();

	texture = new Texture(L"Environment/Starfield.png");
	sStar = shader->AsSRV("StarMap");
	
}

Scattering::~Scattering()
{
	
	SafeDelete(vertexBuffer);
	SafeDelete(domeVertexBuffer);
	SafeDelete(domeIndexBuffer);
}



void Scattering::Render()
{
	
	sStar->SetResource(texture->SRV());

    domeVertexBuffer->Render();
	domeIndexBuffer->Render();

	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	shader->DrawIndexed(0, pass, domeIndexCount);
}





void Scattering::CreateQuad()
{
	VertexTexture vertices[6];
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

	vertexBuffer = new VertexBuffer(vertices, 6, sizeof(VertexTexture));
}

void Scattering::CreateDoom()
{
	UINT latitude = domeCount / 2; // 위도
	UINT longitude = domeCount; // 경도

	domeVertexCount = longitude * latitude * 2;
	domeIndexCount = (longitude - 1) * (latitude - 1) * 2 * 8;


	VertexTexture* vertices = new VertexTexture[domeVertexCount];

	UINT index = 0;
	for (UINT i = 0; i < longitude; i++)
	{
		float xz = 100.0f * (i / (longitude - 1.0f)) * Math::PI / 180.0f;

		for (UINT j = 0; j < latitude; j++)
		{
			float y = Math::PI * j / (latitude - 1);

			vertices[index].Position.x = sinf(xz) * cosf(y);
			vertices[index].Position.y = cosf(xz);
			vertices[index].Position.z = sinf(xz) * sinf(y);

			vertices[index].Uv.x = 0.5f / (float)longitude + i / (float)longitude;
			vertices[index].Uv.y = 0.5f / (float)latitude + j / (float)latitude;

			index++;
		} // for(j)
	}  // for(i)

	for (UINT i = 0; i < longitude; i++)
	{
		float xz = 100.0f * (i / (longitude - 1.0f)) * Math::PI / 180.0f;

		for (UINT j = 0; j < latitude; j++)
		{
			float y = (Math::PI * 2.0f) - (Math::PI * j / (latitude - 1));

			vertices[index].Position.x = sinf(xz) * cosf(y);
			vertices[index].Position.y = cosf(xz);
			vertices[index].Position.z = sinf(xz) * sinf(y);

			vertices[index].Uv.x = 0.5f / (float)longitude + i / (float)longitude;
			vertices[index].Uv.y = 0.5f / (float)latitude + j / (float)latitude;

			index++;
		} // for(j)
	}  // for(i)


	index = 0;
	UINT* indices = new UINT[domeIndexCount * 3];

	for (UINT i = 0; i < longitude - 1; i++)
	{
		for (UINT j = 0; j < latitude - 1; j++)
		{
			indices[index++] = i * latitude + j;
			indices[index++] = (i + 1) * latitude + j;
			indices[index++] = (i + 1) * latitude + (j + 1);

			indices[index++] = (i + 1) * latitude + (j + 1);
			indices[index++] = i * latitude + (j + 1);
			indices[index++] = i * latitude + j;
		}
	}

	UINT offset = latitude * longitude;
	for (UINT i = 0; i < longitude - 1; i++)
	{
		for (UINT j = 0; j < latitude - 1; j++)
		{
			indices[index++] = offset + i * latitude + j;
			indices[index++] = offset + (i + 1) * latitude + (j + 1);
			indices[index++] = offset + (i + 1) * latitude + j;

			indices[index++] = offset + i * latitude + (j + 1);
			indices[index++] = offset + (i + 1) * latitude + (j + 1);
			indices[index++] = offset + i * latitude + j;
		}
	}

	domeVertexBuffer = new VertexBuffer(vertices, domeVertexCount, sizeof(VertexTexture), 0);
	domeIndexBuffer = new IndexBuffer(indices, domeIndexCount);

	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);
}
