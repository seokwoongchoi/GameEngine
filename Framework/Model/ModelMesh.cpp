#include "Framework.h"
#include "ModelMesh.h"


ModelBone::ModelBone()
 
{

}

ModelBone::~ModelBone()
{

}

///////////////////////////////////////////////////////////////////////////////

ModelMesh::ModelMesh()
	:shader(nullptr),material(nullptr),bone(nullptr), pass(0), roughness(1), metallic(1), drawCount(0)
{
	topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boneBuffer = new ConstantBuffer(&boneDesc, sizeof(BoneDesc));
	
}

ModelMesh::~ModelMesh()
{
	SafeDelete(vertexBuffer);
	SafeDelete(indexBuffer);

	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);
	SafeDelete(vertexBuffer);
	SafeDelete(indexBuffer);

	SafeDelete(boneBuffer);
	SafeDelete(material);
}


void ModelMesh::SetShader(Shader * shader)
{
	this->shader = shader;
	
	material->SetShader(shader);
	sBoneBuffer = shader->AsConstantBuffer("CB_Bone");
	
}



void ModelMesh::Render()
{

	//if (boneDesc.Index != boneIndex)
	{
		boneDesc.Index = boneIndex;
		boneBuffer->Apply();
		sBoneBuffer->SetConstantBuffer(boneBuffer->Buffer());
	}
	
	
	

	if (pass > 2)
	{
		material->Render();
	}

	vertexBuffer->Render();
	indexBuffer->Render();
	//pass != 1 ? topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST :
		//topology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
	topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	D3D::GetDC()->IASetPrimitiveTopology(topology);
	//D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	shader->DrawIndexedInstanced(0, pass, indexCount, drawCount);
}

void ModelMesh::PreviewRender()
{
	//if (boneDesc.Index != boneIndex)
	{
		boneDesc.Index = boneIndex;

		boneBuffer->Apply();
		sBoneBuffer->SetConstantBuffer(boneBuffer->Buffer());
	}
	

	material->Render();
	
	vertexBuffer->Render();
	indexBuffer->Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	shader->DrawIndexed(0, pass, indexCount);
	
}


void ModelMesh::Binding(Model * model)
{
	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(Model::ModelVertex));
	indexBuffer = new IndexBuffer(indices, indexCount);

	class Material* srcMaterial = model->MaterialByName(materialName);

	material = new class Material();
	material->Ambient(srcMaterial->Ambient());
	material->Diffuse(srcMaterial->Diffuse());
	material->Specular(srcMaterial->Specular());
	material->Roughness(srcMaterial->Specular().a);
	material->Matallic(metallic);

	if (srcMaterial->DiffuseMap() != NULL)
		material->DiffuseMap(srcMaterial->DiffuseMap()->GetFile());

	if (srcMaterial->SpecularMap() != NULL)
		material->SpecularMap(srcMaterial->SpecularMap()->GetFile());

	if (srcMaterial->NormalMap() != NULL)
		material->NormalMap(srcMaterial->NormalMap()->GetFile());

	if (srcMaterial->RoughnessMap() != NULL)
		material->RoughnessMap(srcMaterial->RoughnessMap()->GetFile());

	if (srcMaterial->MatallicMap() != NULL)
		material->MatallicMap(srcMaterial->MatallicMap()->GetFile());
	
}

