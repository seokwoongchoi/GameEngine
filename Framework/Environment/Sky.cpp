#include "Framework.h"
#include "Sky.h"


Sky::Sky()
	:cubeSRV(NULL)
{
	shader = new Shader(L"022_Sky.fx");

	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	sBuffer = shader->AsConstantBuffer("CB_Sky");

	sphere = new MeshSphere(shader, 0.5f);
	//sphere->GetTransform()->Scale(5, 5, 5);
}

Sky::Sky(wstring CubeMapFile)
{
	shader = new Shader(L"022_Sky.fx");

	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	sBuffer = shader->AsConstantBuffer("CB_Sky");

	sphere = new MeshSphere(shader, 0.5f);
	sphere->GetTransform()->Scale(1000, 1000, 1000);
	sphere->Pass(2);
	//sphere->GetTransform()->Scale(5, 5, 5);
	wstring temp = L"../../_Textures/" + CubeMapFile;
	D3DX11CreateShaderResourceViewFromFile
	(
		D3D::GetDevice(),temp.c_str(),NULL,NULL,&cubeSRV,NULL
	);
	sCubeSRV = shader->AsSRV("SkyCubeMap");
}


Sky::~Sky()
{
	SafeDelete(shader);
	SafeDelete(sphere);
	SafeDelete(buffer);
}

void Sky::Update()
{/*
	ImGui::ColorEdit3("Center", (float*)&desc.Center);
	ImGui::ColorEdit3("Apex", (float*)&desc.Apex);
	ImGui::InputFloat("Sky Height", &desc.Height, 0.1f);*/
	Vector3 Position;
	Context::Get()->GetCamera()->Position(&Position);
	sphere->GetTransform()->Position(Position);
	sphere->Update();
	
}

void Sky::Render()
{
	//D3D::GetDC()->RSSetState();
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());
	sCubeSRV->SetResource(cubeSRV);
	sphere->Render();
}
