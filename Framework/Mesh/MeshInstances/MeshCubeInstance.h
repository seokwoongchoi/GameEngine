#pragma once
#define MAX_INSTANCE 4096 //�迭�� �ִ� ����
#include "Renders/Renderer.h"
class MeshCubeInstance :public Renderer
{
public:
	typedef VertexTextureNormal MeshVertex;
public:
	MeshCubeInstance();

	~MeshCubeInstance();

	void Update();
	void Render();

	uint Push();//�ѹ� ȣ��� drawCount++
	Transform* GetTransform(uint index);
private:
	void CreateCubeVertex();
private:
	uint drawCount;

	Transform* transforms[MAX_INSTANCE];

	Matrix worlds[MAX_INSTANCE];


	VertexBuffer* instanceBuffer;

	vector<MeshVertex> v;
	



};