#include "Framework.h"
#include "TerrainLOD.h"

#include "Environment/Billboard.h"


TerrainLOD::TerrainLOD(InitializeInfo & info)
	:Renderer(info.shader), info(info)
	, baseTexture(NULL)
	, normalTexture(NULL)
	, roughnessTexture(NULL)
	, displaymentTexture(NULL)
	, brushType(BrushType::None)
	, raiseSpeed(1.0f)
	, selectedBillboard(L"N/A")
	, billbaordIndex(0)
	, billboardCount(1)
	
	
{
	UpdateShader = new Shader(L"Deferred/UpdateHeightCS.fx");
	sUpdate = UpdateShader->AsUAV("Update");
	billboardScale[0] = 1;
	billboardScale[1] = 1;
	currentPath = L"../../_Textures/Terrain/Billboards/";
	BillboardFinder(currentPath);
	for (uint i = 0; i < billboarfiles.size(); i++)
		billboardTexture[i] = new Texture(billboardTextures[i]);
	//billboardShader = new Shader(L"020_BillboardGS.fx");
	billboard = new Billboard(info.shader, billboardTextures);
	mousePos = Vector3(0, 0, 0);
	//Topology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	
	sBaseTexture = shader->AsSRV("BaseMap");
	sNormalTexture = shader->AsSRV("NormalMap");
	sRoughnessTexture = shader->AsSRV("TerrainRoughMap");
	sDisplaymentTexture = shader->AsSRV("displaymentMap");
	sHeightTexture = shader->AsSRV("LodHeightMap");

	//rayBuffer = new ConstantBuffer(&rayDesc, sizeof(RayDesc));
	//sRayBuffer = UpdateShader->AsConstantBuffer("CB_Ray");

	frustum = new Frustum();
	buffer = new ConstantBuffer(&bufferDesc, sizeof(BufferDesc));
	sBuffer = shader->AsConstantBuffer("CB_Terrain");

	bufferDesc.HeightRatio = info.HeightRatio;


	heightMap = new Texture(info.heightMap);
	
	//sHeightTexture->SetResource(heightMap->SRV());
	
	//heightMap->ReadPixels(DXGI_FORMAT_R8G8B8A8_UNORM, &heightMapPixel);
	heightMap->ReadPixels(DXGI_FORMAT_R32_FLOAT, &heightMapPixel);
	

	width = this->heightMap->GetWidth() - 1;
	height = this->heightMap->GetHeight() - 1;

	patchVertexRows = (width / info.CellsPerPatch) + 1; //
	patchVertexCols = (height / info.CellsPerPatch) + 1;

	vertexCount = patchVertexRows * patchVertexCols;
	faceCount = (patchVertexRows - 1) * (patchVertexCols - 1);
	indexCount = faceCount * 4;

	

	CalcBoundY();                                                                                                                                                    
	CreateVertexData();
	CreateIndexData();


	root = new QuadTreeNode();
	
	

	CreateQuadTree(root, info, Vector2(0, 0), Vector2(width, height));
	

	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(VertexTerrain));
	indexBuffer = new IndexBuffer(indices, indexCount);

	bufferDesc.TexelCellSpaceU = 1.0f / ((float)heightMap->GetWidth() - 1.0f);
	bufferDesc.TexelCellSpaceV = 1.0f / ((float)heightMap->GetHeight() - 1.0f);
	//float fov = 0.25;
	//float zFar = 100.f;
	//camera = new Fixity();
	//camera->Position(0, 0, -50.0f);
	//perspective = new Perspective(D3D::Width(), D3D::Height(), 0.1f, zFar, fov);
	//
	//frustum = new Frustum(camera, perspective);
	//frustum = new Frustum(Context::Get()->GetCamera(), Context::Get()->GetPerspective());
	brushBuffer = new ConstantBuffer(&brushDesc, sizeof(BrushDesc));
	sBrushBuffer = shader->AsConstantBuffer("CB_TerrainBrush");

	updateBuffer = new ConstantBuffer(&updateDesc, sizeof(UpdateDesc));
	sUpdateBuffer = UpdateShader->AsConstantBuffer("UpdateDesc");

	D3D11_TEXTURE2D_DESC desc;
	heightMap->GetTexture()->GetDesc(&desc);
	// Allocate the downscaled target
	D3D11_TEXTURE2D_DESC dtd = {
		desc.Width,//512, //UINT Width;
		desc.Height,//512, //UINT Height;
		1, //UINT MipLevels;
		1, //UINT ArraySize;
		DXGI_FORMAT_R32_FLOAT,
		//DXGI_FORMAT_R8G8B8A8_UNORM,//desc.Format,//DXGI_FORMAT_R32_TYPELESS, //DXGI_FORMAT Format;
		1, //DXGI_SAMPLE_DESC SampleDesc;
		0,
		D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS,//UINT BindFlags;
		0,//UINT CPUAccessFlags;
		0//UINT MiscFlags;    
	};
	Check(D3D::GetDevice()->CreateTexture2D(&dtd, NULL, &updateTexture));

	//ID3D11Texture2D* srcTexture;
	//heightMap->SRV()->GetResource((ID3D11Resource **)&srcTexture);
	/*uint rowPitch = (desc.Width*4) * sizeof(unsigned char);
	D3D::GetDC()->UpdateSubresource(updateTexture, 0, NULL, heightMap->GetTexture(), rowPitch,NULL);*/
	auto hr = D3DX11LoadTextureFromTexture(D3D::GetDC(), heightMap->GetTexture(), NULL, updateTexture);
	Check(hr);

	// Create the resource views
	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd;
	ZeroMemory(&dsrvd, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	dsrvd.Format = dtd.Format;//desc.Format;
//	dsrvd.Format = DXGI_FORMAT_R32_FLOAT;
	dsrvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	dsrvd.Texture2D.MipLevels = 1;
	Check(D3D::GetDevice()->CreateShaderResourceView(updateTexture, &dsrvd, &updateSRV));
	
	// Create the UAVs
	D3D11_UNORDERED_ACCESS_VIEW_DESC DescUAV;
	ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.Format = dtd.Format;// ;
	//DescUAV.Format = DXGI_FORMAT_R32_FLOAT;
	DescUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	DescUAV.Buffer.FirstElement = 0;
	DescUAV.Buffer.NumElements = desc.Width * desc.Height;
	Check(D3D::GetDevice()->CreateUnorderedAccessView(updateTexture, &DescUAV, &updateUAV));

	
	D3DXMatrixIdentity(&world);
}

TerrainLOD::~TerrainLOD()
{
	
	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);

	SafeDelete(buffer);
	//SafeDelete(frustum);
	SafeDelete(heightMap);

	SafeDelete(baseTexture);
	SafeDelete(normalTexture);
	
}

void TerrainLOD::BaseTexture(wstring file)
{
	SafeDelete(baseTexture);

	baseTexture = new Texture(file);
//	uint pixel = 2048;
//	wstring fileName = String::ToWString(Path::GetFileName(String::ToString(file)));
//	wstring ex = String::ToWString(Path::GetExtension(String::ToString(file)));
//	ID3D11Texture2D * texture=nullptr;
//	D3D11_TEXTURE2D_DESC desc;
//	//ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
//	baseTexture->GetTexture()->GetDesc(&desc);
//	D3D11_TEXTURE2D_DESC dtd = {
//		desc.Width,//512, //UINT Width;
//		desc.Height,//512, //UINT Height;
//		4, //UINT MipLevels;
//		1, //UINT ArraySize;
//		desc.Format ,
//		//DXGI_FORMAT_R8G8B8A8_UNORM,//desc.Format,//DXGI_FORMAT_R32_TYPELESS, //DXGI_FORMAT Format;
//		1, //DXGI_SAMPLE_DESC SampleDesc;
//		0,
//		D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
//		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
//		0,//UINT CPUAccessFlags;
//		D3D11_RESOURCE_MISC_GENERATE_MIPS//UINT MiscFlags;    
//	};
//	
//	
//	
//	Check(D3D::GetDevice()->CreateTexture2D(&dtd, NULL, &texture));
//
//	auto hr = D3DX11LoadTextureFromTexture(D3D::GetDC(), baseTexture->GetTexture(), NULL, texture);
//	Check(hr);
//
//	// Create the resource views
//	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd;
//	//ZeroMemory(&dsrvd, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
//	baseTexture->SRV()->GetDesc(&dsrvd);
////	
//	dsrvd.Texture2D.MipLevels = 4;
//	dsrvd.Texture2D.MostDetailedMip = 0;
//
//	
//	Check(D3D::GetDevice()->CreateShaderResourceView(texture, &dsrvd, &mipSrv));
//	
//	
//	D3D::GetDC()->GenerateMips(mipSrv);
}

void TerrainLOD::PBRTextures(wstring normal, wstring rough, wstring display)
{
	SafeDelete(normalTexture);
	SafeDelete(roughnessTexture);
	SafeDelete(displaymentTexture);


	normalTexture = new Texture(normal);

	//D3D11_TEXTURE2D_DESC desc;
	//normalTexture->GetTexture()->GetDesc(&desc);
	//desc.MipLevels = 5;
	//desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	//D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	//normalTexture->SRV()->GetDesc(&srvDesc);
	//srvDesc.Texture2D.MostDetailedMip = 0;
	//srvDesc.Texture2D.MipLevels = 5;
	////srvDesc.TextureCube.MipLevels = maxMipLevels;
	//ID3D11Texture2D* ntexture;
	//ID3D11ShaderResourceView* normalSrv;
	//Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &ntexture));
	//Check(D3D::GetDevice()->CreateShaderResourceView(ntexture, &srvDesc, &normalSrv));
	//sNormalTexture->SetResource(normalSrv);
    //SafeDelete(normalTexture);
	roughnessTexture = new Texture(rough);
	/*ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	roughnessTexture->GetTexture()->GetDesc(&desc);
	desc.MipLevels = 5;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	roughnessTexture->SRV()->GetDesc(&srvDesc);
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 5;
	ID3D11Texture2D* rtexture;
	ID3D11ShaderResourceView* rSrv;
	Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &rtexture));
	Check(D3D::GetDevice()->CreateShaderResourceView(rtexture, &srvDesc, &rSrv));
	SafeDelete(roughnessTexture);*/
	//sRoughnessTexture->SetResource(rSrv);

	/*displaymentTexture = new Texture(display);
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	roughnessTexture->GetTexture()->GetDesc(&desc);
	desc.MipLevels = 5;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	roughnessTexture->SRV()->GetDesc(&srvDesc);
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 5;
	ID3D11Texture2D* rtexture;
	ID3D11ShaderResourceView* rSrv;
	Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &rtexture));
	Check(D3D::GetDevice()->CreateShaderResourceView(rtexture, &srvDesc, &rSrv));
    sDisplaymentTexture->SetResource(displaymentTexture->SRV());*/
	
}

void TerrainLOD::NormalTextures(wstring normal)
{
	SafeDelete(normalTexture);
	
	normalTexture = new Texture(normal);
	
}

void TerrainLOD::RoughnessTextures(wstring rough)
{

	SafeDelete(roughnessTexture);
    roughnessTexture = new Texture(rough);
	
}

void TerrainLOD::Update()
{
	Super::Update();

	//static bool bUpdate = false;
	//if (Keyboard::Get()->Down('G'))
	//{
	//	bUpdate ? bUpdate = false : bUpdate = true;
	//}
	//if (bUpdate)
	{
		frustum->Update();
		frustum->Render(bufferDesc.WorldFrustumPlanes);
	}
	
	brushDesc.Location = pos;
	ImGui();

	/*V = Context::Get()->View();
	P = Context::Get()->Projection();


	Context::Get()->GetViewport()->GetRay(&org, &dir, world, V, P);

	
	rayDesc.org = org;
	rayDesc.dir = dir;

	rayBuffer->Apply();
	sRayBuffer->SetConstantBuffer(rayBuffer->Buffer());*/
	billboard->Update();
	//Debug::DebugVector(pos);
	
	//auto mainCamera = Context::Get()->GetCamera();
	//Vector3 camPos;
	//mainCamera->Position(&camPos);
	//camera->Position(camPos);
	//frustum->Update();
	
}

void TerrainLOD::Render()
{
	Super::Render();
    sBaseTexture->SetResource(baseTexture->SRV());
	sNormalTexture->SetResource(normalTexture->SRV());
	sRoughnessTexture->SetResource(roughnessTexture->SRV());
	//if (displaymentTexture != NULL)
	//	sDisplaymentTexture->SetResource(displaymentTexture->SRV());

	
	sHeightTexture->SetResource(updateSRV);
	
	
	/*static bool onfFog = false;
	if (Keyboard::Get()->Down('R'))
	{
		onfFog = onfFog ? false : true;
	}
	if (onfFog)
	{
		RenderBox(root);

	}*/

	
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());
	
	
	Topology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	shader->DrawIndexed(0, Pass(), indexCount);
	billboard->Pass(12);
	billboard->Render();
}

bool TerrainLOD::InBounds(UINT row, UINT col)
{
	return row >= 0 && row < height && col >= 0 && col < width;
}

void TerrainLOD::CalcBoundY()
{
	//한지역의 boundy를 계산
	bounds.clear();
	bounds.shrink_to_fit();
	bounds.assign(faceCount, Vector2());

	for (UINT row = 0; row < patchVertexRows - 1; row++)
	{
		for (UINT col = 0; col < patchVertexCols - 1; col++)
			CalcPatchBounds(row, col);
	}
	
}

void TerrainLOD::CalcPatchBounds(UINT row, UINT col)
{
	//각 구역별 계산
	UINT x0 = col * info.CellsPerPatch;
	UINT x1 = (col + 1) * info.CellsPerPatch;

	UINT y0 = row * info.CellsPerPatch;
	UINT y1 = (row + 1) * info.CellsPerPatch;
	
	float minY = FLT_MAX;
	float maxY = -FLT_MAX;

	for (UINT y = y0; y <= y1; y++)
	{
		for (UINT x = x0; x <= x1; x++)
		{
			
			float data = 0.0f;
			if (InBounds(y, x))
				data = heightMapPixel[y * (width + 1) + x].b*info.HeightRatio;

			minY = min(minY, data);
			maxY = max(maxY, data);
		}
	}

	UINT patchID = row * (patchVertexCols - 1) + col;
	bounds[patchID] = Vector2(minY, maxY);
	
}
void TerrainLOD::CreateVertexData()
{
	vertices = new VertexTerrain[vertexCount];

	float halfWidth = 0.5f * (float)width;
	float halfDepth = 0.5f * (float)height;

	float patchWidth = (float)width / (float)(patchVertexCols - 1);
	float patchDepth = (float)height / (float)(patchVertexRows - 1);

	float du = 1.0f / (float)(patchVertexCols - 1);
	float dv = 1.0f / (float)(patchVertexRows - 1);

	for (UINT row = 0; row < patchVertexRows; row++)
	{
		float z = halfDepth - (float)row * patchDepth;
		for (UINT col = 0; col < patchVertexCols; col++)
		{
			float x = -halfWidth + (float)col * patchWidth;
			UINT vertId = row * patchVertexCols + col;

			vertices[vertId].Position = Vector3(x, 0, z);
			
			vertices[vertId].Uv = Vector2(col * du, row * dv);

			//vertices[vertId].Uv.x *=3;// ((float)x / (float)width)*3;
			//vertices[vertId].Uv.y *= 3; //((float)(height - 1 - z) / (float)height)*3;
		}
	}

	for (UINT row = 0; row < patchVertexRows - 1; row++)
	{
		for (UINT col = 0; col < patchVertexCols - 1; col++)
		{
			UINT patchID = row * (patchVertexCols - 1) + col;
			UINT vertID = row * patchVertexCols + col;

			vertices[vertID].BoundsY = bounds[patchID];
		}
	}
}

void TerrainLOD::CreateIndexData()
{
	vector<WORD> indices;
	for (WORD row = 0; row < (WORD)patchVertexRows - 1; row++)
	{
		for (WORD col = 0; col < (WORD)patchVertexCols - 1; col++)
		{
			indices.push_back(row * (WORD)patchVertexCols + col);
			indices.push_back(row * (WORD)patchVertexCols + col + 1);
			indices.push_back((row + 1) * (WORD)patchVertexCols + col);
			indices.push_back((row + 1) * (WORD)patchVertexCols + col + 1);
		}
	}

	this->indices = new UINT[indexCount];
	copy
	(
		indices.begin(), indices.end(),
		stdext::checked_array_iterator<UINT *>(this->indices, indexCount)
	);

	
}

void TerrainLOD::CreateQuadTree(QuadTreeNode* node, InitializeInfo info, Vector2 leftTop, Vector2 rightBottom)
{
	float TileSize = 2;


	// convert the heightmap index bounds into world-space coordinates
	/*minX = leftTop.x * CellsPerPatch - Width / 2;
	maxX = rightBottom.x * CellsPerPatch - Width / 2;
	minZ = -leftTop.y * CellsPerPatch + Depth / 2;
	maxZ = -rightBottom.y * CellsPerPatch + Depth / 2;*/
	node->minX = leftTop.x * info.CellSpacing - width / 2;
	node->maxX = rightBottom.x * info.CellSpacing - width / 2;


	
	node->minZ = -leftTop.y * info.CellSpacing + height / 2;
	node->maxZ = -rightBottom.y * info.CellSpacing + height / 2;

	//cout << node->minX << endl;

	/*node->minX = leftTop.x ;
	node->maxX = rightBottom.x;
    node->minZ = leftTop.y ;
	node->maxZ = rightBottom.y;
*/
	// adjust the bounds to get a very slight overlap of the bounding boxes
	node->minX -= tolerance;
	node->maxX += tolerance;
	node->minZ += tolerance;
	node->maxZ -= tolerance;
	//UINT patchID = leftTop.x * (patchVertexCols - 1) + leftTop.y;

	node->minMaxY = GetMinMaxY(leftTop, rightBottom);



	node->boundsMin = Vector3(node->minX, node->minMaxY.x, node->minZ);
	node->boundsMax = Vector3(node->maxX, node->minMaxY.y, node->maxZ );
	/*
	Matrix world = D3DXMATRIX(
		0, 0, 1, 0,
		0, 1, 0, 0,
		1, 0, 0, 0,
		0, 0, 0, 1);
	
	D3DXVec3TransformCoord(&node->boundsMin, &Vector3(node->minX, node->minMaxY.x, node->minZ), &world);
	D3DXVec3TransformCoord(&node->boundsMax, &Vector3(node->maxX, node->minMaxY.y, node->maxZ), &world);*/
	// construct the new node and assign the world-space bounds of the terrain region


	
	float nodeWidth = (rightBottom.x - leftTop.x)*0.5f;
	float nodeDepth = (rightBottom.y - leftTop.y)*0.5f;
	

	// we will recurse until the terrain regions match our logical terrain tile sizes

	node->childs.clear();
	if (nodeWidth >= TileSize && nodeWidth >= TileSize)
	{
		QuadTreeNode* child1 = new QuadTreeNode();
		QuadTreeNode* child2 = new QuadTreeNode();
	    QuadTreeNode* child3 = new QuadTreeNode();
		QuadTreeNode* child4 = new QuadTreeNode();
	
		node->childs.emplace_back(child1);
		node->childs.emplace_back(child2);
		node->childs.emplace_back(child3);
		node->childs.emplace_back(child4);

		
		CreateQuadTree(node->childs[0], info, leftTop, Vector2(leftTop.x + nodeWidth, leftTop.y + nodeDepth));
		CreateQuadTree(node->childs[1], info, Vector2(leftTop.x + nodeWidth, leftTop.y), Vector2(rightBottom.x, leftTop.y + nodeDepth));
		CreateQuadTree(node->childs[2], info, Vector2(leftTop.x, leftTop.y + nodeDepth), Vector2(leftTop.x + nodeDepth, rightBottom.y));
		CreateQuadTree(node->childs[3], info, Vector2(leftTop.x + nodeWidth, leftTop.y + nodeDepth), rightBottom);
		

		
	}
	
}

Vector2 TerrainLOD::GetMinMaxY(Vector2 tl, Vector2 br)
{
	float minY = FLT_MAX;
	float maxY = FLT_MIN;

	for (uint x = (uint)tl.x; x < br.x; x++) {
		for (uint y = (uint)tl.y; y < br.y; y++) {

			uint index = y * (width+1) + x;
			
			float data = 0.0f;
			if (InBounds(y, x))
				data = heightMapPixel[index].b*info.HeightRatio ;
			//cout << data << endl;
			minY = min(minY, data);
			maxY = max(maxY, data);
		}
	}
	
	return Vector2(minY, maxY);
}

bool TerrainLOD::Intersection(Vector3& Pos)
{
	/*if (Keyboard::Get()->Down(1))
	{
		bClicked = true;
	}
	if (!bClicked) return;*/

	 V= Context::Get()->View();
	 P= Context::Get()->Projection();

	
	Context::Get()->GetViewport()->GetRay(&org, &dir, world, V, P);
	
	if (!root->Intersection( org, dir, pos))
	{
		return false;
	}
	
	mousePos = pos;
	Pos = pos;

	//bClicked = false;
	return true;
}

//float Lerp(float a,float b,float t)
//
//cc.lerp = function(a, b, r) {
//	return a + (b - a) * r;
//};
void TerrainLOD::Intersection( Matrix* matrix)
{
	Vector3 actorPos;
		if (!root->Intersection(Vector3(matrix->_41, 100, matrix->_43), Vector3(-0.0f, -1.0f, -0.000001f), actorPos))
		{
			return;
		}
	

		D3DXVec3Lerp(&lerp, &Vector3(0, matrix->_42, 0), &actorPos, Time::Delta()*10.0f);
		matrix->_42 = lerp.y;
		
}

void TerrainLOD::RenderBox(QuadTreeNode* node)
{

	D3DXVECTOR3 dest[8];
	
	//cout << "boundsMax.y:";
	//cout << node->boundsMax.y << endl;
	
	dest[0] = Vector3(node->boundsMin.x, node->boundsMin.y, node->boundsMax.z);
	dest[1] = Vector3(node->boundsMax.x, node->boundsMin.y, node->boundsMax.z);
	dest[2] = Vector3(node->boundsMin.x, node->boundsMax.y, node->boundsMax.z);
	dest[3] = Vector3(node->boundsMax.x, node->boundsMax.y, node->boundsMax.z);
	dest[4] = Vector3(node->boundsMin);
	dest[5] = Vector3(node->boundsMax.x, node->boundsMin.y, node->boundsMin.z);
	dest[6] = Vector3(node->boundsMin.x, node->boundsMax.y, node->boundsMin.z);
	dest[7] = Vector3(node->boundsMax.x, node->boundsMax.y, node->boundsMin.z);

	//
	//D3DXMATRIX world = transform->World();
	////D3DXMatrixTranspose(&world, &transform->World());
	//for (UINT i = 0; i < 8; i++)
	//	D3DXVec3TransformCoord(&dest[i], &dest[i], &world);

	Color color = node->hitted ? Color(1, 0, 0, 1) : Color(0, 0, 1, 1);
	//Front
	DebugLine::Get()->RenderLine(dest[0], dest[1], color);
	DebugLine::Get()->RenderLine(dest[1], dest[3], color);
	DebugLine::Get()->RenderLine(dest[3], dest[2], color);
	DebugLine::Get()->RenderLine(dest[2], dest[0], color);

	//Backward
	DebugLine::Get()->RenderLine(dest[4], dest[5], color);
	DebugLine::Get()->RenderLine(dest[5], dest[7], color);
	DebugLine::Get()->RenderLine(dest[7], dest[6], color);
	DebugLine::Get()->RenderLine(dest[6], dest[4], color);

	//Side
	DebugLine::Get()->RenderLine(dest[0], dest[4], color);
	DebugLine::Get()->RenderLine(dest[1], dest[5], color);
	DebugLine::Get()->RenderLine(dest[2], dest[6], color);
	DebugLine::Get()->RenderLine(dest[3], dest[7], color);

	for (auto& child : node->childs)
	{
		RenderBox(child);
	}
}

void TerrainLOD::ImGui()
{
	if (bStart) return;

	ImGui::Begin("Terrain", 0, ImGuiWindowFlags_NoMove);
	//for brushShape combo box
	if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Diffuse");
		ImGui::SameLine(100.f);
		ImGui::Text("Specular");
		ImGui::SameLine(190.0f);
		ImGui::Text("Normal");
		if (ImGui::ImageButton(baseTexture ? baseTexture->SRV() : nullptr, ImVec2(50.0f, 50.0f)))
		{
			HWND hWnd = NULL;
			function<void(wstring)> f = bind(&TerrainLOD::BaseTexture, this, placeholders::_1);
			Path::OpenFileDialog(L"", Path::ImageFilter, L"../../_Textures/Terrain/", f, hWnd);
		}
		
		ImGui::SameLine();
		if (ImGui::ImageButton(roughnessTexture ? roughnessTexture->SRV() : nullptr, ImVec2(50.0f, 50.0f)))
		{
			HWND hWnd = NULL;
			function<void(wstring)> f = bind(&TerrainLOD::RoughnessTextures, this, placeholders::_1);
			Path::OpenFileDialog(L"", Path::ImageFilter, L"../../_Textures/Terrain/", f, hWnd);
		}
		
		ImGui::SameLine();

		if (ImGui::ImageButton(normalTexture ? normalTexture->SRV() : nullptr, ImVec2(50.0f, 50.0f)))
		{
			HWND hWnd = NULL;
			function<void(wstring)> f = bind(&TerrainLOD::NormalTextures, this, placeholders::_1);
			Path::OpenFileDialog(L"", Path::ImageFilter, L"../../_Textures/Terrain/", f, hWnd);
		}

		ImGui::Separator();
	}
	const char* brushShapes[] = { "None" ,"Square", "Circle" };
	static const char* brushShape = brushShapes[0];
	if (ImGui::BeginCombo("BrushShape", brushShape))
	{
		for (uint i = 0; i < IM_ARRAYSIZE(brushShapes); i++)
		{
			bool bSelected = brushShape == brushShapes[i];
			if (ImGui::Selectable(brushShapes[i], bSelected))
			{
				brushShape = brushShapes[i];
				if (brushShape == "None")
					brushDesc.BrushShape = 0;
				else if (brushShape == "Square")
					brushDesc.BrushShape = 1;
				else if (brushShape == "Circle")
					brushDesc.BrushShape = 2;

			}

			if (bSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	//for brushtype combo box

	
	
	const char* brushNames[] = { "None","Raise", "Smoothing","Splatting","Billboard" };
	static const char* brushName = brushNames[static_cast<uint>(brushType)];
	if (ImGui::CollapsingHeader("Terrain Brush", ImGuiTreeNodeFlags_DefaultOpen))
	{

		//ImGui::Checkbox("Update", &bUpdate);
		//ImGui::SameLine();
		ImGui::PushItemWidth(100);
		ImGui::ColorEdit4("Color", (float*)&brushDesc.Color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
		ImGui::SameLine();
		ImGui::SliderInt("Range", (int*)&brushDesc.Range, 1, 100);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if(ImGui::Button("UpdateHeight", ImVec2(100, 30)))
		{
			UpdateHeightMap();
		}
		ImGui::Separator();
		if (ImGui::BeginCombo("BrushType", brushName))
		{
			for (uint i = 0; i < IM_ARRAYSIZE(brushNames); i++)
			{
				bool bSelected = brushName == brushNames[i];
				if (ImGui::Selectable(brushNames[i], bSelected))
				{
					brushName = brushNames[i];
					if (brushName == "None")
						brushType = BrushType::None;
					else if (brushName == "Raise")
						brushType = BrushType::Raise;
					else if (brushName == "Smoothing")
						brushType = BrushType::Smooth;
					else if (brushName == "Splatting")
						brushType = BrushType::Splatting;
					else if (brushName == "Billboard")
						brushType = BrushType::Billboard;

					
				}

				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();

		}
	}
	static float rfactor = 0.0f;
	if (brushType == BrushType::Raise&&brushDesc.BrushShape == 2)
	{
		ImGui::SliderFloat("BrushRadian", &rfactor, 0.0f, 50.0f);
		ImGui::Separator();
	}
	if (brushType == BrushType::Raise&&brushDesc.BrushShape == 1)
	{
		ImGui::SliderFloat("RaiseSpeed", (float*)(&updateDesc.raiseSpeed), 0.01f, 10.0f);
		
		ImGui::Separator();
	}
	if (brushType == BrushType::Billboard)
	{
		if (ImGui::CollapsingHeader("Billboard Detail", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderInt("BillBoard Count", reinterpret_cast<int*>(&billboardCount), 1, 1000);
			
			ImGui::InputFloat2("BillBoard Scale", (float*)(&billboardScale));
			static bool bTemp = false;
			//static const char* billboardName = "N/A";
			string billboardName = "N/A";
			if (ImGui::BeginCombo("Billboard", bTemp ? String::ToString(billboarfiles[billbaordIndex]).c_str() : billboardName.c_str()))// 
			{
				for (uint i = 0; i < billboarfiles.size(); i++)
				{
					ImGui::Image(billboardTexture[i]->SRV(), ImVec2(100, 100));
					ImGui::SameLine();
					bool bSelected = billboardName == String::ToString(billboarfiles[i]);
					if (ImGui::Selectable(String::ToString(billboarfiles[i]).c_str(), bSelected))
					{
						if (bTemp == false)
							bTemp = true;

						billboardName = String::ToString(billboarfiles[i]);
						billbaordIndex = i;
						selectedBillboard = billboardTextures[i];
					}


					if (bSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();

				brushBuffer->Apply();
				sBrushBuffer->SetConstantBuffer(brushBuffer->Buffer());
			}
			//ImGui::Checkbox("Eraser", &bEraser);

		}
		ImGui::Separator();
	}
	
	
	if (Mouse::Get()->Press(0))
	{
		
		
		
		
			updateDesc.Location = pos;
			updateDesc.Range = brushDesc.Range;
			updateDesc.BrushShape = brushDesc.BrushShape;
			if (sUpdateBuffer)
			{
				updateBuffer->Apply();
				sUpdateBuffer->SetConstantBuffer(updateBuffer->Buffer());
			}
		
		
		//temp = bUpdate ? false : true;
		switch (brushType)
		{
		case BrushType::Raise:
			RaiseHeight();
			break;
		case BrushType::Smooth:
			SmoothBrush();
			break;
	/*	case BrushType::Splatting:
			Splatting(mousePos, brushDesc.Type, brushDesc.Range, rfactor, splatFactor);
			break;*/
		case BrushType::Billboard:
			BillboardBrush(mousePos, brushDesc.BrushShape, brushDesc.Range, billboardCount,Vector2(billboardScale[0], billboardScale[1]));
			break;
		default:
			break;
		}

	}

		ImGui::End();
}




void TerrainLOD::ReadPixel()
{

	ID3D11Texture2D* srcTexture;
	updateSRV->GetResource((ID3D11Resource **)&srcTexture);

	


	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	srcTexture->GetDesc(&desc);
	desc.BindFlags = 0;
	desc.MiscFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Usage = D3D11_USAGE_STAGING;


	HRESULT hr;

	ID3D11Texture2D* texture;
	hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, &texture);
	Check(hr);

	//D3D::Get()->GetDC()->CopyResource(texture, srcTexture);

	hr = D3DX11LoadTextureFromTexture(D3D::GetDC(), updateTexture, NULL, texture);
	Check(hr);


	D3D11_MAPPED_SUBRESOURCE map;
	UINT* colors = new UINT[desc.Width * desc.Height];
	D3D::GetDC()->Map(texture, 0, D3D11_MAP_READ, NULL, &map);
	{
		memcpy(colors, map.pData, sizeof(UINT) * desc.Width * desc.Height);
	}
	D3D::GetDC()->Unmap(texture, 0);


	//heightMapPixel.reserve(desc.Width * desc.Height);
	for (UINT y = 0; y < desc.Height; y++)
	{
		for (UINT x = 0; x < desc.Width; x++)
		{
			UINT index = desc.Width * y + x;

			CONST FLOAT f = 1.0f / 255.0f;
			float r = f * (float)((0xFF000000 & colors[index]) >> 24);
			float g = f * (float)((0x00FF0000 & colors[index]) >> 16);
			float b = f * (float)((0x0000FF00 & colors[index]) >> 8);
			float a = f * (float)((0x000000FF & colors[index]) >> 0);

			heightMapPixel.emplace_back(D3DXCOLOR(r,g,b, a));
		}
	}

	SafeDeleteArray(colors);
	SafeRelease(srcTexture);
	SafeRelease(texture);
}
void TerrainLOD::UpdateHeightMap()
{
	
	heightMapPixel.clear();
	heightMapPixel.shrink_to_fit();
	ReadPixel();
	//ID3D11Texture2D* srcTexture;
	
	//Texture::ReadPixels(updateTexture, DXGI_FORMAT_R32_FLOAT, &heightMapPixel);
	//CalcBoundY();
	SafeDelete(root);
	root = new QuadTreeNode();
	CreateQuadTree(root, info, Vector2(0, 0), Vector2(width, height));
	
	//SafeRelease(srcTexture);
}


void TerrainLOD::RaiseHeight()
{
	sUpdate->SetUnorderedAccessView(updateUAV);
	UpdateShader->Dispatch(0, 0, 1024, 1, 1);

	
}

void TerrainLOD::SmoothBrush()
{
	sUpdate->SetUnorderedAccessView(updateUAV);
	UpdateShader->Dispatch(0, 1, 1024, 1, 1);
}

void TerrainLOD::Platting()
{
}

void TerrainLOD::Splatting(Vector3 & position, uint type, uint range, uint alpha, float factor)
{
	
}

void TerrainLOD::BillboardBrush(Vector3 & position, uint type, uint range, uint count, Vector2 scale)
{
	if (ImGui::IsAnyItemHovered())
		return;

	if (selectedBillboard == L"N/A") return;
	D3D11_BOX box;
	box.left = (uint)position.x - range;
	box.top = (uint)position.z + range;
	box.right = (uint)position.x + range;
	box.bottom = (uint)position.z - range;

	if (box.left < 0)box.left = 0;
	if (box.top >= height)box.top = height;
	if (box.right >= width)box.right = width;
	if (box.bottom < 0)box.bottom = 0;




	//auto vertices=billboard->GetVertex();

	switch (type)
	{
	case 1:
	{
		
		Vector3 p;
		//p.x = Math::Random(static_cast<int>(box.left), static_cast<int>(box.right));
	//	p.z = Math::Random(static_cast<int>(box.bottom), static_cast<int>(box.top));
		p = position;
		//Vector2 scale = Vector2(10, 10);
		p.y = position.y + (scale.y*0.5f);

		Vector2 randomScale = Math::RandomVec2(10, 100);
		
		billboard->Add(p, scale, billbaordIndex);
		billboard->CreateBuffer();
	}
	break;
	case 2:
	{
	
			for (uint i = 0; i < count; i++)
			{
				Vector3 p;
				p.x = static_cast<float>((Math::Random(static_cast<int>(box.left), static_cast<int>(box.right))));
				p.z = static_cast<float>(Math::Random(static_cast<int>(box.bottom), static_cast<int>(box.top)));
				float dx = p.x - position.x;
				float dz = p.z - position.z;

				dx *= dx;
				dz *= dz;
				float distanceSquared = dx + dz;
				float dist = sqrt(dx + dz);
				float radiusSquared = range * range;

				uint index = width * p.z + p.x;

				Vector2 scale = Math::RandomVec2(10, 20);

				//if (index < width*height&&distanceSquared <= radiusSquared)
				{
				/*	p.y = vertices[index].Position.y + (scale.y*0.5f);
					if (p.x >= 0 && p.z <= height)*/
					p.y = mousePos.y + (scale.y*0.5f);
						billboard->Add(p, scale, billbaordIndex);

				}
			}
			billboard->CreateBuffer();

		

	}
	break;

	}
}

void TerrainLOD::BillboardFinder(wstring name)
{
	vector<wstring> files;
	wstring terrainPath = L"Terrain/Billboards/";
	wstring filter = L"*.png";
	Path::GetFiles(&files, name, filter, false);
	
	for (uint i = 0; i < files.size(); i++)
	{
		auto fileName = Path::GetFileName(files[i]);
		auto noExfileName = Path::GetFileNameWithoutExtension(files[i]);
		billboardTextures.emplace_back(terrainPath + fileName);
		
		billboarfiles.emplace_back(noExfileName);
		
	}
}


float TerrainLOD::Height(float px, float pz)
{
	//node->minX = leftTop.x * info.CellSpacing - width / 2;
	//node->maxX = rightBottom.x * info.CellSpacing - width / 2;
	//node->minZ = -leftTop.y * info.CellSpacing + height / 2;
	//node->maxZ = -rightBottom.y * info.CellSpacing + height / 2;
	float c = (px + (0.5f * width)) / info.CellSpacing;
	float d = (pz - (0.5f * height)) / -info.CellSpacing;
	uint row =d;
	uint col =c;
	//uint row = static_cast<uint>(floor(d));
	//uint col = static_cast<uint>(floor(c));;
	//cout << "row:";
	//cout << row << endl;
	float h00 = 0;
	float h01 = 0;
	float h10 = 0;
	float h11 = 0;
	if(InBounds(row, col))
	{
		 h00 = heightMapPixel[row* height + col].b ;
		 h01 = heightMapPixel[row* height + (col + 1)].b ;
		 h10 = heightMapPixel[(row + 1)* height + col].b ;
		 h11 = heightMapPixel[(row + 1)* height +( col + 1)].b;
		
		
	}
	

	float s = c - col;
	float t = d - row;

	if (s + t <= 1.0f) {
		float uy = h01 - h00;
		float vy = h01 - h11;
		return h00 + (1.0f - s) * uy + (1.0f - t) * vy;
	}
	else {
		float uy = h10 - h11;
		float vy = h01 - h11;
		return h11 + (1.0f - s) * uy + (1.0f - t) * vy;
	}
		
}
bool Compare(QuadTreeNode* a, QuadTreeNode* b)
{
	return a->dist > b->dist;
}
bool QuadTreeNode::Intersection(const Vector3& org, const Vector3& dir, Vector3 & Pos)
{
	
	Pos = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	
	if (childs.empty())
	{
		float d;
		if (!IntersectionAABB(org, dir, Pos, d))
		{
			
			//cout << "Not Intersection" << endl;
			return false;
		}
		Pos = (boundsMin + boundsMax)*0.5f;
		
		
		return true;
	}
	vector<QuadTreeNode*> hittedChilds;
	//priority_queue<float> pq;
	for (auto& child : childs)
	{
		float cd=0;
		if (child->IntersectionAABB(org, dir, Pos, child->dist))
		{
			child->dist += Math::Random(-0.001f, 0.001f);
			
			//pq.push(hittedChilds.back()->dist);
			hittedChilds.emplace_back(child);
		}
	}
	

	if (hittedChilds.empty())
	{
		
		return false;
	}

	
	sort(hittedChilds.begin(), hittedChilds.end(), Compare);

	bool intersect = false;
	Vector3 bestHit =org+1000*dir;
	

	for (auto& p: hittedChilds)
	{
		Vector3 thisHit;
		
		bool wasHit = p->Intersection(org,dir, thisHit);
		if (!wasHit)
		{
			//cout << "Not was Hit" << endl;
			continue;
		}
		/*Vector3 nor;
		D3DXVec3Normalize(&nor, &(thisHit - org));
		Vector3 dotDir = dir;
		float dot = D3DXVec3Dot(&nor,&dir);
		if (dot > 1.4f||dot<0.6f) {
			cout << "False due to dot" << endl;
			continue;
		}*/
		float l1=D3DXVec3LengthSq(&(org - thisHit));
		float l2 = D3DXVec3LengthSq(&(org - bestHit));
		// check that the intersection is closer than the nearest intersection found thus far
		if (!(l1 < l2))
		{
			//cout << "check that the intersection LengthSquard" << endl;
			continue;
		}
			

		bestHit = thisHit;
		
		//SafeDelete(thisNode);
		intersect = true;
	}
	
	Pos = bestHit;
	hittedChilds.clear();
	
	return intersect;
}



bool QuadTreeNode::IntersectionAABB(const Vector3& org, const Vector3& dir, Vector3 & Pos, float& d)
{


	float t_min = 0;
	float t_max = FLT_MAX;
	

	for (int i = 0; i < 3; i++)
	{
		if (abs(dir[i]) < Math::EPSILON)
		{
			if (org[i] < boundsMin[i] ||
				org[i] >boundsMax[i])
			{
				hitted = false;
				return false;
			}

		}
		else
		{
			float denom = 1.0f / dir[i];
			float t1 = (boundsMin[i]-org[i] ) * denom;
			float t2 = ( boundsMax[i]-org[i] ) * denom;

			if (t1 > t2)
			{
				swap(t1, t2);
			}

			t_min = max(t_min, t1);
			t_max = min(t_max, t2);

			if (t_min > t_max)
			{
				hitted = false;
				return false;
			}


		}
	}
	hitted = true;
	//Vector3 hit =  org + t_min * dir;
	

	//Pos = (boundsMin + boundsMax)*0.5f;
	//;
	d = t_min;
	return true;
}
