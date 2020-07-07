#pragma once

class ModelBone
{
public:
	friend class Model;

public:
	explicit ModelBone();
	~ModelBone();

	
		ModelBone(const ModelBone&) = delete;
	ModelBone& operator=(const ModelBone&) = delete;
	int BoneIndex() { return index; }
	int ParentIndex() { return parentIndex; }
	ModelBone* Parent() { return parent; }

	wstring Name() { return name; }

	inline const D3DXMATRIX& Transform() { return transform; }
	
	void PointerTransform(Matrix* mat) { *mat=transform; }
	void Transform(const Matrix& matrix) { transform = matrix; }

	vector<ModelBone*> GetChilds() { return childs; }
	inline ModelBone** ChildsData(){ return childs.data(); }
	inline const uint& ChildCcount() {
		const uint& temp =childs.size();
		return temp; }
private:

	int index;
	wstring name;

	int parentIndex;
	ModelBone* parent;

	D3DXMATRIX transform;

	Vector3 position;
	//Vector3 
	vector<ModelBone *> childs;

};

///////////////////////////////////////////////////////////////////////////////

class ModelMesh
{
public:
	friend class Model;

public:
	explicit ModelMesh();
	~ModelMesh();

	
		ModelMesh(const ModelMesh&) = delete;

	void Binding(Model* model);
	
public:
	inline void Pass(UINT pass) { this->pass = pass; }
	inline void DrawCount(const uint& drawCount) { this->drawCount = drawCount; }
	void SetShader(Shader* shader);

	
	
	void Render();
	void PreviewRender();
	
	wstring Name() { return name; }

	Material* Material() { return material; }
	wstring MaterialName() { return materialName; }

	Model::ModelVertex* Vertices() { return vertices; }
	VertexBuffer* GetBuffer() { return vertexBuffer; }

	int BoneIndex() { return boneIndex; }
	class ModelBone* Bone() { return bone; }

	uint GetVertexCount() { return vertexCount; }
	//void SetTess(float tess) { boneDesc.tess = tess; }

	inline Vector3 MinPos() { return minPos; }
	inline Vector3 MaxPos() { return maxPos; }
	
private:
	D3D10_PRIMITIVE_TOPOLOGY topology;
	class Shader* shader;
	class Material* material;
	
	class ModelBone* bone;
	int boneIndex;

	wstring name = L"";
	wstring materialName = L"";
	uint pass;

	uint drawCount;
    Model::ModelVertex* vertices;
	VertexBuffer* vertexBuffer;
	UINT vertexCount;
		
	IndexBuffer* indexBuffer;
	UINT indexCount;
	UINT* indices;

	Vector3 minPos;
	Vector3 maxPos;
private:
	struct BoneDesc
	{
		
		int Index=-1;
		float Padding[3];
	} boneDesc;

	ConstantBuffer* boneBuffer;
	ID3DX11EffectConstantBuffer* sBoneBuffer;
private:
	float roughness;
	float metallic;
	
};
