#include "Framework.h"
#include "Mesh.h"

Mesh::Mesh(Shader * shader)
	:Renderer(shader), vertexType(VertexType::MeshVertex)
{
	
}

Mesh::~Mesh()
{
	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);
}

void Mesh::Render()
{

	if (vertexBuffer == nullptr&&indexBuffer == nullptr)
	{
		Create();

		vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(MeshVertex));
		indexBuffer = new IndexBuffer(indices, indexCount);

	}
		
	
	
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Renderer::Render();
	shader->DrawIndexed(0, Pass(), indexCount);
	
	
	/*switch (topology)
	{
	case topologyType::TRIANGLELIST:
		
		break;
	case topologyType::LINELIST:
		D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		Renderer::Render();
		shader->DrawIndexed(0, Pass(), indexCount);
		
		break;

	}*/
	
}



