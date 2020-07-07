#pragma once
class MeshCollider : public Mesh
{
public:
	MeshCollider(Shader * shader, VertexType vertexType);
	~MeshCollider();
	bool ModelPicking(IN const Vector3& position, IN const Vector3& scale, IN const Vector3& rotation, OUT Vector3 outPsotion);
	void SetIndex(uint index) {this->index= index;}
	uint GetIndex() { return index; }

	void SetColor(bool bPicked);
protected:
	void Create() override;

	

private:
	
	vector<VertexColor> v;

	uint* nIndices;
	uint nIndexCount;

	uint index;
};

