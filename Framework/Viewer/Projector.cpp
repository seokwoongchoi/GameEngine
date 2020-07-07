#include "Framework.h"
#include "Projector.h"
#include "Viewer/Fixity.h"

Projector::Projector(Shader * shader, wstring textureFile, UINT width, UINT height)
	: shader(shader), width(width), height(height)
{
	camera = new Fixity();
	/*camera->RotationDegree(90, 0,0);
	camera->Position(0, 10, 0);*/
	camera->RotationDegree(45, 0, 0);
	camera->Position(0, 10, -10); 

	projection = new Perspective((float)width, (float)height, 0.1f, 100.0f, Math::PI * 0.25f);
	//projection = new Orthographic((float)width, (float)height);
	texture = new Texture(textureFile);
	buffer = new ConstantBuffer(&desc, sizeof(Desc));


	sMap = shader->AsSRV("ProjectorMap");
	sMap->SetResource(texture->SRV());

	sBuffer = shader->AsConstantBuffer("CB_Projector");
}
Projector::~Projector()
{
	SafeDelete(camera);
	SafeDelete(projection);

	SafeDelete(texture);
	SafeDelete(buffer);
}

void Projector::Update()
{
	Vector3 position;
	camera->Position(&position);

	ImGui::SliderFloat3("Position", (float *)&position, -40, 1000);
	camera->Position(position);


	ImGui::InputInt("Width", (int *)&width);
	width = width < 1 ? 1 : width;

	ImGui::InputInt("Height", (int *)&height);
	height = height < 1 ? 1 : height;

	((Perspective *)projection)->Set((float)width, (float)height);

	ImGui::ColorEdit4("Color", (float *)&desc.Color);


	camera->GetMatrix(&desc.ProjectorView);
	projection->GetMatrix(&desc.Projection2);

	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());
}