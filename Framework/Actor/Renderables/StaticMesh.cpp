#include "Framework.h"
#include "StaticMesh.h"

StaticMesh::StaticMesh(Shader * shader, SharedData* sharedData)
	: Renderable(shader, sharedData),pass(1),texture(nullptr),srv(nullptr), meshCount(0)
{
	forwardBuffer = new ConstantBuffer(&forward, sizeof(forwardDesc));
	sForwardBuffer = shader->AsConstantBuffer("CB_PreviewForward");

	sTransformSRV = shader->AsSRV("BoneTransforms");

}

void StaticMesh::SetModel(Model * model)
{
	this->model = *model;
}

void StaticMesh::OnDestroy()
{
	
	
}

void StaticMesh::Render()
{
	
	sTransformSRV->SetResource(srv);
	if (meshCount > 0)
	{
		//mesh = model->MeshsData();
		for (uint i = 0; i < meshCount; i++)
		{

			mesh[i].Pass(this->pass);
			mesh[i].DrawCount(sharedData->drawCount);
			mesh[i].Render(); //depth 1 packing 6

		}
	}
}



void StaticMesh::ForwardRender()
{
	

	if (forwardCount < 0) return;
	
	for (uint i = 0; i < forwardCount; i++)
	{
			forwardMesh[i].Pass(15);
			forwardMesh[i].DrawCount(sharedData->drawCount);
			forwardMesh[i].Render(); //depth 1 packing 6
	}
	
	
}


void StaticMesh::BlendModelBone(const Matrix & matrix)
{
	forward.matrix = matrix;
	forwardBuffer->Apply();
	sForwardBuffer->SetConstantBuffer(forwardBuffer->Buffer());
}

void StaticMesh::CreateBoneTransforms()
{
	if (srv != nullptr)
	{
		SafeRelease(texture);
		SafeRelease(srv);
	}

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
	forwardCount = model.forwardMeshesCount();
	if (forwardCount > 0)
	{
		forwardMesh=*model.forwardMeshsData();
		forwardMesh->SetShader(shader);
	}
	//CreateTexture
	{    uint boneCount = model.BoneCount();
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = boneCount * 4;
		desc.Height = MAX_MODEL_INSTANCE;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;
		Matrix* temp = new Matrix[boneCount];
		
		Matrix* boneTransforms = new Matrix[MAX_MODEL_INSTANCE*boneCount];
		for (UINT i = 0; i < MAX_MODEL_INSTANCE; i++)
		{
			for (UINT b = 0; b < model.BoneCount(); b++)
			{
				ModelBone* bone = model.BoneByIndex(b);

				Matrix parent;
				int parentIndex = bone->ParentIndex();

				if (parentIndex < 0)
					D3DXMatrixIdentity(&parent);
				else
					parent = temp[parentIndex];

				Matrix matrix = bone->Transform();
				temp[b] = parent;
				uint index = i * boneCount + b;
				boneTransforms[index] = matrix * temp[b];
			}//for(b)
		}//for(i)
		SafeDelete(temp);

		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = boneTransforms;
		subResource.SysMemPitch =  sizeof(Matrix);
		subResource.SysMemSlicePitch = 0;

		Check(D3D::GetDevice()->CreateTexture2D(&desc, &subResource, &texture));
		SafeDeleteArray(boneTransforms);
	}


	//Create SRV
	{
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Format = desc.Format;

		Check(D3D::GetDevice()->CreateShaderResourceView(texture, &srvDesc, &srv));
	}

	
	
}

