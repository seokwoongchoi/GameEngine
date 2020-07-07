#include "Framework.h"
#include "MeshNormal.h"

MeshNormal::MeshNormal(Shader * shader)
	: Mesh(shader)
{
	Topology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	topology = topologyType::LINELIST;
}

MeshNormal::~MeshNormal()
{

}



void MeshNormal::Create()
{
	//forward normal
	v.emplace_back(MeshVertex(0,0,0,0,0,0,0,1,0,0,0));
	v.emplace_back(MeshVertex(0, 0, 3, 0, 0, 0, 0, 1, 0, 0, 0));
	
	//up normal
	v.emplace_back(MeshVertex(0, 0, 0, 0, 0, 0, 1, 0,0,0,0));
	v.emplace_back(MeshVertex(0, 2, 0, 0, 0, 0, 1, 0,0,0,0));
	
	//right normal
	v.emplace_back(MeshVertex(0, 0, 0, 0, 0, 1, 0, 0,0,0,0));
	v.emplace_back(MeshVertex(2, 0, 0, 0, 0, 1, 0, 0,0,0,0));

	vertices = new MeshVertex[v.size()];
	vertexCount = v.size();

	copy
	(
		v.begin(), v.end(),
		stdext::checked_array_iterator<MeshVertex *>(vertices, vertexCount)
	);

	indexCount = 36;
	this->indices = new UINT[indexCount]
	{
	   0, 1, 2,3, 4,5
	};
}