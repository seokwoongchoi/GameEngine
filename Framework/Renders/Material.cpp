#include "Framework.h"
#include "Material.h"

Material::Material()
{
	Initialize();
}

Material::Material(Shader * shader)
{
	Initialize();

	SetShader(shader);
}

void Material::Initialize()
{
	name = L"";

	diffuseMap = NULL;
	specularMap = NULL;
	normalMap = NULL;
	roughnessMap = NULL;
	matallicMap = NULL;

	buffer = new ConstantBuffer(&colorDesc, sizeof(ColorDesc));
}

Material::~Material()
{
	SafeDelete(diffuseMap);
	SafeDelete(specularMap);
	SafeDelete(normalMap);
	SafeDelete(roughnessMap);
	SafeDelete(matallicMap);
	SafeDelete(buffer);
}

void Material::SetShader(Shader * shader)
{
	this->shader = shader;

	sBuffer = shader->AsConstantBuffer("CB_Material");

	sDiffuseMap = shader->AsSRV("DiffuseMap");
	sSpecularMap = shader->AsSRV("SpecularMap");
	sNormalMap = shader->AsSRV("NormalMap");
	sRoughnessMap = shader->AsSRV("RoughnessMap");
	sMatallicMap = shader->AsSRV("MatallicMap");

	sDiffuseMaps = shader->AsSRV("DiffuseMaps");
	sSpecularMaps = shader->AsSRV("SpecularMaps");
	sNormalMaps = shader->AsSRV("NormalMaps");
	sRoughnessMaps = shader->AsSRV("RoughnessMaps");
	sMatallicMaps = shader->AsSRV("MatallicMaps");
}

void Material::Ambient(Color & color)
{
	colorDesc.Ambient = color;
}

void Material::Ambient(float r, float g, float b, float a)
{
	Ambient(Color(r, g, b, a));
}

void Material::Diffuse(Color & color)
{
	colorDesc.Diffuse = color;
}

void Material::Diffuse(float r, float g, float b, float a)
{
	Diffuse(Color(r, g, b, a));
}

void Material::Specular(Color & color)
{
	colorDesc.Specular = color;
}

void Material::Specular(float r, float g, float b, float a)
{
	Specular(Color(r, g, b, a));
}

//void Material::Shininess(float val)
//{
//	colorDesc.Shininess = val;
//}

void Material::Roughness(float val)
{
	colorDesc.Roughness = val;
}

void Material::Matallic(float val)
{
	colorDesc.Matallic = val;
}

void Material::DiffuseMap(string file)
{
	DiffuseMap(String::ToWString(file));
}

void Material::DiffuseMap(wstring file)
{
	SafeDelete(diffuseMap);

	diffuseMap = new Texture(file);

	
	//D3D11_TEXTURE2D_DESC desc;
	//ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	//->GetDesc(&desc);

	//desc.MipLevels = 5;
	//desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	//D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	//diffuseMap->SRV()->GetDesc(&srvDesc);
	//srvDesc.Texture2D.MostDetailedMip = 0;
	//srvDesc.Texture2D.MipLevels = 5;
	//
	//Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &dTexture));
	////srvDesc.TextureCube.MipLevels = maxMipLevels;
	//
	//
	//Check(D3D::GetDevice()->CreateShaderResourceView(diffuseMap->GetTexture(), &srvDesc, &dSrv));
	//SafeDelete(diffuseMap);
	//D3D::GetDC()->GenerateMips(diffuseMap->SRV());
	isDiffuseMap = true;
}

void Material::DiffuseMaps(wstring file)
{
	diffuseNames.emplace_back(file);
}

void Material::SpecularMap(string file)
{
	SpecularMap(String::ToWString(file));
}

void Material::SpecularMap(wstring file)
{
	SafeDelete(specularMap);

	specularMap = new Texture(file);
	//D3D11_TEXTURE2D_DESC desc;
	//specularMap->GetTexture()->GetDesc(&desc);
	//desc.MipLevels = 5;
	//desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	//D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	//specularMap->SRV()->GetDesc(&srvDesc);
	//srvDesc.Texture2D.MostDetailedMip = 0;
	//srvDesc.Texture2D.MipLevels = 5;
	////srvDesc.TextureCube.MipLevels = maxMipLevels;
	//Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &sTexture));
	//Check(D3D::GetDevice()->CreateShaderResourceView(sTexture, &srvDesc, &sSrv));
	//SafeDelete(specularMap);
	//D3D::GetDC()->GenerateMips(specularMap->SRV());
	isSpecularMap = true;
}

void Material::SpecularMaps(wstring file)
{
	specularNames.emplace_back(file);
}

void Material::NormalMap(string file)
{
	NormalMap(String::ToWString(file));
}

void Material::NormalMap(wstring file)
{
	SafeDelete(normalMap);


	normalMap = new Texture(file);
	//D3D11_TEXTURE2D_DESC desc;
	//normalMap->GetTexture()->GetDesc(&desc);
	//desc.MipLevels = 5;
	//desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	//D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	//normalMap->SRV()->GetDesc(&srvDesc);
	//srvDesc.Texture2D.MostDetailedMip = 0;
	//srvDesc.Texture2D.MipLevels = 5;
	////srvDesc.TextureCube.MipLevels = maxMipLevels;
	//Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &nTexture));
	//Check(D3D::GetDevice()->CreateShaderResourceView(nTexture, &srvDesc, &nSrv));
	//SafeDelete(normalMap);
//	D3D::GetDC()->GenerateMips(normalMap->SRV());
	isNormalMap = true;
}

void Material::NormalMaps(wstring file)
{
	normalNames.emplace_back(file);
}

void Material::RoughnessMap(string file)
{

	RoughnessMap(String::ToWString(file));
}

void Material::RoughnessMap(wstring file)
{

	SafeDelete(roughnessMap);
	

	roughnessMap = new Texture(file);
	//D3D11_TEXTURE2D_DESC desc;
	//roughnessMap->GetTexture()->GetDesc(&desc);
	//desc.MipLevels = 5;
	//desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	//D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	//roughnessMap->SRV()->GetDesc(&srvDesc);
	//srvDesc.Texture2D.MostDetailedMip = 0;
	//srvDesc.Texture2D.MipLevels = 5;
	////srvDesc.TextureCube.MipLevels = maxMipLevels;
	//Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &rTexture));
	//Check(D3D::GetDevice()->CreateShaderResourceView(rTexture, &srvDesc, &rSrv));
	//SafeDelete(roughnessMap);
	//D3D::GetDC()->GenerateMips(roughnessMap->SRV());
	isRoughnessMap = true;
}

void Material::RoughnessMaps(wstring file)
{
   roughnessNames.emplace_back(file);
}

void Material::MatallicMap(string file)
{

	MatallicMap(String::ToWString(file));
}

void Material::MatallicMap(wstring file)
{
	SafeDelete(matallicMap);
	
	
	matallicMap = new Texture(file);
	//D3D11_TEXTURE2D_DESC desc;
	//matallicMap->GetTexture()->GetDesc(&desc);
	//desc.MipLevels = 5;
	//desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	//D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	//matallicMap->SRV()->GetDesc(&srvDesc);
	//srvDesc.Texture2D.MostDetailedMip = 0;
	//srvDesc.Texture2D.MipLevels = 5;
	////srvDesc.TextureCube.MipLevels = maxMipLevels;
	//Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &mTexture));
	//Check(D3D::GetDevice()->CreateShaderResourceView(mTexture, &srvDesc, &mSrv));
	//SafeDelete(matallicMap);
//	D3D::GetDC()->GenerateMips(matallicMap->SRV());
	isMatallicMap = true;
}

void Material::MatallicMaps(wstring file)
{
	matallicNames.emplace_back(file);
}

void Material::Render()
{
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	if (isDiffuseMap)
		sDiffuseMap->SetResource(diffuseMap->SRV());
	else
		sDiffuseMap->SetResource(NULL);

	if (isSpecularMap)
		sSpecularMap->SetResource(specularMap->SRV());
	else
		sSpecularMap->SetResource(NULL);

	if (isNormalMap)
		sNormalMap->SetResource(normalMap->SRV());
	else
		sNormalMap->SetResource(NULL);

	if (isRoughnessMap)
		sRoughnessMap->SetResource(roughnessMap->SRV());
	else
		sRoughnessMap->SetResource(NULL);

	if (isMatallicMap)
		sMatallicMap->SetResource(matallicMap->SRV());
	else
		sMatallicMap->SetResource(NULL);
	/*if (isDiffuseMap)
		sDiffuseMap->SetResource(dSrv);
	else
		sDiffuseMap->SetResource(NULL);

	if (isSpecularMap)
		sSpecularMap->SetResource(sSrv);
	else
		sSpecularMap->SetResource(NULL);

	if (isNormalMap)
		sNormalMap->SetResource(nSrv);
	else
		sNormalMap->SetResource(NULL);

	if (isRoughnessMap)
		sRoughnessMap->SetResource(rSrv);
	else
		sRoughnessMap->SetResource(NULL);

	if (isMatallicMap)
		sMatallicMap->SetResource(mSrv);
	else
		sMatallicMap->SetResource(NULL);*/
}

void Material::Renders()
{
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	if (diffuseMaps != NULL)
		sDiffuseMaps->SetResource(diffuseMaps->SRV());
	else
		sDiffuseMaps->SetResource(NULL);

	if (specularMaps != NULL)
		sSpecularMaps->SetResource(specularMaps->SRV());
	else
		sSpecularMaps->SetResource(NULL);

	if (normalMaps != NULL)
		sNormalMaps->SetResource(normalMaps->SRV());
	else
		sNormalMaps->SetResource(NULL);

	if (roughnessMaps != NULL)
		sRoughnessMaps->SetResource(roughnessMaps->SRV());
	else
		sRoughnessMaps->SetResource(NULL);

	if (matallicMaps != NULL)
		sMatallicMaps->SetResource(matallicMaps->SRV());
	else
		sMatallicMaps->SetResource(NULL);
}
