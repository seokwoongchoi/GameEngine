#pragma once
enum class topologyType
{
	TRIANGLELIST,
	LINELIST,
};
enum class VertexType
{
	MeshVertex,
	VertexColor,
};
class Mesh :public Renderer
{
public:
	typedef VertexTextureNormalTangent MeshVertex;
public:
	Mesh(Shader* shader);
	virtual ~Mesh();

	void Render();
	


protected:
	virtual void Create() = 0;


protected:
	MeshVertex* vertices;
	VertexColor* cVertices;

	uint* indices;

	
	VertexType vertexType;
	topologyType topology;

	

	
	bool bCheck;

	
};

