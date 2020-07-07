#include "Framework.h"
#include "Model.h"
#include "ModelMesh.h"
#include "Utilities/Xml.h"
#include "Utilities/BinaryFile.h"
#include "ModelClip.h"
Model::Model()
{
}

Model::~Model()
{
	for (Material* material : materials)
		SafeDelete(material);

	for (ModelBone* bone : bones)
		SafeDelete(bone);

	for (ModelMesh* mesh : meshes)
		SafeDelete(mesh);

	for (ModelClip* clip : clips)
		SafeDelete(clip);
}

Material * Model::MaterialByName(wstring name)
{
	for (Material* material : materials)
	{
		if (material->Name() == name)
			return material;
	}

	return NULL;
}

void Model::AddBone(int index, string name, int parentIndex, Matrix transform)
{
	ModelBone* bone = new ModelBone();

	if (index < 0)
	{
		bone->index = bones.size();
	}
	else
	bone->index = index;

	bone->name = String::ToWString(name);

	bone->parentIndex = parentIndex;
	bone->transform = transform;

	bones.emplace_back(bone);
	AddBindingBone(parentIndex);
}

void Model::AddBindingBone(int parentIndex)
{
	if (parentIndex < 0) return;
	bones.back()->parent = bones[parentIndex];
	bones.back()->parent->childs.push_back(bones.back());
}



ModelBone * Model::BoneByIndex(UINT index)
{
	if(index<bones.size())
	 return bones[index]; 

	return nullptr;
}

ModelBone * Model::BoneByName(wstring name)
{
	for (uint i = 0; i < bones.size(); i++)
	{
		if (bones[i]->Name()==name)
		{
			return bones[i];
		}
	}
}

void Model::ChangeBone(const Matrix& matrix,uint index)
{
	//Matrix local;
	//D3DXMatrixInverse(&local,nullptr,&bones[index]->transform);

	bones[index]->transform=matrix;

	/*auto childs = bones[index]->GetChilds();
	for (auto child : childs)
	{
		
		ChangeBone(child->transform*local*matrix,child->BoneIndex());
	}*/
	
}

ModelMesh * Model::MeshByName(wstring name)
{
	for (uint i = 0; i < meshes.size(); i++)
	{
		if (meshes[i]->Name() == name)
		{
		
			return meshes[i];
		}
	}
}



void Model::ReadMaterial(wstring file)
{
	file = L"../../_Textures/" + file + L".material";

	Xml::XMLDocument* document = new Xml::XMLDocument();
	Xml::XMLError error = document->LoadFile(String::ToString(file).c_str());
	assert(error == Xml::XML_SUCCESS);

	Xml::XMLElement* root = document->FirstChildElement();
	Xml::XMLElement* materialNode = root->FirstChildElement();

	do
	{
		Material* material = new Material();


		Xml::XMLElement* node = NULL;

		node = materialNode->FirstChildElement();
		material->Name(String::ToWString(node->GetText()));


		wstring directory = Path::GetDirectoryName(file);
		String::Replace(&directory, L"../../_Textures", L"");

		wstring texture = L"";

		node = node->NextSiblingElement();
		texture = String::ToWString(node->GetText());
		if (texture.length() > 0)
			material->DiffuseMap(directory + texture);

		node = node->NextSiblingElement();
		texture = String::ToWString(node->GetText());
		if (texture.length() > 0)
			material->SpecularMap(directory + texture);

		node = node->NextSiblingElement();
		texture = String::ToWString(node->GetText());
		if (texture.length() > 0)
			material->NormalMap(directory + texture);

		node = node->NextSiblingElement();
		texture = String::ToWString(node->GetText());
		if (texture.length() > 0)
			material->RoughnessMap(directory + texture);


		node = node->NextSiblingElement();
		texture = String::ToWString(node->GetText());
		if (texture.length() > 0)
			material->MatallicMap(directory + texture);

	   	D3DXCOLOR color;

		node = node->NextSiblingElement();
		color.r = node->FloatAttribute("R");
		color.g = node->FloatAttribute("G");
		color.b = node->FloatAttribute("B");
		color.a = node->FloatAttribute("A");
		material->Ambient(color);

		node = node->NextSiblingElement();
		color.r = node->FloatAttribute("R");
		color.g = node->FloatAttribute("G");
		color.b = node->FloatAttribute("B");
		color.a = node->FloatAttribute("A");
		material->Diffuse(color);

		node = node->NextSiblingElement();
		color.r = node->FloatAttribute("R");
		color.g = node->FloatAttribute("G");
		color.b = node->FloatAttribute("B");
		color.a = node->FloatAttribute("A");
		material->Specular(color);

	/*	node = node->NextSiblingElement();
		color.r = node->FloatAttribute("R");
		color.g = node->FloatAttribute("G");
		color.b = node->FloatAttribute("B");
		color.a = node->FloatAttribute("A");
		material->Roughness(color);

		node = node->NextSiblingElement();
		color.r = node->FloatAttribute("R");
		color.g = node->FloatAttribute("G");
		color.b = node->FloatAttribute("B");
		color.a = node->FloatAttribute("A");
		material->Matallic(color);*/
		//material->Shininess(node->FloatText());

		materials.push_back(material);


		materialNode = materialNode->NextSiblingElement();
	} while (materialNode != NULL);
}

void Model::ReadMesh(wstring file)
{
	modelName = String::ToString(file);
	file = file + L"/" + file;
	file = L"../../_Models/StaticMeshes/" + file + L".mesh";


	BinaryReader* r = new BinaryReader();
	r->Open(file);
	   	

	UINT count = 0;

	count = r->UInt();
	unArmedBoneCount = count;
	for (UINT i = 0; i < count; i++)
	{
		ModelBone* bone = new ModelBone();

		bone->index = r->Int();
		bone->name = String::ToWString(r->String());
		bone->parentIndex = r->Int();
		bone->transform = r->Matrix();

		bones.push_back(bone);
	}
	

	count = r->UInt();
	
	for (UINT i = 0; i < count; i++)
	{
		ModelMesh* mesh = new ModelMesh();

		mesh->name = String::ToWString(r->String());
		mesh->boneIndex = r->Int();

		mesh->materialName = String::ToWString(r->String());
		//VertexData
		{
			UINT count = r->UInt();
			
			vector<Model::ModelVertex> vertices;
			vertices.assign(count, Model::ModelVertex());

			void* ptr = (void *)&(vertices[0]);
			r->Byte(&ptr, sizeof(Model::ModelVertex) * count);

			mesh->vertices = new Model::ModelVertex[count];
			mesh->vertexCount = count;
			copy
			(
				vertices.begin(), vertices.end(),
				stdext::checked_array_iterator<Model::ModelVertex *>(mesh->vertices, count)
			);
		
		}
		
		//IndexData
		{
			UINT count = r->UInt();

			vector<UINT> indices;
			indices.assign(count, UINT());

			void* ptr = (void *)&(indices[0]);
			r->Byte(&ptr, sizeof(UINT) * count);


			mesh->indices = new UINT[count];
			mesh->indexCount = count;
			copy
			(
				indices.begin(), indices.end(),
				stdext::checked_array_iterator<UINT *>(mesh->indices, count)
			);
		}
		
		
		mesh->minPos.x=r->Float();
		mesh->minPos.y = r->Float();
		mesh->minPos.z = r->Float();

		mesh->maxPos.x = r->Float();
		mesh->maxPos.y = r->Float();
		mesh->maxPos.z = r->Float();
			

		meshes.push_back(mesh);
	}//for(i)

	r->Close();
	SafeDelete(r);


	BindingBone();
	BindingMesh();
	
	
}

void Model::ReadPreviewMesh(wstring file, const uint& MeshType)
{
	modelName = String::ToString(file);
	file = file + L"/" + file;
	if(MeshType==0)
	file = L"../../_Models/SkeletalMeshes/" + file + L".mesh";

	else if (MeshType == 1)
		file = L"../../_Models/StaticMeshes/" + file + L".mesh";

	BinaryReader* r = new BinaryReader();
	r->Open(file);





	UINT count = 0;

	count = r->UInt();
	unArmedBoneCount = count;
	for (UINT i = 0; i < count; i++)
	{
		ModelBone* bone = new ModelBone();

		bone->index = r->Int();
		bone->name = String::ToWString(r->String());
		bone->parentIndex = r->Int();
		bone->transform = r->Matrix();

		bones.push_back(bone);
	}


	count = r->UInt();

	for (UINT i = 0; i < count; i++)
	{
		ModelMesh* mesh = new ModelMesh();

		mesh->name = String::ToWString(r->String());
		mesh->boneIndex = r->Int();
		mesh->materialName = String::ToWString(r->String());
		//VertexData
		{
			UINT count = r->UInt();

			vector<Model::ModelVertex> vertices;
			vertices.assign(count, Model::ModelVertex());
		
			void* ptr = (void *)&(vertices[0]);
			r->Byte(&ptr, sizeof(Model::ModelVertex) * count);

			mesh->vertices = new Model::ModelVertex[count];
			mesh->vertexCount = count;
			copy
			(
				vertices.begin(), vertices.end(),
				stdext::checked_array_iterator<Model::ModelVertex *>(mesh->vertices, count)
			);
			

		}

		//IndexData
		{
			UINT count = r->UInt();

			vector<UINT> indices;
			indices.assign(count, UINT());

			void* ptr = (void *)&(indices[0]);
			r->Byte(&ptr, sizeof(UINT) * count);


			mesh->indices = new UINT[count];
			mesh->indexCount = count;
			copy
			(
				indices.begin(), indices.end(),
				stdext::checked_array_iterator<UINT *>(mesh->indices, count)
			);
		}


		mesh->minPos.x = r->Float();
		mesh->minPos.y = r->Float();
		mesh->minPos.z = r->Float();

		mesh->maxPos.x = r->Float();
		mesh->maxPos.y = r->Float();
		mesh->maxPos.z = r->Float();

		

		previewMeshes.push_back(mesh);
	}//for(i)

	r->Close();
	SafeDelete(r);


	BindingBone();
	BindingPreviewMesh();
}



void Model::BindingBone()
{
	
	root = bones[0];
	for (ModelBone* bone : bones)
	{
		//cout << String::ToString(bone->Name()) << endl;
		if (bone->parentIndex > -1)
		{
			//cout <<String::ToString( bone->Name()) << endl;
			bone->parent = bones[bone->parentIndex];
			bone->parent->childs.push_back(bone);
		    int a = 0;
		}
		else
			bone->parent = NULL;
	}
}

void Model::BindingMesh()
{
	for (ModelMesh* mesh : meshes)
	{
		
		for (ModelBone* bone : bones)
		{
			if (mesh->boneIndex == bone->index)
			{
				mesh->bone = bone;
				break;
			}
		}

		mesh->Binding(this);
	}
}

void Model::BindingPreviewMesh()
{
	for (ModelMesh* mesh : previewMeshes)
	{
		for (ModelBone* bone : bones)
		{
			if (mesh->boneIndex == bone->index)
			{
				mesh->bone = bone;
				break;
			}
		}

		mesh->Binding(this);
	}
}

ModelClip * Model::ClipByName(wstring name)
{
	for (ModelClip* clip : clips)
	{
		if (clip->name == name)
			return clip;
	}

	return NULL;
}
void Model::DeleteBone(ModelBone * bone)
{
	for (auto& iter = previewMeshes.begin(); iter != previewMeshes.end();)
	{
		auto previewMesh = *iter;
		if (previewMesh->BoneIndex() == bone->BoneIndex())
		{
			
			iter = previewMeshes.erase(iter);

		}
		else
			iter++;
	}

}
void Model::BlendModelMeshes(ModelBone * currentBone)
{

	for (auto iter = previewMeshes.begin(); iter != previewMeshes.end();)
	{
		auto previewMesh = *iter;
		if (previewMesh->BoneIndex() == currentBone->BoneIndex())
		{
			forwardMeshes.emplace_back(previewMesh);
			iter=previewMeshes.erase(iter);
			
		}
		else
			iter++;
	}



}
void Model::Attach(Shader * shader, Model * model, int parentBoneIndex, Transform * transform)
{
	//Copy Material
	for (Material* material : model->Materials())
	{
		Material* newMaterial = new Material(shader);

		newMaterial->Name(material->Name());
		newMaterial->Ambient(material->Ambient());
		newMaterial->Diffuse(material->Diffuse());
		newMaterial->Specular(material->Specular());

		if (material->DiffuseMap() != NULL)
			newMaterial->DiffuseMap(material->DiffuseMap()->GetFile());

		if (material->SpecularMap() != NULL)
			newMaterial->SpecularMap(material->SpecularMap()->GetFile());

		if (material->NormalMap() != NULL)
			newMaterial->NormalMap(material->NormalMap()->GetFile());
		if (material->RoughnessMap() != NULL)
			newMaterial->RoughnessMap(material->RoughnessMap()->GetFile());
		if (material->MatallicMap() != NULL)
			newMaterial->MatallicMap(material->MatallicMap()->GetFile());

		materials.push_back(newMaterial);
	}


	vector<pair<int, int>> changes;

	//Matrix iden;
	//D3DXMatrixIdentity(&iden);
	//ModelBone* joint = new ModelBone();
	//joint->index = bones.size();
	//joint->name = L"Joint";
	//joint->transform = iden;

	//joint->parentIndex =parentBoneIndex;
	//joint->parent = bones[parentBoneIndex];
	//joint->parent->childs.push_back(joint);

	//bones.emplace_back(joint);
	//Copy Bone
	{
		ModelBone* parentBone = BoneByIndex(parentBoneIndex);
		
		for (ModelBone* bone : model->Bones())
		{
			ModelBone* newBone = new ModelBone();
			newBone->name = bone->name;
			newBone->transform = bone->transform;
			if (newBone->name == L"4_Cube.007")
			{
				Matrix matrix = Matrix(
					1.0f, 0.0f, 15.0f, 0.0f,
					15.0f, -2.0f, -1.0f, 0.0f,
					2.0f, 15.0f, 0.0f, 0.0f,
					6.0f, 29.0f, -4.0f, 1.0f);
				newBone->transform = matrix;
			}


		if (transform != NULL)
		   newBone->transform = newBone->transform * transform->World();

		if (bone->parent != NULL)
		{
			int parentIndex = bone->parentIndex;

			for (pair<int, int>& temp : changes)
			{
				if (temp.first == parentIndex)
				{
					newBone->parentIndex = temp.second;
					newBone->parent = bones[newBone->parentIndex];
					newBone->parent->childs.push_back(newBone);
					
					break;
				}
			}//for(temp)
		}
		else
			{
				newBone->parentIndex = parentBoneIndex;
				newBone->parent = parentBone;
				newBone->parent->childs.push_back(newBone);
			}

			newBone->index = bones.size();
			changes.push_back(pair<int, int>(bone->index, newBone->index));

		

			bones.push_back(newBone);
		}//for(bone)
	}
	
	
	//Copy Mesh
	{
		uint count = model->MeshCount();
		if (count > 0)
		for (uint i = 0; i < count; i++)
		{
			auto mesh = model->MeshsData();
			ModelMesh* newMesh = new ModelMesh();

			for (pair<int, int>& temp : changes)
			{
				if (temp.first == mesh[i]->boneIndex)
				{
					newMesh->boneIndex = temp.second;

					break;
				}
			}//for(temp)


			newMesh->bone = bones[newMesh->boneIndex];
			newMesh->name = mesh[i]->name;
			newMesh->materialName = mesh[i]->materialName;

			newMesh->vertexCount = mesh[i]->vertexCount;
			newMesh->indexCount = mesh[i]->indexCount;

			UINT verticesSize = newMesh->vertexCount * sizeof(ModelVertex);
			newMesh->vertices = new ModelVertex[newMesh->vertexCount];
			memcpy_s(newMesh->vertices, verticesSize, mesh[i]->vertices, verticesSize);

			UINT indicesSize = newMesh->indexCount * sizeof(UINT);
			newMesh->indices = new UINT[newMesh->indexCount];
			memcpy_s(newMesh->indices, indicesSize, mesh[i]->indices, indicesSize);


			newMesh->Binding(this);
			newMesh->SetShader(shader);

			previewMeshes.push_back(newMesh);
		}
	}
}
void Model::ReadClip(wstring file)
{
	file = L"../../_Models/SkeletalMeshes/" + file + L".clip";

	BinaryReader* r = new BinaryReader();
	r->Open(file);


	ModelClip* clip = new ModelClip();

	clip->name = String::ToWString(r->String());
	clip->duration = r->Float();
	clip->frameRate = r->Float();
	clip->frameCount = r->UInt();

	UINT keyframesCount = r->UInt();
	for (UINT i = 0; i < keyframesCount; i++)
	{
		ModelKeyframe* keyframe = new ModelKeyframe();
		keyframe->BoneName = String::ToWString(r->String());

		UINT size = r->UInt();
		if (size > 0)
		{
			keyframe->Transforms.assign(size, ModelKeyframeData());

			void* ptr = (void *)&keyframe->Transforms[0];
			r->Byte(&ptr, sizeof(ModelKeyframeData) * size);
		}

		clip->keyframeMap[keyframe->BoneName] = keyframe;
		
	}

	r->Close();
	SafeDelete(r);

	clips.push_back(clip);
}

void Model::CompileMesh()
{
	if (!meshes.empty())
	{
		meshes.clear();
		meshes.shrink_to_fit();
	}
	/*meshes.resize(previewMeshes.size());
	meshes.reserve(previewMeshes.size());*/
	for (uint i = 0; i < previewMeshes.size(); i++)
	{
		meshes.emplace_back(previewMeshes[i]);
			
	}
	
}
