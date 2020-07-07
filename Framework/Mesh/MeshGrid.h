#pragma once
#include "Mesh.h"

class MeshGrid : public Mesh
{
public:
	MeshGrid(Shader* shader, float offsetU=1.0f, float offsetV=1.0f);
	~MeshGrid();
	bool Picked(Vector3& position);
	void SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology) { Topology(topology); }
protected:
	void Create() override;

private:
	float offsetU;
	float offsetV;
	vector<MeshVertex> v;
};
