#pragma once
#include "Mesh.h"
class MeshQuad :public Mesh
{
public:
	MeshQuad(Shader* shader);
	 ~MeshQuad();

private:
	// Mesh��(��) ���� ��ӵ�
	void Create() override;
private:
	Shader* shader;

};
