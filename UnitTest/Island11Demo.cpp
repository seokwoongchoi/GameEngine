#include "stdafx.h"
#include "Island11Demo.h"


#include "Environment/Island11.h"


void Island11Demo::Initialize()
{
	//	selectT = new Transform();

	float width = D3D::Width();
	float height = D3D::Height();

	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 46, -85);
	//island11 = new Island11();
	//TerrainLOD::InitializeInfo info =
}

void Island11Demo::Update()
{
}

void Island11Demo::Render()
{

	island11->Update();
	//island11->Render();
}
