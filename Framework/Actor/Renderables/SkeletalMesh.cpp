#include "Framework.h"
#include "SkeletalMesh.h"
#include "Actor/Components/Animator.h"
SkeletalMesh::SkeletalMesh(Shader* shader, SharedData* sharedData)
	:Renderable(shader, sharedData), pass(2), meshCount(0)
{

	forwardBuffer = new ConstantBuffer(&forward, sizeof(forwardDesc));
	sForwardBuffer = shader->AsConstantBuffer("CB_PreviewForward");

	//sCollisonSrv = shader->AsSRV("CollisonData");

}



void SkeletalMesh::SetModel(Model * model)
{
	this->model = *model;
}

void SkeletalMesh::OnDestroy()
{
	SafeRelease(sForwardBuffer);
	SafeDelete(forwardBuffer);
	SafeDelete(animator);
}

void SkeletalMesh::Render()
{

	animator->Render();
	//sCollisonSrv->SetResource(ColliderSystem::Get()->SRV());
	//mesh = model->MeshsData();
	for (uint i = 0; i < meshCount; i++)
	{
		mesh[i].Pass(this->pass);
		mesh[i].DrawCount(sharedData->drawCount);
		mesh[i].Render(); //depth 1 packing 6
	}

}



void SkeletalMesh::ForwardRender()
{
	//if (forwardBuffer != NULL)
	//{
	//	forwardBuffer->Apply();
	//	sForwardBuffer->SetConstantBuffer(forwardBuffer->Buffer());
	//}

	//uint count = model->forwardMeshesCount();

	//if (count > 0)
	//{
	//	auto mesh = model->forwardMeshsData();
	//	for (uint i = 0; i < count; i++)
	//	{
	//		mesh[i]->Pass(15);
	//		mesh[i]->DrawCount(drawCount);
	//		mesh[i]->Render(); //depth 1 packing 6
	//	}
	//}
}







void SkeletalMesh::CreateBoneTransforms()
{
	//if (srv != nullptr)
	//{
	//	//SafeRelease(texture);
	//	SafeRelease(srv);
	//}

	meshCount = model.MeshCount();
	if (meshCount > 0)
	{
		mesh = new ModelMesh[meshCount];
		auto temp = model.MeshsData();
		for (uint i = 0; i < meshCount; i++)
		{

			mesh[i] = *temp[i];
			mesh[i].SetShader(shader);


		}
	}



}










