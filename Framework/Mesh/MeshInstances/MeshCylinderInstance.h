#pragma once
#define MAX_INSTANCE 4096 //�迭�� �ִ� ����
#include "Renders/Renderer.h"
class MeshCylinderInstance :public Renderer
{
public:
	typedef VertexTextureNormal MeshVertex;
public:
	MeshCylinderInstance(float bottomRadius, float topRadius,  float height, UINT sliceCount = 10, UINT stackCount = 10);
	~MeshCylinderInstance();

	void Update();
	void Render();

	uint Push();//�ѹ� ȣ��� drawCount++
	Transform* GetTransform(uint index);
private:
	void CreateSphereVertex();

	void BuildTopCap(vector<MeshVertex>& vertices, vector<UINT>& indices);
	void BuildBottomCap(vector<MeshVertex>& vertices, vector<UINT>& indices);
private:
	uint drawCount;

	Transform* transforms[MAX_INSTANCE];

	Matrix worlds[MAX_INSTANCE];


	VertexBuffer* instanceBuffer;

	vector<MeshVertex> v;
	vector<UINT> indices;
	
	UINT stackCount;
	UINT sliceCount;
	float topRadius;
	float bottomRadius;
	float height ;


};