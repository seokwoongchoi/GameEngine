#pragma once
#define MAX_INSTANCE 4096 //�迭�� �ִ� ����
#include "Renders/Renderer.h"
class MeshSphereInstance :public Renderer
{
public:
	typedef VertexTextureNormal MeshVertex;
public:
	MeshSphereInstance();

	~MeshSphereInstance();

	void Update();
	void Render();

	uint Push();//�ѹ� ȣ��� drawCount++
	Transform* GetTransform(uint index);
	void SetColor(uint index,Color color);
private:
	void CreateSphereVertex();

	struct ColorDesc
	{

		Color RandomColor=Color(1,0,0,1);

		

	}colorDesc;
private:
	uint drawCount;

	Transform* transforms[MAX_INSTANCE];

	
	struct instDesc
	{
		Matrix worlds;
		Color color;
	};

	instDesc instdesc[MAX_INSTANCE];
	VertexBuffer* instanceBuffer;

	vector<MeshVertex> v;
	vector<UINT> indices;
	
	ConstantBuffer* randomBuffer;
	ID3DX11EffectConstantBuffer* sRandomBuffer;
	
	TextureArray* textures;

};