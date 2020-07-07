#pragma once
#include "Mesh.h"
class MeshQuad :public Mesh
{
public:
	MeshQuad(Shader* shader);
	 ~MeshQuad();

private:
	// Mesh을(를) 통해 상속됨
	void Create() override;
private:
	Shader* shader;

};
