#include "Framework.h"
#include "MeshGrid.h"

MeshGrid::MeshGrid(Shader * shader, float offsetU, float offsetV)
	: Mesh(shader), offsetU(offsetU), offsetV(offsetV)
{
	
}

MeshGrid::~MeshGrid()
{

}

bool MeshGrid::Picked(Vector3& position)
{
	if (v.empty()) return false;
	Vector3 org, dir;
	Matrix V = Context::Get()->View();
	Matrix P = Context::Get()->Projection();

	Matrix world = GetTransform()->World();
	Context::Get()->GetViewport()->GetRay(&org,&dir,world,V,P);

	

	for (uint z = 0; z < 11 - 1; z++)
	{
		for (uint x = 0; x < 11 - 1; x++)
		{
			uint index[4];

			index[0] =11 * z + x;
			index[1] =11 * (z + 1) + x;
			index[2] =11 * z + (x + 1);
			index[3] =11 * (z + 1) + (x + 1);

			Vector3 v1[4];
			for (int i = 0; i < 4; i++)
			{
				v1[i] = v[index[i]].Position;

			}

			float u, v2, distance;


			if (D3DXIntersectTri(&v1[0], &v1[1], &v1[2], &org, &dir, &u, &v2, &distance))
			{
			     
				position = org + distance * dir;
				return true;


			}

			else if (D3DXIntersectTri(&v1[3], &v1[1], &v1[2], &org, &dir, &u, &v2, &distance))
			{
				
				position = org + distance * dir;
				return true;
			}

		}
	}

	return false;
}

void MeshGrid::Create()
{
	UINT countX = 11;
	UINT countZ = 11;

	float w = ((float)countX - 1) * 0.5f;
	float h = ((float)countZ - 1) * 0.5f;

	
	for (UINT z = 0; z < countZ; z++)
	{
		for (UINT x = 0; x < countX; x++)
		{
			MeshVertex vertex;
			vertex.Position = D3DXVECTOR3((float)x - w, 0, (float)z - h);
			vertex.Normal = D3DXVECTOR3(0, 1, 0);
			vertex.Tangent = D3DXVECTOR3(1, 0, 0);

			vertex.Uv.x = (float)x / (float)(countX - 1) * offsetU;
			vertex.Uv.y = (float)z / (float)(countZ - 1) * offsetV;
			
			v.push_back(vertex);
		}
	}


	vertices = new MeshVertex[v.size()];
	vertexCount = v.size();
	copy
	(
		v.begin(), v.end(),
		stdext::checked_array_iterator<MeshVertex *>(vertices, vertexCount)
	);


	vector<UINT> indices;
	for (UINT z = 0; z < countZ - 1; z++)
	{
		for (UINT x = 0; x < countX - 1; x++)
		{
			indices.push_back(countX * z + x);
			indices.push_back(countX * (z + 1) + x);
			indices.push_back(countX * z + x + 1);

			indices.push_back(countX * z + x + 1);
			indices.push_back(countX * (z + 1) + x);
			indices.push_back(countX * (z + 1) + x + 1);
		}
	}

	this->indices = new UINT[indices.size()];
	indexCount = indices.size();
	copy
	(
		indices.begin(), indices.end(),
		stdext::checked_array_iterator<UINT *>(this->indices, indexCount)
	);
}