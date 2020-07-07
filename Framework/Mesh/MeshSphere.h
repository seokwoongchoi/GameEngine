#pragma once
#include "Mesh.h"

class MeshSphere : public Mesh
{
public:
	MeshSphere(Shader* shader, float radius, UINT stackCount = 20, UINT sliceCount = 20);
	~MeshSphere();
	uint IndexCount() {return sphereIndex;}
	void SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology) { Topology(topology); }
protected:
	void Create() override;

private:
	float radius;

	UINT stackCount;
	UINT sliceCount;

	uint sphereIndex;
};