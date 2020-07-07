#include "Framework.h"
#include "MeshCollider.h"




MeshCollider::MeshCollider(Shader * shader, VertexType vertexType)
	: Mesh(shader)
{
	this->vertexType = vertexType;
}

MeshCollider::~MeshCollider()
{
}

void MeshCollider::SetColor(bool bPicked)
{
}

void MeshCollider::Create()
{


	float w = 0.5f;
	float h = 0.5f;
	float d = 0.5f;

	//Front
	v.push_back(VertexColor(-w, -h, -d, 0, 1, 0));
	v.push_back(VertexColor(-w, +h, -d, 0, 1, 0));
	v.push_back(VertexColor(+w, +h, -d, 0, 1, 0));
	v.push_back(VertexColor(+w, -h, -d, 0, 1, 0));

	//Back
	v.push_back(VertexColor(-w, -h, +d, 0, 1, 0));
	v.push_back(VertexColor(+w, -h, +d, 0, 1, 0));
	v.push_back(VertexColor(+w, +h, +d, 0, 1, 0));
	v.push_back(VertexColor(-w, +h, +d, 0, 1, 0));

	//Top
	v.push_back(VertexColor(-w, +h, -d, 0, 1, 0));
	v.push_back(VertexColor(-w, +h, +d, 0, 1, 0));
	v.push_back(VertexColor(+w, +h, +d, 0, 1, 0));
	v.push_back(VertexColor(+w, +h, -d, 0, 1, 0));

	//Bottom
	v.push_back(VertexColor(-w, -h, -d, 0, 1, 0));
	v.push_back(VertexColor(+w, -h, -d, 0, 1, 0));
	v.push_back(VertexColor(+w, -h, +d, 0, 1, 0));
	v.push_back(VertexColor(-w, -h, +d, 0, 1, 0));

	//Left
	v.push_back(VertexColor(-w, -h, +d, 0, 1, 0));
	v.push_back(VertexColor(-w, +h, +d, 0, 1, 0));
	v.push_back(VertexColor(-w, +h, -d, 0, 1, 0));
	v.push_back(VertexColor(-w, -h, -d, 0, 1, 0));

	//Right
	v.push_back(VertexColor(+w, -h, -d, 0, 1, 0));
	v.push_back(VertexColor(+w, +h, -d, 0, 1, 0));
	v.push_back(VertexColor(+w, +h, +d, 0, 1, 0));
	v.push_back(VertexColor(+w, -h, +d, 0, 1, 0));


	cVertices = new VertexColor[v.size()];
	vertexCount = v.size();

	copy
	(
		v.begin(), v.end(),
		stdext::checked_array_iterator<VertexColor *>(cVertices, vertexCount)
	);

	indexCount = 36;
	this->indices = new UINT[indexCount]
	{
	   0, 1, 2, 0, 2, 3,
	   4, 5, 6, 4, 6, 7,
	   8, 9, 10, 8, 10, 11,
	   12, 13, 14, 12, 14, 15,
	   16, 17, 18, 16, 18, 19,
	   20, 21, 22, 20, 22, 23
	};
}

bool MeshCollider::ModelPicking(IN const Vector3 & position, IN const Vector3 & scale, IN const Vector3 & rotation, OUT Vector3 outPsotion)
{
	Matrix V = Context::Get()->View();
	Matrix P = Context::Get()->Projection();
	Matrix W = GetTransform()->World();
	Vector3 org, dir;
	Context::Get()->GetViewport()->GetRay(&org, &dir, W, V, P);
	float u, v1, distance;
	
	if(v.size()>0)
	//front
	if (D3DXIntersectTri(&v[0].Position, &v[1].Position,&v[2].Position,	&org, &dir, &u, &v1, &distance))
	{
		
		outPsotion=org + distance * dir;
		return true;
	}
	else if (D3DXIntersectTri(&v[0].Position, &v[2].Position, &v[3].Position, &org, &dir, &u, &v1, &distance))
	{

		outPsotion = org + distance * dir;
		return true;
	}

	//back
	else if (D3DXIntersectTri(&v[4].Position, &v[5].Position, &v[6].Position, &org, &dir, &u, &v1, &distance))
	{

		outPsotion = org + distance * dir;
		return true;
	}
	else if (D3DXIntersectTri(&v[4].Position, &v[6].Position, &v[7].Position, &org, &dir, &u, &v1, &distance))
	{

		outPsotion = org + distance * dir;
		return true;
	}

	//top
	else if (D3DXIntersectTri(&v[8].Position, &v[9].Position, &v[10].Position, &org, &dir, &u, &v1, &distance))
	{

		outPsotion = org + distance * dir;
		return true;
	}
	else if (D3DXIntersectTri(&v[8].Position, &v[10].Position, &v[11].Position, &org, &dir, &u, &v1, &distance))
	{

		outPsotion = org + distance * dir;
		return true;
	}
	
	
	//Bottom
	else if (D3DXIntersectTri(&v[12].Position, &v[13].Position, &v[14].Position, &org, &dir, &u, &v1, &distance))
	{

		outPsotion = org + distance * dir;
		return true;
	}
	else if (D3DXIntersectTri(&v[12].Position, &v[14].Position, &v[15].Position, &org, &dir, &u, &v1, &distance))
	{

		outPsotion = org + distance * dir;
		return true;
	}
	//Left
	else if (D3DXIntersectTri(&v[16].Position, &v[17].Position, &v[18].Position, &org, &dir, &u, &v1, &distance))
	{

		outPsotion = org + distance * dir;
		return true;
	}
	else if (D3DXIntersectTri(&v[16].Position, &v[18].Position, &v[19].Position, &org, &dir, &u, &v1, &distance))
	{

		outPsotion = org + distance * dir;
		return true;
	}
	//right
	else if (D3DXIntersectTri(&v[20].Position, &v[21].Position, &v[22].Position, &org, &dir, &u, &v1, &distance))
	{

		outPsotion = org + distance * dir;
		return true;
	}
	else if (D3DXIntersectTri(&v[20].Position, &v[22].Position, &v[23].Position, &org, &dir, &u, &v1, &distance))
	{

		outPsotion = org + distance * dir;
		return true;
	}
	
	
	
	return false;
}

