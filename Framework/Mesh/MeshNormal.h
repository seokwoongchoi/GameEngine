#pragma once
#include "Mesh.h"

class MeshNormal: public Mesh
{
public:
	MeshNormal(Shader* shader);
	~MeshNormal();
	
protected:
	void Create() override;

private:
	vector<MeshVertex> v;
};
