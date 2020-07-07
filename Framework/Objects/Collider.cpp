#include "Framework.h"
#include "Collider.h"

Collider::Collider(Transform * transform, Transform* init)
	:transform(transform),init(init)
{
	lines[0] = D3DXVECTOR3(-0.5f, -0.5f, -0.5f); 
	lines[1] = D3DXVECTOR3(-0.5f, +0.5f, -0.5f); 
	lines[2] = D3DXVECTOR3(+0.5f, -0.5f, -0.5f); 
	lines[3] = D3DXVECTOR3(+0.5f, +0.5f, -0.5f); 
	lines[4] = D3DXVECTOR3(-0.5f, -0.5f, +0.5f);   
	lines[5] = D3DXVECTOR3(-0.5f, +0.5f, +0.5f);   
	lines[6] = D3DXVECTOR3(+0.5f, -0.5f, +0.5f);   
	lines[7] = D3DXVECTOR3(+0.5f, +0.5f, +0.5f); 

	
}

Collider::~Collider()
{
}

bool Collider::Isintersect(Collider * other)
{
	
	return Collision(this->obb,other->obb);
}

void Collider::Render(Color color, vector<Vector3> lines)
{
	if (lines.empty()) return;

	D3DXVECTOR3 dest[8];
	D3DXMATRIX world = transform->World();
	//D3DXMatrixTranspose(&world, &transform->World());
	for (UINT i = 0; i < 8; i++)
		D3DXVec3TransformCoord(&dest[i], &lines[i], &world);


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
}

void Collider::Update()
{
	SetObb();
}

void Collider::SetObb()
{
	Transform temp;
	temp.WorldDecompose(transform->World());
	
	if (init != NULL)
	{
		temp.WorldDecompose(init->World()*transform->World());
	}
	
	temp.Position(&obb.Position);
	D3DXVec3Normalize(&obb.AxisX, &temp.Right());
	D3DXVec3Normalize(&obb.AxisY, &temp.Up());
	D3DXVec3Normalize(&obb.AxisZ, &temp.Direction());
	
	Vector3 scale;
	temp.Scale(&scale);
	obb.HalfSize = scale * 0.5f;
}

bool Collider::SperatingPlane(Vector3 & position, Vector3 & direction, Obb & box1, Obb & box2)
{
	float val = fabsf(D3DXVec3Dot(&position, &direction));

	float val2 = 0.0f;
	val2 += fabsf(D3DXVec3Dot(&(box1.AxisX * box1.HalfSize.x), &direction));
	val2 += fabsf(D3DXVec3Dot(&(box1.AxisY * box1.HalfSize.y), &direction));
	val2 += fabsf(D3DXVec3Dot(&(box1.AxisZ * box1.HalfSize.z), &direction));
	val2 += fabsf(D3DXVec3Dot(&(box2.AxisX * box2.HalfSize.x), &direction));
	val2 += fabsf(D3DXVec3Dot(&(box2.AxisY * box2.HalfSize.y), &direction));
	val2 += fabsf(D3DXVec3Dot(&(box2.AxisZ * box2.HalfSize.z), &direction));

	return val > val2;
}

bool Collider::Collision(Obb & box1, Obb & box2)
{
	D3DXVECTOR3 position = box2.Position - box1.Position;

	if (SperatingPlane(position, box1.AxisX, box1, box2) == true) return false;
	if (SperatingPlane(position, box1.AxisY, box1, box2) == true) return false;
	if (SperatingPlane(position, box1.AxisZ, box1, box2) == true) return false;
	if (SperatingPlane(position, box2.AxisX, box1, box2) == true) return false;
	if (SperatingPlane(position, box2.AxisY, box1, box2) == true) return false;
	if (SperatingPlane(position, box2.AxisZ, box1, box2) == true) return false;
	if (SperatingPlane(position, Cross(box1.AxisX, box2.AxisX), box1, box2) == true) return false;	
	if (SperatingPlane(position, Cross(box1.AxisX, box2.AxisY), box1, box2) == true) return false;	
	if (SperatingPlane(position, Cross(box1.AxisX, box2.AxisZ), box1, box2) == true) return false;																		 
	if (SperatingPlane(position, Cross(box1.AxisY, box2.AxisX), box1, box2) == true) return false;
	if (SperatingPlane(position, Cross(box1.AxisY, box2.AxisY), box1, box2) == true) return false;
	if (SperatingPlane(position, Cross(box1.AxisY, box2.AxisZ), box1, box2) == true) return false;				 
	if (SperatingPlane(position, Cross(box1.AxisZ, box2.AxisX), box1, box2) == true) return false;
	if (SperatingPlane(position, Cross(box1.AxisZ, box2.AxisY), box1, box2) == true) return false;
	if (SperatingPlane(position, Cross(box1.AxisZ, box2.AxisZ), box1, box2) == true) return false;
	


	return true;
}

Vector3 Collider::Cross(Vector3 & vec1, Vector3 & vec2)
{
	float x = vec1.y*vec2.z - vec1.z*vec2.y;
	float y = vec1.z*vec2.x - vec1.x*vec2.z;
	float z = vec1.x*vec2.y - vec1.y*vec2.x;

	return Vector3(x,y,z);
}
