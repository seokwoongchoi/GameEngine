#include "Framework.h"
#include "Orbit.h"

Orbit::Orbit()
	:distance(30),R(0.375414,-0.562102f,0)
{
	D3DXMatrixIdentity(&matrixR);


}

Orbit::~Orbit()
{
}

void Orbit::Update()
{
	
	R.y += (moveValue.y *0.15f)*Time::Delta();
	R.x += (moveValue.x *0.15f)*Time::Delta();
	
	
	float dist = D3DXVec3Length(&deltaPos);
	
	
	position.x = targetPosition.x +dist * sinf(R.y)*cosf(-R.x-89.2f);
	position.y = targetPosition.y +dist * cosf(R.y);
	position.z = targetPosition.z +dist * sinf(R.y)*sinf(-R.x - 89.2f);
	
	Position(position);
	

	
	D3DXVec3Normalize(&forward,&(targetPosition-position));
	Forward(forward);
	
}

void Orbit::PreviewUpdate()
{
	Vector2 val = Vector2(ImGui::GetMouseDragDelta(1).x, ImGui::GetMouseDragDelta(1).y);
	if (ImGui::IsAnyWindowHovered())
	{
		if (ImGui::IsMouseDown(1))
		{

			R.y += (val.y / 20)*Time::Delta();
			R.x += (val.x / 20)*Time::Delta();
			//R.x += Math::ToRadian(val.x*Time::Delta()*2);
			R.z = 0.0f;
		}

		
		if (ImGui::IsKeyPressed('W'))//&& distance > 0
		{
			distance -= 1.0f;
		}
		else if (ImGui::IsKeyPressed('S') && distance < 300)
		{
			distance += 1.0f;
		}

	}
	

	position.x = targetPosition.x + distance * sinf(R.y)*cosf(R.x);
	position.y = targetPosition.y + distance * cosf(R.y);
	position.z = targetPosition.z + distance * sinf(R.y)*sinf(R.x);

	Position(position);
}
