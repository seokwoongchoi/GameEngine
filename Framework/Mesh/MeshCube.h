#pragma once
#include "Mesh.h"

class MeshCube : public Mesh
{
public:
	MeshCube(Shader* shader);
	~MeshCube();

	
	MeshVertex* GetVertiecs() { return v.data(); }
	void SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology) { Topology(topology); }
protected:
	void Create() override;
	
	
private:
	vector<MeshVertex> v;

	uint* nIndices;
	uint nIndexCount;
};
