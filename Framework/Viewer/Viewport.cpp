#include "Framework.h"
#include "Viewport.h"

Viewport::Viewport(float width, float height, float x, float y, float minDepth, float maxDepth)
{
	Set(width, height, x, y, minDepth, maxDepth);
}

Viewport::~Viewport()
{
	
}

void Viewport::Set(float width, float height, float x, float y, float minDepth, float maxDepth)
{
	viewport.TopLeftX = this->x = x;
	viewport.TopLeftY = this->y = y;
	viewport.Width = this->width = width;
	viewport.Height = this->height = height;
	viewport.MinDepth = this->minDepth = minDepth;
	viewport.MaxDepth = this->maxDepth = maxDepth;

	RSSetViewport();
}

void Viewport::GetRay(OUT Vector3 * position, OUT Vector3 * direction, IN Matrix & w, IN Matrix & v, IN Matrix & p)
{
	Vector3 mouse = Mouse::Get()->GetPosition();

	Vector2 point;
	//Inv Viewport
	{
		point.x = (((2.0f*mouse.x) / width) - 1.0f);
		point.y = (((2.0f*mouse.y) / height) - 1.0f)*-1.0f;
		
	}

	//Inv Projection
	{
		point.x = point.x / p._11;
		point.y = point.y / p._22;
	}

	Vector3 cameraPosition;
	//inv View
	{
		Matrix invView;
		D3DXMatrixInverse(&invView, nullptr, &v);

		cameraPosition = Vector3(invView._41, invView._42, invView._43);

		D3DXVec3TransformNormal(direction, &Vector3(point.x, point.y, 1), &invView);
		D3DXVec3Normalize(direction, direction);
	}
	//inv world
	{
		Matrix invWorld;
		D3DXMatrixInverse(&invWorld, nullptr, &w);

		D3DXVec3TransformCoord(position, &cameraPosition, &invWorld); //������ ���� ��ü�� ī�޶��� ������ ��ġ�����ֱ����ؼ� 
		D3DXVec3TransformNormal(direction, direction,&invWorld);//������ ���� ��ü�� ī�޶��� ������ ��ġ�����ֱ����ؼ�
		D3DXVec3Normalize(direction, direction);
		D3DXVec3TransformCoord(position, position, &invWorld);
	}
}

void Viewport::Unprojection(OUT Vector3* position, Vector3 & source, Matrix & W, Matrix & V, Matrix & P)
{
	Vector3 temp = source;

	position->x = ((temp.x - x) / width)*2.0f - 1.0f;
	position->y = (((temp.y - y) / height)*2.0f - 1.0f)*-1.0f;
	position->z = (temp.z - minDepth / (maxDepth - minDepth));

	Matrix wvp = W * V*P;
	D3DXMatrixInverse(&wvp, nullptr, &wvp);
	D3DXVec3TransformCoord(position, position, &wvp);
}

void Viewport::Projection(OUT Vector3 * position, Vector3 & source, Matrix & W, Matrix & V, Matrix & P)
{
	Matrix wvp = W * V*P;
	
	Vector3 temp = source;
	D3DXVec3TransformCoord(position, &temp, &wvp);
	

	position->x = ((position->x + 1.0f)*0.5f)*width + x;
	position->y = ((-position->y + 1.0f)*0.5f)*height + y;
	position->z=(position->z*(maxDepth-minDepth))+minDepth;

}




void Viewport::RSSetViewport()
{
	D3D::GetDC()->RSSetViewports(1, &viewport);
}