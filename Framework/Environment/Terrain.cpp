#include "Framework.h"
#include "Terrain.h"


Terrain::Terrain(Shader * shader, wstring heightMap)
	:Renderer(shader),bCheck(false), baseMap(nullptr), spacing(3, 3)
	, terrainSpacing(1.0f), brushtype(BrushType::None)
	,alphaMap(nullptr)
	,layerMap(nullptr)
	, layerMap2(nullptr)
	, layerMap3(nullptr)
	, layerMap4(nullptr)
	, bUpdate(false)
	, bLayer1(false)
	, bLayer2(false)
	, bLayer3(false)
	, bLayer4(false)
	, bGrid(true)
	
{
	this->heightMap = new Texture(heightMap); //이경로로 텍스쳐가 만들어진다.
	CreateVertexData();
	CreateIndexData();
	CreateNormalData();
	

	SBaseMap = shader->AsSRV("BaseMap");
	//nShader = new Shader(L"004_Quad.fx");
	

	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(TerrainVertex),0,true);
	indexBuffer = new IndexBuffer(indices, indexCount);

	brushBuffer = new ConstantBuffer(&brushDesc, sizeof(BrushDesc));
	sBrushBuffer = shader->AsConstantBuffer("CB_TerrainBrush");

	gridBuffer = new ConstantBuffer(&gridDesc, sizeof(GridDesc));
	sGridBuffer = shader->AsConstantBuffer("CB_GridLine");

	SLayerMap = shader->AsSRV("LayerMap");
	SLayer2Map = shader->AsSRV("LayerMap2");
	SLayer3Map = shader->AsSRV("LayerMap3");
	SLayer4Map = shader->AsSRV("LayerMap4");

	SAlphaMap = shader->AsSRV("AlphaMap");

}

Terrain::~Terrain()
{
	SafeDelete(brushBuffer);
	SafeDelete(heightMap);
	//SafeDelete(nShader);
	
	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);

	//SafeDeleteArray(nv);
	//SafeRelease(nvertexBuffer);

	
}

void Terrain::Update()
{
	//Renderer::Update();
	
	
	//for brushShape combo box
	
		const char* brushShapes[] = { "None" ,"Rectangle", "Circle" };
		static const char* brushShape = brushShapes[0];
	
		//for brushtype combo box

		static float rfactor = 1.0f;
		auto brushType = brushtype;
		const char* brushNames[] = { "None","Raise", "Smoothing","Splatting" };
		static const char* brushName = brushNames[static_cast<uint>(brushType)];

	if (ImGui::CollapsingHeader("Terrain Brush", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Update", &bUpdate);
		if (ImGui::BeginCombo("BrushType", brushName))
		{
			for (uint i = 0; i < IM_ARRAYSIZE(brushNames); i++)
			{
				bool bSelected = brushName == brushNames[i];
				if (ImGui::Selectable(brushNames[i], bSelected))
				{
					brushName = brushNames[i];
					if (brushName == "None")
						brushtype= BrushType::None;
					else if(brushName=="Raise")
						brushtype = BrushType::Raise;
					else if (brushName == "Smoothing")
						brushtype = BrushType::Smooth;
					else if (brushName == "Splatting")
						brushtype = BrushType::Splatting;

				}

				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
			
		}
		if (brushtype == BrushType::Splatting)
		{
			if (ImGui::CollapsingHeader("Splatting Detail", ImGuiTreeNodeFlags_CollapsingHeader))
			{

				SplattingDetail("Base", "##Layer4", layer4name, layerMap4, 4);
				ImGui::SameLine();
				ImGui::Checkbox("##4", &bLayer4);
				SplattingDetail("Layer1", "##Layer1",layer1name,layerMap,1);
				ImGui::SameLine();
				ImGui::Checkbox("##1", &bLayer1);
				
				SplattingDetail("Layer2", "##Layer2", layer2name, layerMap2, 2);
				ImGui::SameLine();
				ImGui::Checkbox("##2", &bLayer2);
			
				SplattingDetail("Layer3", "##Layer3", layer3name, layerMap3, 3);
				ImGui::SameLine();
				ImGui::Checkbox("##3", &bLayer3);
				
				
				
				
				if (bLayer1)
				{
					gridDesc.LayerNum = 1;
				}
				else if (bLayer2)
				{
					gridDesc.LayerNum = 2;
				}
				else if (bLayer3)
				{
					gridDesc.LayerNum = 3;
				}
				else if (bLayer4)
				{
					gridDesc.LayerNum = 4;
				}
				else
					gridDesc.LayerNum = 0;
				
				//cout << gridDesc.LayerNum << endl;
				
				//cout << funcs.size() << endl;
				/*if (ImGui::Button("Save"))
				{
					HWND hWnd = NULL;
					function<void(wstring)> f = bind(&Terrain::SaveMap, this, placeholders::_1);
					Path::SaveFileDialog(L"", Path::ImageFilter, L"", f, hWnd);
				}*/

				
			}
		}
		if (ImGui::BeginCombo("BrushShape", brushShape))
		{
			for (uint i = 0; i < IM_ARRAYSIZE(brushShapes); i++)
			{
				bool bSelected = brushShape == brushShapes[i];
				if (ImGui::Selectable(brushShapes[i], bSelected))
				{
					brushShape = brushShapes[i];
					if (brushShape == "None")
						brushDesc.Type = 0;
					else if (brushShape == "Rectangle")
						brushDesc.Type = 1;
					else if (brushShape == "Circle")
						brushDesc.Type = 2;

				}

				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		//ImGui::InputInt("BrushShape", (int*)&brushDesc.Type);
		ImGui::ColorEdit3("BrushColor", (float*)&brushDesc.Color);
		ImGui::InputInt("BrushRange", (int*)&brushDesc.Range);
		
		ImGui::SliderFloat("BrushRadian", &rfactor, 0.0f, 10.0f);
		if (ImGui::Button("SavePixel"))
		{
			SavePixel();
		}
	}
	ImGui::Checkbox("Grid", &bGrid);
	gridDesc.VisibleGridLine = bGrid ? 1 : 0;
	ImGui::InputFloat("Thickness", &gridDesc.GridLineThickness, 0.1f);
	ImGui::Checkbox("Terrain : WireFrame", &bCheck);

	bool temp = false;
	static Vector3 position(-1, -1, -1);
	if (brushDesc.Type > 0&&!ImGui::IsAnyItemHovered())
	{
		
		if (Mouse::Get()->Press(0))
		{
			temp = bUpdate ?false:true;
			switch (brushtype)
			{
			case BrushType::Raise:
				RaiseHeight(position, brushDesc.Type, brushDesc.Range);
				break;
			case BrushType::Smooth:
				SmoothBrush(position, brushDesc.Type, brushDesc.Range, rfactor);
				break;
			case BrushType::Splatting:
				Splatting(position, brushDesc.Type, brushDesc.Range, rfactor);
				break;
			default:
				break;
			}
			
		}
		
		if (temp == false)
		{
			position = GetPickedPosition();

			brushDesc.Location = position;
		}
	}
	Super::Update();
}

void Terrain::Render()
{
	
	
	//UINT stride = sizeof(TerrainVertex);
	//UINT offset = 0;

	//vertexBuffer->Render();
	//indexBuffer->Render();

	//
	//D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Super::Render();

	if (sBrushBuffer)
	{
		brushBuffer->Apply();
		sBrushBuffer->SetConstantBuffer(brushBuffer->Buffer());
	}

	if (sGridBuffer)
	{
		gridBuffer->Apply();
		sGridBuffer->SetConstantBuffer(gridBuffer->Buffer());
	}

	
	
	bCheck ? Pass(1) : Pass(0);
	shader->DrawIndexed(0,Pass(), indexCount);

	//normal
	UINT nstride = sizeof(Vertex);
	UINT noffset = 0;

	//D3D::GetDC()->IASetVertexBuffers(0, 1, &nvertexBuffer, &nstride, &noffset);
	//D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	//nShader->Draw(0, pass, vertexCount * 4);

	
}

void Terrain::BaseMap(wstring file)
{
	SafeDelete(baseMap);

	baseMap = new Texture(file);
	SBaseMap->SetResource(baseMap->SRV());
}
void Terrain::LayerMap(wstring layer, wstring alpha)
{
	SafeDelete(layerMap);
	SafeDelete(alphaMap);
	layerMap = new Texture(layer);
	alphaMap = new Texture(alpha);
	SLayerMap->SetResource(layerMap->SRV());
	SAlphaMap->SetResource(alphaMap->SRV());
	layer1name = Path::GetFileName(String::ToString(layer));
	
}

void Terrain::LayerMap1(wstring layer)
{
	SafeDelete(layerMap);
	layerMap = new Texture(layer);
	SLayerMap->SetResource(layerMap->SRV());
	layer1name = Path::GetFileName(String::ToString(layer));
	
}

void Terrain::LayerMap2(wstring layer)
{
	SafeDelete(layerMap2);
	layerMap2 = new Texture(layer);
	SLayer2Map->SetResource(layerMap2->SRV());
	layer2name = Path::GetFileName(String::ToString(layer));
	
}

void Terrain::LayerMap3(wstring layer)
{
	SafeDelete(layerMap3);
	layerMap3 = new Texture(layer);
	SLayer3Map->SetResource(layerMap3->SRV());
	layer3name = Path::GetFileName(String::ToString(layer));
}

void Terrain::LayerMap4(wstring layer)
{
	SafeDelete(layerMap4);
	layerMap4 = new Texture(layer);
	SLayer4Map->SetResource(layerMap4->SRV());
	layer4name = Path::GetFileName(String::ToString(layer));
}


float Terrain::GetHeight(Vector3 & position, float terrainSpacing)
{
	uint x = static_cast<uint>(position.x);
	uint z = static_cast<uint>(position.z);

		
	if (x <0 || x> width) return -1.0f;
	if (z <0 || z> height) return -1.0f;

	
	uint index[4];

	index[0] = width * z + x;
	index[1] = width * (z + 1) + x;
	index[2] = width * z + (x + 1);
	index[3] = width * (z + 1) + (x + 1);

	Vector3 v[4];
	
	for (int i = 0; i < 4; i++)
	{
		v[i] = vertices[index[i]].Position;
		
	}
	
	

	float dx = (position.x - v[0].x)/ terrainSpacing;
	float dz = (position.z - v[0].z)/ terrainSpacing;

	Vector3 temp;
	if (dx + dz <= 1) // 아래 삼각형
	{
		temp = v[0] + (v[2] - v[0]) * dx + (v[1] - v[0]) * dz;
		
	}
	else // 윗 삼각형
	{
		dx = 1 - dx;
		dz = 1 - dz;

		temp = v[3] + (v[1] - v[3]) * dx + (v[2] - v[3]) * dz;
		
	}

	return temp.y;
}

Vector3 Terrain::GetTerrainNormal(Vector3& position, Vector3& org)
{
	uint x = static_cast<uint>(position.x);
	uint z = static_cast<uint>(position.z);


	if (x < 0 || x> width) return Vector3(0,0,0);
	if (z < 0 || z> height) return Vector3(0, 0, 0);

	uint index[4];

	index[0] = width * z + x;
	index[1] = width * (z + 1) + x;
	index[2] = width * z + (x + 1);
	index[3] = width * (z + 1) + (x + 1);

	Vector3 v[4];
	Vector3 n[4];

	for (int i = 0; i < 4; i++)
	{
		v[i] = vertices[index[i]].Position;
		D3DXVec3Normalize(&n[i], &vertices[index[i]].Normal);
	}
	float u, v1, distance;
	Vector3 start(position.x, position.y+1, position.z);
	Vector3 dir(0, -1, 0);
	Vector3 normal;
	

	if (D3DXIntersectTri(&v[0], &v[1], &v[2], &start, &dir, &u, &v1, &distance))
	{
		org = start + distance * dir;
		D3DXVec3Cross(&normal, &(v[1] - v[0]), &(v[2] - v[0]));
		//normal = v[0] + (v[1] - v[0])*u + (v[2] - v[0])*v1;
		
	}

	else if (D3DXIntersectTri(&v[3], &v[1], &v[2], &start, &dir, &u, &v1, &distance))
	{
		org = start + distance * dir;
		D3DXVec3Cross(&normal, &(v[2] - v[3]), &(v[1] - v[3]) );
		//normal = v[3] + (v[1] - v[3])*u + (v[2] - v[3])*v1;
	}
	D3DXVec3Normalize(&normal, &normal);
	/*cout << "index 0::"	;
	cout << index[0] << endl;
	cout << "index 1::"	;
	cout << index[1] << endl;
	cout << "index 2::"	;
	cout << index[2] << endl;
	cout << "index 3::"	;
	cout << index[3] << endl;*/
	return normal;
	
}

Vector3 Terrain::GetPickedPosition()
{
	Vector3 org, dir;
	Matrix V = Context::Get()->View();
	Matrix P = Context::Get()->Projection();

//	Context::Get()->GetViewport()->GetRay(&org,&dir,world,V,P);
	Matrix world= GetTransform()->World();
	
	 mouse = Mouse::Get()->GetPosition();
	Vector3 n, f;
	mouse.z = 0.0f;
	Context::Get()->GetViewport()->Unprojection(&n, mouse, world, V, P);

	mouse.z = 1.0f;
	Context::Get()->GetViewport()->Unprojection(&f, mouse, world, V, P);

	dir = f - n;
	org = n;

	for (uint z = 0; z < height - 1; z++)
	{
		for (uint x = 0; x < width-1; x++)
		{
			uint index[4];

			index[0] = width * z + x;
			index[1] = width * (z + 1) + x;
			index[2] = width * z + (x + 1);
			index[3] = width * (z + 1) + (x + 1);

			Vector3 v[4];
			for (int i = 0; i < 4; i++)
			{
				v[i] = vertices[index[i]].Position;
				
			}

			float u, v1, distance;
			

			if (D3DXIntersectTri(&v[0], &v[1], &v[2], &org, &dir, &u, &v1, &distance))
			{
				
				return org + distance * dir;
			
				
			}

			else if (D3DXIntersectTri(&v[3], &v[1], &v[2], &org, &dir, &u, &v1, &distance))
			{
				
				return org + distance * dir;
			}
			
		}
	}

	return Vector3(-1,-1,-1);
}

void Terrain::SavePixel()
{
	

	D3D11_TEXTURE2D_DESC dstDesc;
	ZeroMemory(&dstDesc, sizeof(D3D11_TEXTURE2D_DESC));
	dstDesc.Width = temp.Width;
	dstDesc.Height = temp.Height;
	dstDesc.MipLevels = 1;
	dstDesc.ArraySize = 1;
	dstDesc.Format = temp.Format;
	//dstDesc.BindFlags = heightMapDesc.BindFlags;
	dstDesc.SampleDesc = temp.SampleDesc;
	dstDesc.Usage = D3D11_USAGE_STAGING;
	dstDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE;
	dstDesc.BindFlags = temp.BindFlags;
	

	ID3D11Texture2D* dstTexture = nullptr;

	auto hr = D3D::GetDevice()->CreateTexture2D
	(
		&dstDesc,
		nullptr,
		&dstTexture
	);
	assert(SUCCEEDED(hr));

	D3D11_MAPPED_SUBRESOURCE mappedResource;

	D3D::GetDC()->Map
	(
		dstTexture,
		0,
		D3D11_MAP_WRITE,
		0,
		&mappedResource
	);

	std::vector<Color> pixels;
	pixels.clear();
	pixels.resize(temp.Width*temp.Height);
	uint* pTexels = static_cast<uint*>(mappedResource.pData);


	for (uint z = 0; z < height; z++)
	{
		for (uint x =0; x<width; x++)
		{
			const float factor = 255.0f;
			uint index = width * z + x;
			uint pixel = width * (height - 1 - z) + x;
			pixels[index].r = static_cast<uint>(vertices[index].Position.y*7.5);//*  7.5f 
			pixels[index].g = 0;
			pixels[index].b = 0;
			pixels[index].a = 0;
			
			pTexels[pixel] =
				   (static_cast<uint>(pixels[index].r*factor) << 24 |
					static_cast<uint>(pixels[index].g*factor) << 16 |
					static_cast<uint>(pixels[index].b*factor) << 8 |
					static_cast<uint>(pixels[index].a*factor));

			
							
		}
	}
	
	D3D::GetDC()->Unmap(dstTexture, 0);
	
	hr = D3DX11SaveTextureToFileA
	(
		D3D::GetDC(), 
		dstTexture,
		D3DX11_IFF_BMP,
		"Map.bmp"
	);
	assert(SUCCEEDED(hr));

	SafeRelease(dstTexture);
}

void Terrain::SaveMap(wstring name)
{
}

void Terrain::OnenFile(wstring name, uint layerNum)
{
	switch (layerNum)
	{
	case 1:
		LayerMap1(name);
		break;
	case 2:
		LayerMap2(name);
		break;
	case 3:
		LayerMap3(name);
		break;
	case 4:
		LayerMap4(name);
		break;
	}
}



void Terrain::SplattingDetail(const char * name, const char * InputName, const string& layerName,  Texture* layerMap, uint num)
{
	ImGui::Text(name);
	ImGui::SameLine();
	ImGui::PushItemWidth(110.0f);
	ImGui::InputText(InputName, layerMap ? layerName.c_str() : "N/A", ImGuiInputTextFlags_ReadOnly);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	switch (num)
	{
	case 1:
		if (ImGui::Button("Image"))
		{
			HWND hWnd = NULL;
			function<void(wstring, uint)> f = bind(&Terrain::OnenFile, this, placeholders::_1, placeholders::_2);
			Path::OpenFileDialog(L"", Path::ImageFilter, L"", num, f, hWnd);
		}
		
		break;
	case 2:
		if (ImGui::Button("Image1"))
		{
			HWND hWnd = NULL;
			function<void(wstring, uint)> f = bind(&Terrain::OnenFile, this, placeholders::_1, placeholders::_2);
			Path::OpenFileDialog(L"", Path::ImageFilter, L"", num, f, hWnd);
		}
		
		
		break;
	case 3:
		if (ImGui::Button("Image2"))
		{
			HWND hWnd = NULL;
			function<void(wstring, uint)> f = bind(&Terrain::OnenFile, this, placeholders::_1, placeholders::_2);
			Path::OpenFileDialog(L"", Path::ImageFilter, L"", num, f, hWnd);
		}
		
		
		break;
	case 4:
		if (ImGui::Button("Base"))
		{
			HWND hWnd = NULL;
			function<void(wstring, uint)> f = bind(&Terrain::OnenFile, this, placeholders::_1, placeholders::_2);
			Path::OpenFileDialog(L"", Path::ImageFilter, L"", num, f, hWnd);
		}
		
		
		break;
	}
	
	
}



void Terrain::RaiseHeight(Vector3 & position, uint type, uint range)
{
	D3D11_BOX box;
	box.left = (uint)position.x - range;
	box.top = (uint)position.z + range;
	box.right = (uint)position.x + range;
	box.bottom = (uint)position.z - range;

	if (box.left < 0)box.left = 0;
	if (box.top >= height)box.top = height;
	if (box.right >= width)box.right = width;
	if (box.bottom < 0)box.bottom = 0;

	
	
	switch (type)
	{
	case 1:
		for (uint z = box.bottom; z <= box.top; z++)
		{
			for (uint x = box.left; x <= box.right; x++)
			{
				uint index = width * z + x;
				vertices[index].Position.y += 10.f*Time::Delta();
			}
		}
		break;
	case 2:
		for (uint z = box.bottom; z < box.top ; z++)
			for (uint x = box.left; x < box.right;  x++)
			{
				uint index = width * z + x;

				
					float dx = vertices[index].Position.x-position.x;
					float dy = vertices[index].Position.z - position.z;

					dx *= dx;
					dy *= dy;
					float distanceSquared = dx + dy;
					float radiusSquared = range * range;

					if (distanceSquared <= radiusSquared)
						vertices[index].Position.y += 10.0f*Time::Delta();
				

			}
		break;
	
	}
	
	CreateNormalData();

	/*D3D::GetDC()->UpdateSubresource
	(
		vertexBuffer->Buffer(),
		0,
		nullptr,
		vertices,
		sizeof(TerrainVertex) * vertexCount,
		0
	);*/
	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, vertices, sizeof(TerrainVertex)*vertexCount);
	}
	D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);
}

void Terrain::SmoothBrush(Vector3 & position, uint type, uint range, float factor)
{
	D3D11_BOX box;
	box.left = (uint)position.x - range;
	box.top = (uint)position.z + range;
	box.right = (uint)position.x + range;
	box.bottom = (uint)position.z - range;

	for (uint z = box.bottom; z < box.top; z++)
		for (uint x = box.left; x < box.right; x++)
		{
			uint index = width * z + x;


			float dx = vertices[index].Position.x - position.x;
			float dy = vertices[index].Position.z - position.z;
			

			dx *= dx;
			dy *= dy;
			float distanceSquared =sqrtf( dx + dy);
			//float distance = dx + dy;
			float c = acosf(distanceSquared /range);
			float rfactor = sinf(c)*factor;
			if(range>= distanceSquared)
			vertices[index].Position.y += 15* Time::Get()->Delta()*(rfactor+1);
				
			

		}
	CreateNormalData();
	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, vertices, sizeof(TerrainVertex)*vertexCount);
	}
	D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);
}

void Terrain::Platting(Vector3 & position, uint type, uint range)
{
}

void Terrain::Splatting(Vector3 & position, uint type, uint range, uint alpha)
{
	if (!bLayer1 && !bLayer2 && !bLayer3 && !bLayer4) return;
	D3D11_BOX box;
	box.left = (uint)position.x - range;
	box.top = (uint)position.z + range;
	box.right = (uint)position.x + range;
	box.bottom = (uint)position.z - range;

	if (box.left < 0)box.left = 0;
	if (box.top >= height)box.top = height;
	if (box.right >= width)box.right = width;
	if (box.bottom < 0)box.bottom = 0;

	static Vector3 mousePos(0, 0, 0);
	mousePos = GetPickedPosition();

	
	switch (type)
	{
	case 1:
		for (uint z = box.bottom; z <= box.top; z++)
		{
			for (uint x = box.left; x <= box.right; x++)
			{
				uint index = width * z + x;
				
				float dx = vertices[index].Position.x - mousePos.x;
				float dz = vertices[index].Position.z - mousePos.z;

				float dist = sqrt(dx * dx + dz * dz);
			
				if (bLayer1)
				{
					vertices[index].Color.r = 1.0f/(dist);
					
					vertices[index].Color.g = 0.0f;
					vertices[index].Color.b = 0.0f;
					vertices[index].Color.a = 0.0f;
				}
				else if (bLayer2)
				{
					vertices[index].Color.r = 0.0f;
					vertices[index].Color.g = 1.0f;
					vertices[index].Color.b = 0.0f;
					vertices[index].Color.a = 0.0f;
				}
					
				else if (bLayer3)
				{
					vertices[index].Color.r = 0.0f;
					vertices[index].Color.g = 0.0f;
					vertices[index].Color.b = 1.0f;
					vertices[index].Color.a = 0.0f;
				}
				else if (bLayer4)
				{
					vertices[index].Color.r = 0.0f;
					vertices[index].Color.g = 0.0f;
					vertices[index].Color.b = 0.0f;
					vertices[index].Color.a = 1.0f;
				}
			}
		}
		break;
	case 2:
		for (uint z = box.bottom; z < box.top; z++)
			for (uint x = box.left; x < box.right; x++)
			{
				uint index = width * z + x;


				float dx = vertices[index].Position.x - position.x;
				float dy = vertices[index].Position.z - position.z;

				dx *= dx;
				dy *= dy;
				float distanceSquared = dx + dy;
				float radiusSquared = range * range;

				if (distanceSquared <= radiusSquared)
				{
					if (bLayer1)
					{
						vertices[index].Color.r = 1.0f;
						vertices[index].Color.g = 0.0f;
						vertices[index].Color.b = 0.0f;
						vertices[index].Color.a = 0.0f;
					}
					else if (bLayer2)
					{
						vertices[index].Color.r = 0.0f;
						vertices[index].Color.g = 1.0f;
						vertices[index].Color.b = 0.0f;
						vertices[index].Color.a = 0.0f;
					}

					else if (bLayer3)
					{
						vertices[index].Color.r = 0.0f;
						vertices[index].Color.g = 0.0f;
						vertices[index].Color.b = 1.0f;
						vertices[index].Color.a = 0.0f;
					}
					else if (bLayer4)
					{
						vertices[index].Color.r = 0.0f;
						vertices[index].Color.g = 0.0f;
						vertices[index].Color.b = 0.0f;
						vertices[index].Color.a = 1.0f;
					}
				}
					


			}
		break;

	}

	CreateNormalData();

	/*D3D::GetDC()->UpdateSubresource
	(
		vertexBuffer->Buffer(),
		0,
		nullptr,
		vertices,
		sizeof(TerrainVertex) * vertexCount,
		0
	);*/
	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, vertices, sizeof(TerrainVertex)*vertexCount);
	}
	D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);
}

void Terrain::CreateVertexData()
{
	//TODO: ReadPixel
	//unsinged nomal 0~1
	vector<Color>heights;
	//ReadHeightMap(&heights);
	
	temp = heightMap->ReadPixel(DXGI_FORMAT_R8G8B8A8_UNORM, &heights); //
	width = heightMap->GetWidth();
	height = heightMap->GetHeight();
	/*FILE* file = fopen("text.csv", "w");
	for (uint z = 0; z < height; z++)
		for (uint x = 0; x < width; x++)
	{	
			uint index = width * z + x;

			uint r = (uint)heights[index].r * 255;
			uint g = (uint)heights[index].g * 255;
			uint b = (uint)heights[index].b * 255;
			uint a = (uint)heights[index].a * 255;
		fprintf(file, "%d,%d,%d,%f,%f,%f,%f\n", index, x, z,
			r, g, b, a);
	}
	fclose(file);*/
	
	if (temp.Format != DXGI_FORMAT_R8G8B8A8_UNORM)
		return;
	vertexCount = width * height;
	vertices = new TerrainVertex[vertexCount];
	for(uint z=0; z<height;z++)
		for (uint x = 0; x < width; x++)
		{
			uint index = width * z + x;
			uint pixel = width * (height - 1 - z) + x;
			
			vertices[index].Position.x = static_cast<float>(x);
			//vertices[index].Position.y = heights[pixel].r* 255.0f / 7.5f;
				
			vertices[index].Position.y = 0;
			vertices[index].Position.z = static_cast<float>(z);

		/*	vertices[index].Uv.x = ((float)x / (float)width)*spacing.x;
			vertices[index].Uv.y = ((float)(height - 1 - z)/(float)height)*spacing.y;*/

			vertices[index].Uv.x = ((float)x / (float)width);
			vertices[index].Uv.y = ((float)(height - 1 - z) / (float)height);
			
		}
//normal
	nv = new Vertex[vertexCount * 4];

	
		
	
}

void Terrain::CreateIndexData()
{
	indexCount = (width - 1) * (height - 1) * 6;
	indices = new UINT[indexCount];

	UINT index = 0;
	for (UINT z = 0; z < height - 1; z++)
	{
		for (UINT x = 0; x < width - 1; x++)
		{
			indices[index + 0] = width * z + x;
			indices[index + 1] = width * (z + 1) + x;
			indices[index + 2] = width * z + x + 1;
			indices[index + 3] = width * z + x + 1;
			indices[index + 4] = width * (z + 1) + x;
			indices[index + 5] = width * (z + 1) + x + 1;

			index += 6;
		}
	}

}

void Terrain::CreateNormalData()
{
	for (uint i = 0; i < indexCount / 3; i++)
	{
		uint index0 = indices[i * 3 + 0];
		uint index1 = indices[i * 3 + 1];
		uint index2 = indices[i * 3 + 2];

		TerrainVertex v0 = vertices[index0];
		TerrainVertex v1 = vertices[index1];
		TerrainVertex v2 = vertices[index2];

		Vector3 d1 = v1.Position - v0.Position;
		Vector3 d2 = v2.Position - v0.Position;

		Vector3 normal;
		D3DXVec3Cross(&normal, &d1, &d2); 
		D3DXVec3Normalize(&normalize, &normal);
	
		vertices[index0].Normal += normal;
		vertices[index1].Normal += normal;
		vertices[index2].Normal += normal;
	
	}
	
	for (uint i = 0; i < vertexCount; i++)
	{
		D3DXVec3Normalize(&vertices[i].Normal, &vertices[i].Normal);
	}

	/*for (UINT i = 0; i < normals.size(); i++)
	{
		nv[i].Position = normals[i];
	}*/
	
}

void Terrain::CreateTestTexture()
{
	ID3D11Texture2D* texture;
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

	desc.Width = temp.Width;
	desc.Height = temp.Height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc = temp.SampleDesc;
	desc.BindFlags = temp.BindFlags;
	desc.Format=temp.Format;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	
	
	D3D11_SUBRESOURCE_DATA data = { 0 };
	uint* pixels = new uint[temp.Width*temp.Height];
	ZeroMemory(pixels, sizeof(uint)*temp.Width*temp.Height);

	for (uint z = 0; z < height; z++)
	{
		for (uint x = 0; x < width; x++)
		{
			const float factor = 255.0f;
			uint index = width * z + x;
			uint pixel = width * (height - 1 - z) + x;
			pixels[index]=static_cast<uint>(vertices[index].Position.y*7.5);//*  7.5f 
		}
	}

	data.pSysMem = pixels;
	data.SysMemPitch = temp.Width * 4;// 줄의 byte수
	data.SysMemSlicePitch = temp.Width *temp.Height* 4;

	auto hr = D3D::GetDevice()->CreateTexture2D(&desc, &data, &texture);
	
	hr = D3DX11SaveTextureToFile(D3D::GetDC(), texture, D3DX11_IFF_BMP, L"Test.BMP");
	/*D3D11_MAPPED_SUBRESOURCE sub;
	D3D::GetDC()->Map(texture, 0, D3D11_MAP_WRITE, 0, &sub);
	{
		int a = 0;
	}
	D3D::GetDC()->Unmap(texture, 0);*/

}

void Terrain::ReadHeightMap(vector<Color>* colors)
{
	/*D3DX11_IMAGE_INFO info;
	D3DX11GetImageInfoFromFile(L"../../_Texture/HeightMaps/Test.png", nullptr, &info, nullptr);
	D3DX11_IMAGE_LOAD_INFO loadInfo= D3DX11_IMAGE_LOAD_INFO();
	loadInfo.Width = info.Width;
	loadInfo.Height = info.Height;
	loadInfo.Usage = D3D11_USAGE_STAGING;
	loadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_READ;
	loadInfo.MipLevels = 0;*/
	//ID3D11Texture2D* texture;
	//D3DX11CreateTextureFromFile(D3D::GetDevice(), L"../../_Texture/HeightMaps/Test.png", &loadInfo, nullptr, (ID3D11Resource**)&texture, nullptr);
	ID3D11Texture2D* texture = heightMap->GetTexture();

	//texture->GetDesc(&temp);

	
	ID3D11Texture2D* dest = nullptr;
	D3D11_TEXTURE2D_DESC destDesc;
	texture->GetDesc(&destDesc);
	destDesc.Usage = D3D11_USAGE_STAGING;
	destDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	destDesc.SampleDesc = temp.SampleDesc;
	destDesc.BindFlags = 0;

	D3D::GetDevice()->CreateTexture2D(&destDesc, NULL, &dest);
	
	
	D3D::GetDC()->CopyResource(dest,texture);

	uint* pixels = new uint[temp.Width*temp.Height];
	D3D11_MAPPED_SUBRESOURCE subResources;
	D3D::GetDC()->Map(dest, 0, D3D11_MAP_READ, 0, &subResources);
	{
		memcpy(pixels, subResources.pData, sizeof(float)*temp.Width*temp.Height);
	}
	D3D::GetDC()->Unmap(dest, 0);

	
	for (uint y = 0; y < temp.Height; y++)
	{
		uint start = y * subResources.RowPitch;
			for (uint x = 0; x < temp.Width; x++)
			{
				uint index = x * 4;

				float f = 1.0f / 255.0f;
				uint a = (uint)((0xFF000000 & pixels[index]) >> 24);
				uint b = (uint)((0x00FF0000 & pixels[index]) >> 16);
				uint g = (uint)((0x0000FF00 & pixels[index]) >> 8);
				uint r = (uint)((0x000000FF & pixels[index]) >> 0);

				colors->emplace_back(Color(r, g, b, a));
				
				

				int n = 0;
			
			}
	}
	//fclose(file);
	
	
}





