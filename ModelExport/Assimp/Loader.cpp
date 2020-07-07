#include "stdafx.h"
#include "Loader.h"
#include "Type.h"
#include "Utilities/Xml.h"
#include "Utilities/BinaryFile.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/version.h>
Loader::Loader()
{
	importer = new Assimp::Importer();

	//Get Version
	const int major = aiGetVersionMajor();
	const int minor = aiGetVersionMinor();
	const int revision = aiGetVersionRevision();

   //assimpFlags =
   // 	aiProcess_CalcTangentSpace |
   // 	aiProcess_GenSmoothNormals |
   // 	aiProcess_JoinIdenticalVertices |
   // 	aiProcess_OptimizeMeshes |
   // 	aiProcess_ImproveCacheLocality |
   // 	aiProcess_LimitBoneWeights |
   // 	aiProcess_SplitLargeMeshes |
   // 	aiProcess_Triangulate |
   // 	aiProcess_GenUVCoords |
   // 	aiProcess_SortByPType |
   // 	aiProcess_FindDegenerates |
   // 	aiProcess_FindInvalidData |
   // 	aiProcess_FindInstances |
   // 	aiProcess_ValidateDataStructure |
   // 	aiProcess_Debone |
   // 	aiProcess_ConvertToLeftHanded;

	assimpFlags =
		aiProcess_ConvertToLeftHanded |
		aiProcess_Triangulate |//삼각화
		aiProcess_GenUVCoords | //uv를 우리의 형태로
		aiProcess_GenNormals | //normal이 없다면 계산
		aiProcess_CalcTangentSpace;//tangent계산

	
}

Loader::~Loader()
{
	SafeDelete(importer);
}



void Loader::ReadFile(wstring file)
{
	this->file = L"../../_Assets/" + file;
	
	importer->SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f);
	importer->SetPropertyFloat(AI_CONFIG_PP_CT_MAX_SMOOTHING_ANGLE, 80.0f);
	importer->SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, 1'000'000);
	importer->SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, 1'000'000);
	importer->SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
	importer->SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_LIGHTS);
	importer->SetPropertyBool(AI_CONFIG_GLOB_MEASURE_TIME, true);

	scene = importer->ReadFile
	(
		String::ToString(this->file),
		assimpFlags
	
	);
	
	//scene = importer->ReadFile
	//(
	//	String::ToString(this->file),
	//	aiProcess_CalcTangentSpace | //가져온 메쉬의 접선공간 계산
	//	aiProcess_GenSmoothNormals | //메쉬의 모든 정점에서 부드러운 법선 생성
	//	aiProcess_JoinIdenticalVertices | //모든 메쉬에서 동일한 정점 데이터 세트를 식별하고 조인-인덱스
	//	aiProcess_OptimizeMeshes |//메쉬의 수를 줄이기 위한 후처리 단계
	//	aiProcess_ImproveCacheLocality |//정점 캐쉬의 지역성을 높이기 위해 삼각형 재정리
	//	aiProcess_LimitBoneWeights | //단일 정점에 동시에 영향을 미치는 본의 수를 제한
	//	aiProcess_SplitLargeMeshes |//큰메쉬를 작은 메쉬로 분할
	//	aiProcess_Triangulate | //삼각화
	//	aiProcess_GenUVCoords |//비 uv 맵핑을 적절한 텍스쳐 좌표 채널로 변경
	//	aiProcess_SortByPType |//점이나 선이 면 중간중간에있는 경우 없앤다.
	//	aiProcess_FindDegenerates |//점이나 선이 면 중간중간에있는 경우 없앤다.
	//	aiProcess_FindInvalidData | //무효한 데이터를 찾아서 없앤다.
	//	aiProcess_FindInstances | //실체화 중복된 메쉬를 제거한다.
	//	aiProcess_ValidateDataStructure |
	//	aiProcess_ConvertToLeftHanded
	//);
	
	assert(scene != NULL); 
}

void Loader::ExportMaterial(wstring savePath, bool bOverWrite)
{
	savePath = L"../../_Textures/" + savePath + L".material";

	ReadMaterial();
	WriteMaterial(savePath, bOverWrite);
}

void Loader::ReadMaterial()
{
	for (uint i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* readMaterial = scene->mMaterials[i];
		AsMaterial* material = new AsMaterial(); //우리꺼

		material->Name = readMaterial->GetName().C_Str();

		float val;
		aiColor3D color;

		readMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);//color에 리턴받는다.
		material->Ambient = Color(color.r,color.g,color.b, 1.0f);

		readMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);//color에 리턴받는다.
		material->Diffuse = Color(color.r, color.g, color.b, 1.0f);
		
		readMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);//color에 리턴받는느다.
		material->Specular = Color(color.r, color.g, color.b, 1.0f);

		readMaterial->Get(AI_MATKEY_SHININESS, color);
		material->Metallic = Color(color.r, 0, 0, 1.0f);
		readMaterial->Get(AI_MATKEY_COLOR_REFLECTIVE, color);
		material->Roughness = Color(color.r, 0, 0, 1.0f);
		//readMaterial->Get(AI_MATKEY_REFLECTIVITY, material->Metallic);
		
		

		aiString file;
		readMaterial->GetTexture(aiTextureType_DIFFUSE, 0,&file);
		material->DiffuseFile = file.C_Str();

		readMaterial->GetTexture(aiTextureType_SPECULAR, 0, &file);
		material->SpecularFile = file.C_Str();

		readMaterial->GetTexture(aiTextureType_NORMALS, 0, &file);
		material->NormalFile = file.C_Str();
		readMaterial->GetTexture(aiTextureType_REFLECTION, 0, &file);
		material->RoughnessFile = file.C_Str(); 
		readMaterial->GetTexture(aiTextureType_LIGHTMAP, 0, &file);
		material->MetallicFile = file.C_Str();

		

		materials.push_back(material);//vector에 부른정보들 전부 저장.
		int a = 0;
	}
}

void Loader::WriteMaterial(wstring savePath, bool bOverWrite)
{
	if (bOverWrite == false) //덮어쓰기 x
	{
		if (Path::ExistFile(savePath) == true)
			return;
	}


	string folder = String::ToString(Path::GetDirectoryName(savePath));
	string file = String::ToString(Path::GetFileName(savePath));

	Path::CreateFolders(folder);

	Xml::XMLDocument* document = new Xml::XMLDocument();

	Xml::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	Xml::XMLElement* root = document->NewElement("Materials");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(root);

	for (AsMaterial* material : materials)
	{
		Xml::XMLElement*node = document->NewElement("Material");
		root->LinkEndChild(node);

		Xml::XMLElement* element = NULL;

		element = document->NewElement("Name");
		element->SetText(material->Name.c_str());
		node->LinkEndChild(element);

		element = document->NewElement("DiffuseFile");
		element->SetText(WriteTexture(folder,material->DiffuseFile).c_str());
		node->LinkEndChild(element);
		element = document->NewElement("SpecularFile");
		element->SetText(WriteTexture(folder, material->SpecularFile).c_str());
		node->LinkEndChild(element);
		element = document->NewElement("NormalFile");
		element->SetText(WriteTexture(folder, material->NormalFile).c_str());
		node->LinkEndChild(element);
		element = document->NewElement("RoughnessFile");
		element->SetText(WriteTexture(folder, material->RoughnessFile).c_str());
		node->LinkEndChild(element);
		element = document->NewElement("MetallicFile");
		element->SetText(WriteTexture(folder, material->MetallicFile).c_str());
		node->LinkEndChild(element);
		element = document->NewElement("Ambient");
		element->SetAttribute("R", material->Ambient.r);
		element->SetAttribute("G", material->Ambient.g);
		element->SetAttribute("B", material->Ambient.b);
		element->SetAttribute("A", material->Ambient.a);
		node->LinkEndChild(element);

		element = document->NewElement("Diffuse");
		element->SetAttribute("R", material->Diffuse.r);
		element->SetAttribute("G", material->Diffuse.g);
		element->SetAttribute("B", material->Diffuse.b);
		element->SetAttribute("A", material->Diffuse.a);
		node->LinkEndChild(element);

		element = document->NewElement("Specular");
		element->SetAttribute("R", material->Specular.r);
		element->SetAttribute("G", material->Specular.g);
		element->SetAttribute("B", material->Specular.b);
		element->SetAttribute("A", material->Specular.a);
		node->LinkEndChild(element);
		/*element = document->NewElement("Roughness");
		element->SetAttribute("Roughness", material->Roughness);
		
		node->LinkEndChild(element);
		element = document->NewElement("Metallic");
		element->SetAttribute("Metallic", material->Metallic);
		node->LinkEndChild(element);*/
		SafeDelete(material);
	}
	
	//Xml::XMLElement* node = document->NewElement("Test");
	//node->SetText("fafsafst");
	//root->LinkEndChild(node);

	//node = document->NewElement("Test2");
	//node->SetText(3.14f);
	//root->LinkEndChild(node);

	//node = document->NewElement("Color");
	//node->SetAttribute("@", 1.2f);
	//node->SetText("AAA");
	//root->LinkEndChild(node);

	document->SaveFile((folder+file).c_str());
	
}

string Loader::WriteTexture(string savePath, string file)
{
	if (file.empty()) return ""; //따로 연결해야할 경우
	
	string fileName = Path::GetFileName(file);
	//texture가 내장되있는지 판단
	const aiTexture* texture = scene->GetEmbeddedTexture(file.c_str());

	string path = "";
	if (texture)//내장되있다면 texture==nullptr이면 외장에 texture가 있다.
	{
		path = savePath + Path::GetFileNameWithoutExtension(file) + ".png";
		
		if (texture->mHeight < 1)
		{
			//FILE* fp;
			//fopen_s(&fp, path.c_str(), "ab");
			//fwrite(texture->pcData, texture->mWidth, 1,fp);
			////pcData:이미지 전체 데이터를 가지고있다.
			//fclose(fp);

			BinaryWriter w;//분석 숙제
			w.Open(String::ToWString(path));
			w.Byte(texture->pcData, texture->mWidth);
			w.Close();
		}
		else
		{
			ID3D11Texture2D* dest;
			D3D11_TEXTURE2D_DESC destDesc;
			ZeroMemory(&destDesc, sizeof(D3D11_TEXTURE2D_DESC));
			destDesc.Width = texture->mWidth;
			destDesc.Height = texture->mHeight;
			destDesc.MipLevels = 1;
			destDesc.ArraySize = 1;
			destDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			destDesc.SampleDesc.Count = 1;
			destDesc.SampleDesc.Quality = 0;
			//destDesc.Usage = D3D11_USAGE_IMMUTABLE;

			D3D11_SUBRESOURCE_DATA subResource = { 0 };
			subResource.pSysMem = texture->pcData;


			HRESULT hr;
			hr = D3D::GetDevice()->CreateTexture2D(&destDesc, &subResource, &dest);
			assert(SUCCEEDED(hr));

			D3DX11SaveTextureToFileA(D3D::GetDC(), dest, D3DX11_IFF_PNG, savePath.c_str());
		}
	}////////////////////////////////////////////////////////////////////////////////
	else //외장처리
	{
		string directory = Path::GetDirectoryName(String::ToString(this->file));
		string origin = directory + file; //파일경로+상대경로
		String::Replace(&origin, "\\", "/");//역슬래쉬를 슬래쉬로 바꿔준다.
		if (Path::ExistFile(origin) == false)
			return "";
		path = savePath + fileName;

		CopyFileA(origin.c_str(), path.c_str(),FALSE);
		
		String::Replace(&path, "../../Textures/","");
		
	}
	
	return Path::GetFileName(path);
}


///////////////////////////////////////////////////////////////////////////////
void Loader::ExportMesh(wstring savePath, bool bOverWrite)
{
	savePath = L"../../_Models/" + savePath + L".mesh";
	
	
	
	ReadBoneData(scene->mRootNode, -1, -1);
	
	ReadSkinData();
	WriteMeshData(savePath, bOverWrite);
}


void Loader::ReadBoneData(aiNode * node, int index, int parent)
{
	AsBone* bone = new AsBone();
	bone->Index = index;
	bone->Parent = parent;
	bone->Name = node->mName.C_Str();
	
	
	D3DXMATRIX temp1 = D3DXMATRIX(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, -1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f);
	
	Matrix transform(node->mTransformation[0]);//0번째배열만 넣어줘도 매트릭스전체를
	//복사한다.
	D3DXMatrixTranspose(&bone->Transform, &(transform));//assimp는 열우선
	//memcpy(&bone->Transform, &transform, sizeof(Matrix));
	//bone->Transform = transform;
	Matrix temp;
	if (parent == -1)
	{
		D3DXMatrixIdentity(&temp);
		//temp *= temp1;
		//temp = temp1;
	}
		
	else
		temp = bones[parent]->Transform;
	bone->Transform = bone->Transform*temp;//자식* 부모
	
	
	bones.push_back(bone);
		

	
	

	ReadMeshData(node, index);
	
	for (uint i = 0; i < node->mNumChildren; i++)
	{
		ReadBoneData(node->mChildren[i], bones.size(),index);
	}
	
}

void Loader::ReadMeshData(aiNode * node, int bone)
{
	
	if (node->mNumMeshes < 1) return;
	
	minPos = Vector3(
		std::numeric_limits<float>::infinity(),
		std::numeric_limits<float>::infinity(),
		std::numeric_limits<float>::infinity()
	);
	maxPos = Vector3(
		-std::numeric_limits<float>::infinity(),
		-std::numeric_limits<float>::infinity(),
		-std::numeric_limits<float>::infinity()
	);
	AsMesh* asMesh = new AsMesh();
	//static int i = 0;
	asMesh->Name = node->mName.C_Str();
	cout << asMesh->Name << endl;
	//i++;
	asMesh->BoneIndex = bone;
	

	for (UINT i = 0; i < node->mNumMeshes; i++) //mesh가 여러개일수도있다. 
	{
		
		UINT index = node->mMeshes[i]; //scene의 배열번호
		aiMesh* mesh = scene->mMeshes[index]; //scene에있는 배열로부터 매쉬를가져온다.
	
		UINT startVertex = asMesh->Vertices.size();
		UINT startIndex = asMesh->Indices.size();

		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		asMesh->MaterialName = material->GetName().C_Str();

		UINT count = mesh->mNumVertices;
		
		
		for (UINT m = 0; m < count; m++)
		{
		
			Model::ModelVertex vertex;
				
			if (mesh->mVertices)
			{
			
				Vector3 temp;

				
				minPos.x = Math::Min(minPos.x, mesh->mVertices[m].x);
				minPos.y = Math::Min(minPos.y, mesh->mVertices[m].y);
				minPos.z = Math::Min(minPos.z, mesh->mVertices[m].z);

				maxPos.x = Math::Max(maxPos.x,mesh->mVertices[m].x);
				maxPos.y = Math::Max(maxPos.y,mesh->mVertices[m].y);
				maxPos.z = Math::Max(maxPos.z,mesh->mVertices[m].z);
				
				
				
				memcpy(&vertex.Position, &mesh->mVertices[m], sizeof(Vector3));
				
			}
						
			
			if(mesh->HasTextureCoords(0))
				memcpy(&vertex.Uv, &mesh->mTextureCoords[0][m], sizeof(Vector2));
			
			if (mesh->mNormals)
				vertex.Normal = Vector3(mesh->mNormals[m].x,
					mesh->mNormals[m].y, mesh->mNormals[m].z);
		
			if (mesh->HasTangentsAndBitangents())
				vertex.Tangent = Vector3(mesh->mTangents[m].x,
					mesh->mTangents[m].y, mesh->mTangents[m].z);
					
			asMesh->Vertices.emplace_back(vertex);
		}//for(m)

	
		for (UINT f = 0; f < mesh->mNumFaces; f++)
		{
			aiFace& face = mesh->mFaces[f];

			for (UINT k = 0; k <face.mNumIndices; k++)
			{
				asMesh->Indices.push_back(face.mIndices[k]);
				asMesh->Indices.back() += startVertex;//index
			}
		}

		asMesh->max = maxPos;
		asMesh->min = minPos;

		
	}

	meshes.push_back(asMesh);
	
	
}

void Loader::ReadSkinData()
{
	for (uint i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* aiMesh = scene->mMeshes[i];
		
		if (aiMesh->HasBones() == false)continue;
		
		AsMesh* mesh = meshes[i];
		vector<AsBoneWeights> boneWeights;
		boneWeights.assign(mesh->Vertices.size(), AsBoneWeights());
		
		

		for (uint b = 0; b < aiMesh->mNumBones; b++)
		{
			aiBone* aiBone = aiMesh->mBones[b];

			uint boneIndex = 0;
			for (AsBone* bone : bones)
			{
				if (bone->Name == (string)aiBone->mName.C_Str())
				{
					boneIndex = bone->Index;
					break;
				}
					
			}
			
			for (uint m = 0; m < aiBone->mNumWeights; m++)
			{
				uint index = aiBone->mWeights[m].mVertexId;
			

				float weight = aiBone->mWeights[m].mWeight;
				
				
				boneWeights[index].AddWeights(boneIndex, weight);
				
			}
		}

		for (uint w =0; w < boneWeights.size(); w++)
		{
			boneWeights[w].Normalize();
			AsBlendWeight blendWeight;
			boneWeights[w].GetBlendWeights(blendWeight);

		
			mesh->Vertices[w].BlendIndices = blendWeight.Indices;
			mesh->Vertices[w].BlendWeights = blendWeight.Weights;
		}
	}
	
}

void Loader::WriteMeshData(wstring savePath, bool bOverWrite)
{
	if (bOverWrite == false)
	{
		if (Path::ExistFile(savePath) == true)
			return;
	}


	Path::CreateFolders(Path::GetDirectoryName(savePath));

	BinaryWriter* w = new BinaryWriter();
	w->Open(savePath);

	w->UInt(bones.size());
	for (AsBone* bone : bones)
	{
		w->Int(bone->Index);
		w->String(bone->Name);
		w->Int(bone->Parent);
		w->Matrix(bone->Transform);

		SafeDelete(bone);
	}

	w->UInt(meshes.size());
	for (AsMesh* meshData : meshes)
	{
		
		w->String(meshData->Name);
		w->Int(meshData->BoneIndex);

		w->String(meshData->MaterialName);

		w->UInt(meshData->Vertices.size());
		w->Byte(&meshData->Vertices[0], sizeof(Model::ModelVertex) * meshData->Vertices.size());

		w->UInt(meshData->Indices.size());
		w->Byte(&meshData->Indices[0], sizeof(uint) * meshData->Indices.size());

		w->Float(meshData->min.x);
	    w->Float(meshData->min.y);
	    w->Float(meshData->min.z);

	
	    w->Float(meshData->max.x);
	    w->Float(meshData->max.y);
	    w->Float(meshData->max.z);
	
		SafeDelete(meshData);

		

	

		
	}

	w->Close();
	SafeDelete(w);
}


void Loader::GetClipList(vector<wstring>* list)
{
	for (uint i = 0; i < scene->mNumAnimations; i++)
	{
		aiAnimation* anim = scene->mAnimations[i];
		list->emplace_back(String::ToWString(anim->mName.C_Str()));
	}
}

void Loader::ExportAnimClip(uint index, wstring savePath, bool bOverWrite)
{
	savePath=L"../../_Models/SkeletalMeshes/" + savePath + L".clip";
	AsClip* clip = ReadClipData(scene->mAnimations[index]);
	WriteClipData(clip, savePath, bOverWrite);
}

AsClip * Loader::ReadClipData(aiAnimation * animation)
{
	AsClip* clip = new AsClip();
	clip->Name = animation->mName.C_Str();
	
	clip->FrameRate = (float)animation->mTicksPerSecond;
	clip->FrameCount = (uint)animation->mDuration + 1;
	vector<AsClipNode>animNodeInfos;
	for (uint i = 0; i < animation->mNumChannels; i++)
	{
		aiNodeAnim* animNode = animation->mChannels[i];

		AsClipNode animNodeInfo;
		animNodeInfo.Name = animNode->mNodeName;

		uint keyCount = max(animNode->mNumPositionKeys, animNode->mNumRotationKeys);
		keyCount = max(keyCount, animNode->mNumScalingKeys);

		AsKeyframeData frameData;
		for (uint k = 0; k < keyCount; k++)
		{
			bool bFound = false;
			uint t = animNodeInfo.Keyframe.size();

			if (fabsf((float)animNode->mPositionKeys[k].mTime - (float)t) <= D3DX_16F_EPSILON)
			{//시간에 최대한 근접한 프레임을 구한다.
				aiVectorKey key = animNode->mPositionKeys[k];
				memcpy_s(&frameData.Translation, sizeof(Vector3), &key.mValue,sizeof(aiVector3D));
				frameData.Time = (float)animNode->mPositionKeys[k].mTime;
				bFound = true;
			}

			if (fabsf((float)animNode->mRotationKeys[k].mTime - (float)t) <= D3DX_16F_EPSILON)
			{//시간에 최대한 근접한 프레임을 구한다.
				aiQuatKey key = animNode->mRotationKeys[k];
				frameData.Rotation.x = key.mValue.x;
				frameData.Rotation.y = key.mValue.y;
				frameData.Rotation.z = key.mValue.z;
				frameData.Rotation.w = key.mValue.w;

				frameData.Time = (float)animNode->mRotationKeys[k].mTime;
				bFound = true;
			}

			if (fabsf((float)animNode->mScalingKeys[k].mTime - (float)t) <= D3DX_16F_EPSILON)
			{//시간에 최대한 근접한 프레임을 구한다.
				aiVectorKey key = animNode->mScalingKeys[k];
				memcpy_s(&frameData.Scale, sizeof(Vector3), &key.mValue,sizeof(aiVector3D));
				frameData.Time = (float)animNode->mScalingKeys[k].mTime;
				bFound = true;
			}
			if (bFound == true)
			{
				animNodeInfo.Keyframe.emplace_back(frameData);
			}
		}

		if (animNodeInfo.Keyframe.size() < clip->FrameCount)
		{
			uint count = clip->FrameCount - animNodeInfo.Keyframe.size();
			AsKeyframeData keyframe = animNodeInfo.Keyframe.back();

			for (uint i = 0; i < count; i++)
			{
				animNodeInfo.Keyframe.emplace_back(keyframe);
				//마지막 동작이 계속 이어지게한다.
			}
		}
		clip->Duration = max(clip->Duration, animNodeInfo.Keyframe.back().Time);
		//둘중에 더긴거를 총 길이로쓴다.
		animNodeInfos.emplace_back(animNodeInfo);
	}
	ReadKeyFrameData(clip, scene->mRootNode, animNodeInfos);
	return clip;
}

void Loader::ReadKeyFrameData(AsClip * clip, aiNode * node, vector<struct AsClipNode>& aiModelIn)
{
	static int count = 0;
	
	AsKeyframe* keyframe = new AsKeyframe();
	keyframe->BoneName = node->mName.C_Str();
	//
	/*cout << count++ << endl;
	cout << "KeyFrame:";
	cout << keyframe->BoneName << endl;*/
	
	for (uint i = 0; i < clip->FrameCount; i++)
	{
		AsClipNode* aiNode = nullptr;
		for (uint n = 0; n < aiModelIn.size(); n++)
		{
			if (aiModelIn[n].Name == node->mName)
				aiNode = &aiModelIn[n];
		}

		AsKeyframeData frameData;
		if (aiNode != nullptr)
		{
			frameData = aiNode->Keyframe[i];
		}
		else
		{
			
			Matrix transform(node->mTransformation[0]);
			D3DXMatrixTranspose(&transform, &transform);
			//transform = ConvertMatrix * transform*ConvertMatrix;
			D3DXMatrixDecompose(&frameData.Scale, &frameData.Rotation, &frameData.Translation, &transform);
			frameData.Time = (float)i;
		}
		keyframe->Transforms.emplace_back(frameData);
	}
	clip->Keyframes.emplace_back(keyframe);

	for (uint i = 0; i < node->mNumChildren; i++)
	{
		ReadKeyFrameData(clip, node->mChildren[i], aiModelIn);
	}
}

void Loader::WriteClipData(AsClip * clip, wstring savePath, bool bOverwrite)
{
	if (bOverwrite == false)
	{
		if (Path::ExistFile(savePath) == true)
			return;
	}
	Path::CreateFolders(Path::GetDirectoryName(savePath));
	BinaryWriter* w = new BinaryWriter();
	w->Open(savePath);
	w->String(clip->Name);
	w->Float(clip->Duration);
	w->Float(clip->FrameRate);
	w->UInt(clip->FrameCount);
	w->UInt(clip->Keyframes.size());
	for (AsKeyframe* keyframe : clip->Keyframes)
	{
		w->String(keyframe->BoneName);

		w->UInt(keyframe->Transforms.size());
		w->Byte(&keyframe->Transforms[0], sizeof(AsKeyframeData) * keyframe->Transforms.size());

		SafeDelete(keyframe);
	}

	w->Close();
	SafeDelete(w);

}