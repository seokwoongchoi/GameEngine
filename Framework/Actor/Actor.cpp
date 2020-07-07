
#include "Framework.h"
#include "Actor.h"
#include "Utilities/Xml.h"
#include "Model/ModelMesh.h"
#include "Model/ModelClip.h"
#include "PreviewRender.h"
#include "Renderables/SkeletalMesh.h"
#include "Renderables/StaticMesh.h"
#include "Components/ActorCollider.h"
#include "Components/Animator.h"

#include "Components/ActorCamera.h"
#include "Components/ActorAi.h"
#include "Environment/TerrainLOD.h"
#include "SharedData.h"
#include "ProgressBar/ProgressReport.h"
//#include "BehaviorTree/BehaviorTree.h"

//BT::BehaviorTree* Bt=nullptr;
Actor::Actor(Shader * shader)
	:shader(shader), 
	model(nullptr),
	bActive(false),	bEditing(false), 
	bFirst(false), bDrag(false),
	bLoaded(false), bModelLoaded(false),
	bBone(false), bBlend(false), bCompiled(false),
    frame(0.0f), pass(0), hasEffect(false),
	mode(EditMode::Render), currentClipnum(0),gizmoType(GizmoType::Default)
{
	
	

	currentBone = nullptr;
	previewRender = new PreviewRender(shader);
	sharedData= new SharedData();
	//ColliderSystem::Get()->SetColliders(sharedData->;
	
	instanceBuffer = new VertexBuffer(worlds, MAX_MODEL_INSTANCE, sizeof(Matrix), 1, true);
	buttonTextures[0] = new Texture(L"playButton.png");
	buttonTextures[1] = new Texture(L"pauseButton.png");
	buttonTextures[2] = new Texture(L"stopButton.png");
	
	
	D3DXMatrixIdentity(&cameraWorld);
	cameraWorld._41 = 0.0f;
	//cameraWorld._42 = 2.66f;
	cameraWorld._42 = -6.0f;
	cameraWorld._43 = 14.75f;
	D3DXMatrixIdentity(&world);
	D3DXMatrixIdentity(&iden);
	
	D3DXMatrixIdentity(&invLocal);

	for (uint i = 0; i < MAX_ACTOR_BONECOLLIDER;i++)
	{
		D3DXMatrixIdentity(&effectData[i].effectWorld);
	}

	/*BT::BehaviorTreeBuilder* Builder = new BT::BehaviorTreeBuilder();
	
		Builder->ActiveSelector();
		Builder->Sequence();
		Builder->Condition(BT::EConditionMode::IsSeeEnemy, false);
		Builder->Back();
		Builder->ActiveSelector();
		Builder->Sequence();
		Builder->Condition(BT::EConditionMode::IsHealthLow, false);
		Builder->Back();
		Builder->Action(BT::EActionMode::Runaway);
		Builder->Back();
		Builder->Back();
		Builder->Parallel(BT::EPolicy::RequireAll, BT::EPolicy::RequireOne);
		Builder->Condition(BT::EConditionMode::IsEnemyDead, true);
		Builder->Back();
		Builder->Action(BT::EActionMode::Attack);
		Builder->Back();
		Builder->Back();
		Builder->Back();
		Builder->Back();
		Builder->Action(BT::EActionMode::Patrol);
		Bt=Builder->End();

	delete Builder;*/

	
	



}

Actor::~Actor()
{
	/*transforms.clear();
	transforms.shrink_to_fit();*/
	renderable->OnDestroy();
	//renderables[renderableType]->OnDestroy();
	//renderables.erase(renderableType);
	for (auto& list : componentTypeList)
	{
		components[list]->OnDestroy();
		components.erase(list);
	}
	
	for (uint i = 0; i < 3; i++)
	{
		SafeDelete(buttonTextures[i]);
	}
	
	SafeDelete(instanceBuffer);
	SafeDelete(previewRender);
	SafeDelete(model);
	//SafeDelete(shader);
	SafeDelete(currentBone);
	//Bt->Release();
}


void Actor::Save()
{
	
	string renderable;
	switch (renderableType)
	{
	case RenderableType::StaticMesh:
		renderable = "StaticMesh";
		break;
	case RenderableType::SkeletalMesh:
		renderable = "SkeletalMesh";
		break;
	default:
		break;
	}
	
	EventSystem::Get()->Writer()->String(renderable);
	EventSystem::Get()->Writer()->String(String::ToString(modelName).c_str());
	EventSystem::Get()->Writer()->UInt(componentTypeList.size()); 
	for (auto& component : componentTypeList)
	{
		
		switch (component)
		{
		case ComponentType::ActorCollider:
			EventSystem::Get()->Writer()->String("ActorCollider");
			break;
		case ComponentType::ActorCamera:
			EventSystem::Get()->Writer()->String("ActorCamera");
			break;
		case ComponentType::ActorLight:
			EventSystem::Get()->Writer()->String("ActorLight");
			break;
		case ComponentType::Animator:
			EventSystem::Get()->Writer()->String("Animator");
			break;
		
		case ComponentType::ActorAi:
			EventSystem::Get()->Writer()->String("ActorAi");
			break;
		}
	
	}
	EventSystem::Get()->Writer()->Int(sharedData->behviorTreeNum);
	
	EventSystem::Get()->Writer()->Int(boxCount);
	if (boxCount > -1)
	{
		for (int i = 0; i < boxCount + 1; i++)
		{
			EventSystem::Get()->Writer()->UInt(colliderIndex[i]);
		}
	}
	

	//EventSystem::Get()->Writer()->Bool(IsBindedTree);
	EventSystem::Get()->Writer()->UInt(sharedData->TotalCount());
	
	auto collider = dynamic_cast<ActorCollider*>(components[ComponentType::ActorCollider]);
	

	uint sequnce = 0;
	vector<uint>culledSort;
	culledSort.reserve(sharedData->culledCount);
	for (uint i = 0; i < sharedData->culledCount; i++)
		culledSort.emplace_back(collider->GetCulledTransformIndex(i));
	
	sort(culledSort.begin(), culledSort.end());

	vector<uint> instanceSort;
	instanceSort.reserve(sharedData->Index.size());
	for (uint i = 0; i < sharedData->Index.size(); i++)
		instanceSort.emplace_back(sharedData->Index[i]);
	
	sort(instanceSort.begin(), instanceSort.end());
	
	
	for (uint i = 0; i < sharedData->TotalCount(); i++)
	{
		
		for (uint c = 0; c < culledSort.size(); c++)
		{
			if (i == culledSort[c])
			{
				Matrix culledMatrix = collider->GetCulledTransforms(c);
				EventSystem::Get()->Writer()->Matrix(culledMatrix);
				
			}
		}

		for (uint s = 0; s < instanceSort.size(); s++)
		{
			if (i == instanceSort[s])
			{
				EventSystem::Get()->Writer()->Matrix(sharedData->transforms[s]);
				
			}
		}
	
		
		
	}

	
	
	
}


void Actor::Load()
{
	
	string renderable = EventSystem::Get()->Reader()->String();
	
	string modelName = EventSystem::Get()->Reader()->String();
	

	
	modelName = modelName + "/" + modelName;
	if (renderable == "StaticMesh")
	{
		LoadStaticMesh(L"../../_Models/StaticMeshes/" + String::ToWString(modelName) );
	}
	else if (renderable == "SkeletalMesh")
	{
		LoadSkeletalMesh(L"../../_Models/SkeletalMeshes/" + String::ToWString(modelName));

	}
	ProgressReport::Get().IncrementJobsDone(ProgressReport::Model);
	uint count = EventSystem::Get()->Reader()->UInt();

	for (uint i = 0; i < count; i++)
	{
		string temp = EventSystem::Get()->Reader()->String();

	
		if (temp == "ActorCollider")
		{
			continue;
		}
		else if (temp == "ActorCamera")
		{
			CreateActorCamera();
		}
		else if (temp == "ActorLight")
		{
			continue;
		}
		
		else if (temp == "Animator")
		{
			continue;
		}
		else if (temp == "Command")
		{
			continue;
		}
		else if (temp == "ActorAi")
		{
			CreateActorAI();
		}

    }
	ProgressReport::Get().IncrementJobsDone(ProgressReport::Model);
	
	
	int btNum= EventSystem::Get()->Reader()->Int();
	if(btNum >-1)
	BehaviorTree(btNum);
	int bcount=EventSystem::Get()->Reader()->Int();
	if(bcount>-1)
	for (int i = 0; i < bcount+1; i++)
	{
		CreateBox(EventSystem::Get()->Reader()->UInt());
	}
	Compile();
	ProgressReport::Get().IncrementJobsDone(ProgressReport::Model);
	uint instanceCount = EventSystem::Get()->Reader()->UInt();

	for (uint i = 0; i < instanceCount; i++)
	{
		sharedData->transforms.emplace_back(EventSystem::Get()->Reader()->Matrix());
		sharedData->Index.emplace_back(sharedData->culledCount + sharedData->drawCount++);

	
		ColliderSystem::Get()->AddDrawCount();
	
	}
	ProgressReport::Get().IncrementJobsDone(ProgressReport::Model);


	
}

void Actor::LoadComplete()
{
	bActive = true;
	bLoaded = true;
	bEditing = true;
}



void Actor::Start()
{

	for (auto& list : componentTypeList)
	{
		components[list]->OnStart();
	}
}

void Actor::Stop()
{
	for (auto& list : componentTypeList)
	{
		components[list]->OnStop();
	}
}

void Actor::Compile()
{
	//Thread::Get()->AddTask([&]()
	//{
		model->CompileMesh();
		auto animator = dynamic_cast<Animator*>(components[ComponentType::Animator]);
		switch (renderableType)
		{
		case RenderableType::StaticMesh:
		{
			auto staticMesh = static_cast<StaticMesh*>(renderable);
			staticMesh->SetModel(model);
			staticMesh->CreateBoneTransforms();
		}
		break;
		case RenderableType::SkeletalMesh:
		{
			auto skeletalMesh = static_cast<SkeletalMesh*>(renderable);
			skeletalMesh->SetModel(model);
			skeletalMesh->CreateBoneTransforms();
			if (animator)
			{
				animator->CreateBoneTransforms();

			}


		}
		break;

		}

		auto collider = dynamic_cast<ActorCollider*>(components[ComponentType::ActorCollider]);
		if (collider)
		{
			Vector3 min, max;
			previewRender->GetBox(&min, &max);
			collider->TranslationBoxWorld(min, max);

			ColliderSystem::Get()->CreateComputeDesc();
			if (animator)
			{

				if (boxCount + 1 > 0)
				{


					for (uint i = 0; i < static_cast<uint>(boxCount) + 1; i++)
						animator->SetBodyBox(colliderIndex[i], previewRender->GetBox(i), i);

				}
			
				if (effectData[0].effectIndex > -2)
				{
					animator->SetEffectBone(
						effectData[0].effectIndex,
						effectData[0].particleIndex);
				}


				if (effectData[1].effectIndex > -2)
					animator->SetEffectBone(
						effectData[1].effectIndex,
						effectData[1].particleIndex);


				//animator->(EffectIndex, temp, particleIndex);



			}

		}

		auto camera = dynamic_cast<ActorCamera*>(components[ComponentType::ActorCamera]);
		if (camera)
		{
			sharedData->IsPlayer = true;
			camera->SetDeltaPos(Vector3(cameraWorld._41, cameraWorld._42, cameraWorld._43));
		}

		bCompiled = true;
	//});
}



void Actor::ActorPos()
{
	
	/*	if (sharedData->transforms.empty()) return ;

	
		for(uint i=0;i< sharedData->drawCount;i++)
		terrain->Intersection(&sharedData->transforms[i]);
		*/
		
	
}



void Actor::Update()
{
	
	if (!bActive)return;
	
		UpdateTransforms();

	for (auto& list : componentTypeList)
	{
		components[list]->OnUpdate();
	}
	
	// cout << sharedData->drawCount << endl;
}

void Actor::Render()
{
	
	if (!bActive)
		return;
	instanceBuffer->Render();

	
	//static_cast<Animator*>(components[ComponentType::Animator])->Render();
	
	renderable->Pass(pass);
	renderable->Render();

	
	/*renderables[renderableType]->Pass(pass);
    renderables[renderableType]->Render();*/
	
	
}



void Actor::PrevRender()
{
	if (!bEditing||!bModelLoaded)return;
	

	
	previewRender->Update();
	previewRender->Render();
	previewRender->DebugRender();
	
}

void Actor::ForwardRender()
{
	if (!bActive)
		return;
	
		
	if (!bBlend)
		return;
	instanceBuffer->Render();
	renderable->ForwardRender();
	
	
}

void Actor::EditingMode()
{
	if (!bEditing || bLoaded) return;


	ImGui::Begin("EditingMode", &bEditing);
	{
		if (ImGui::Button("Aniamtor", ImVec2(80, 30))&&model&&model->ClipCount()>0)
		{
			mode = EditMode::Animator;
		}
		ImGui::SameLine(100);
		if(ImGui::Button("Render", ImVec2(80, 30)))
		{
			mode = EditMode::Render;
			
			
		}
		ImGui::SameLine(200);
	
	
		if (ImGui::Button("Compile", ImVec2(80, 30)))
		{
			Compile();
			
		}
				
	}
	ImGui::End();
	
	
}

void Actor::Editor()
{
	if (!bEditing|| bLoaded) return;

	ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints
	(
		ImVec2(800, 600),
		ImVec2(1280, 720)
	);
	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove;

	//ImGuiWindowFlags_MenuBar|
		
	ImGui::Begin("Editor", &bEditing, windowFlags);
	{
			
		ImVec2 size = ImGui::GetWindowSize();
		
		
		ShowFrame(size);
		

		if (mode == EditMode::Animator)
		{
			ShowAnimList(size);
			
		}
		else if (mode == EditMode::Render)
		{
			ShowComponents(size);
			ShowMaterial(size);
		}


	
	}
	ImGui::End();
}

void Actor::ShowFrame(const ImVec2 & size)
{
	
	ImGui::BeginChild("##Frame", ImVec2(size.x*0.5f, 0), true, ImGuiWindowFlags_NoScrollbar);
	{
		
		    previewRender->PersPective()->Set(size.x * 0.5f, size.y);
			
			ImGui::Image
			(
				previewRender->SRV()? previewRender->SRV():nullptr, ImVec2(size.x*0.5f, size.y)
			);
			ImGuizmo::SetDrawlist();
			ImGizmo(size);
			

		
			
			
			
	}
	ImGui::EndChild();

}

void Actor::ShowAnimList(const ImVec2 & size)
{
	
	ImGui::SameLine();
	ImGui::BeginChild("##ClipList", ImVec2(size.x*0.5f - 70.0f, 0), true);
	{
		

		if (ImGui::CollapsingHeader("ClipList", ImGuiTreeNodeFlags_DefaultOpen))
		{
			string clipName = "N/A";
		//	ImVec2 listBoxSize = ImVec2(ImGui::GetWindowContentRegionMax().x * 10.0f, 30.0f *clipList.size());
			ImGui::Columns(1, "my", false);
			ImGui::Separator();
			//ImGui::ListBoxHeader("##ComponentList", listBoxSize);
			{
				for (uint i=0; i<clipList.size();i++)
				{
					string name = clipList[i];
					const char* label = name.c_str();
					bool bSelected = clipName == clipList[i];
					if (ImGui::Selectable(label, bSelected, 0, ImGui::CalcTextSize(label)))
					{
						clipName = clipList[i];
						currentClip = clipList[i];
						currentClipnum = i;
						
						previewRender->SetClip(i);
						

					}
					ImGui::NextColumn();
					
				}
				ImGui::Columns(1);
				ImGui::Separator();
				//ImGui::ListBoxFooter();
			}
		}
		if (ImGui::CollapsingHeader("Frame", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ShowAnimFrame(size);
		}
		
	}
	
	ImGui::EndChild();
	

}

void Actor::ShowAnimFrame(const ImVec2 & size)
{
	
	auto clip=model->ClipByIndex(currentClipnum);
	auto animator = previewRender;
	frame = animator->GetPreviewFrame();
	//ImGui::BeginChild("##AnimFrame", ImVec2(size.x*0.25f - 70.0f, 30), true);
	ImGui::SliderFloat("Frame", (float*)&frame, 0.0f, clip->FrameCount()-1);
	if (animator)
	{
		animator->SetFrame(frame);
	}
	if (ImGui::ImageButton(buttonTextures[0]->SRV(), ImVec2(20.0f, 20.0f)))//play
	{
	
		animator->StartAnimation(true);
	}
	ImGui::SameLine();
	if (ImGui::ImageButton(buttonTextures[1]->SRV(), ImVec2(20.0f, 20.0f)))//pause
	{

		animator->StartAnimation(false);
	}
	ImGui::SameLine();
	if (ImGui::ImageButton(buttonTextures[2]->SRV(), ImVec2(20.0f, 20.0f)))//stop
	{

	}
	//ImGui::EndChild();
}

void Actor::ClipFinder(wstring file)
{
	wstring filePath = L"../../_Models/SkeletalMeshes/"+ file+L"/";
	vector<wstring> files;
	
	wstring filter = L"*.clip";
	Path::GetFiles(&files, filePath, filter, false);
	
	
	for (uint i = 0; i < files.size(); i++)
	{
		auto fileName = Path::GetFileName(files[i]);
		wstring noExfileName = Path::GetFileNameWithoutExtension(files[i]);
		clipList.emplace_back(String::ToString(noExfileName));
	}
}

bool CompareName(string src, string dst)
{
	uint count = src.length();


	for (uint i = 0; i < count; i++)
	{

		if (src[i] != dst[i]) return false;
	}

	return true;
}
void Actor::ShowComponents(const ImVec2 & size)
{
	
	ImGui::SameLine();
	ImGui::BeginChild("##Components", ImVec2(size.x*0.25f - 70.0f, 0), true);
	{
		
		if (ImGui::CollapsingHeader("Components", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::Button("Add Component"))
			{
				ImGui::OpenPopup("Component Popup");
			}
			ShowComponentPopUp();
			
			//ImVec2 listBoxSize = ImVec2(ImGui::GetWindowContentRegionMax().x * 10.0f, 30.0f *componentList.size());
		
			
			ImGui::Columns(1, "mycolumns3", false);  // 3-ways, no border
			ImGui::Separator();
		//	ImGui::ListBoxHeader("##ComponentList", listBoxSize);
			{
				for (uint i=0;i< 10;i++)
				{
					if (componentList.size() > i)
					{
						string name = componentList[i];
						const char* label =  name.c_str();
					
						
						ImGuiTreeNodeFlags  flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

						if (ImGui::TreeNodeEx(label, flags))
						{
							if (ImGui::IsItemClicked())
							{
								componentName = componentList[i];
								if (componentName == "Collider")
								{
									gizmoType = GizmoType::Box;
								}
								else if (componentName == "BoneCollider0")
								{
									gizmoType = GizmoType::Box1;
								}
								else if (componentName == "BoneCollider1")
								{
									gizmoType = GizmoType::Box2;
								}
								else if (componentName == "Effect0")
								{
									gizmoType = GizmoType::Effect1;
								}
								else if (componentName == "Effect1")
								{
									gizmoType = GizmoType::Effect2;
								}
								else if (componentName == "ActorCamera")
								{
									gizmoType = GizmoType::ActorCamera;
								}

							}
							if (IsBindedTree&&componentList[i]=="ActorAi")
							{
								string treeName = "BehaviorTree"+ to_string(sharedData->behviorTreeNum);
								if (ImGui::TreeNodeEx(treeName.c_str(), flags))
								{
									ImGui::TreePop();
								}
							}
							ImGui::TreePop();
						}
						
					}
					else
					{
						//string name ="";
						const char* label = "";
						//bSelected = componentName == componentList[i];
						//index = i;
						if (ImGui::Selectable(label)) {}
					}
					
					ImGui::NextColumn();
				}
				if (!componentName.empty() &&ImGui::IsWindowHovered() && ImGui::IsAnyItemHovered())
				if (ImGui::GetIO().MouseDown[1])
					ImGui::OpenPopup("ComponentList Popup");
				ShowComponentListPopUp(componentName);
				ImGui::Columns(1);
				ImGui::Separator();
				//ImGui::ListBoxFooter();
				
			}

			
			if (bModelLoaded)
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				
				auto previewTransform = previewRender->PreviewTransform();
				Quaternion rotation;
				previewTransform->Rotation(&rotation);

				Vector3 scale;
				previewTransform->Scale(&scale);
			

				const auto show_float = [](const char* id, const char* label, float* value)
				{
					ImGui::PushItemWidth(100.0f);
					ImGui::PushID(id);
					ImGui::InputFloat(label, value, 1.0f, 1.0f, "%.3f", ImGuiInputTextFlags_CharsDecimal);
					ImGui::PopID();
					ImGui::PopItemWidth();
				};
				if (currentBone)
				{
					if (currentBone->BoneIndex() > 0)
					{
						auto local = model->BoneByIndex(currentBone->BoneIndex())->Transform();
						Vector3 position = Vector3(local._41, local._42, local._43);
						//previewTransform->Position(&position);

						ImGui::TextUnformatted("BoneTranslation");
						//ImGui::SameLine(70.0f); 
						show_float("##pos_x", "X", &position.x);
						//ImGui::SameLine();    
						show_float("##pos_y", "Y", &position.y);
						//ImGui::SameLine();     
						show_float("##pos_z", "Z", &position.z);
						/*Matrix S, R, T;
						Vector3 position1, scale;
						Quaternion q;
						D3DXMatrixDecompose(&scale, &q, &position1, &local);
						D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
						D3DXMatrixRotationQuaternion(&R, &q);
						D3DXMatrixTranslation(&T, position.x, position.y, position.z);
						model->ChangeBone(S*T*R, currentBone->BoneIndex());*/
					}
					

					
				}
				
				ImGui::TextUnformatted("Rotation");
				//ImGui::SameLine(70.0f); 
				show_float("##rot_x", "X", &rotation.x);
				//ImGui::SameLine();     
				show_float("##rot_y", "Y", &rotation.y);
				//ImGui::SameLine();     
				show_float("##rot_z", "Z", &rotation.z);

				ImGui::TextUnformatted("Scale");
				//ImGui::SameLine(70.0f); 
				show_float("##scl_x", "X", &scale.x);
				//ImGui::SameLine();     
				show_float("##scl_y", "Y", &scale.y);
				//ImGui::SameLine();      
				show_float("##scl_z", "Z", &scale.z);

				
				//previewTransform->Position(position);
				previewTransform->Rotation(rotation);
				previewTransform->Scale(scale);
				
			}

		}
	}

	ImGui::EndChild();
}

void Actor::ShowMaterial(const ImVec2 & size)
{
	const auto ShowTextureSlot = [&](ModelMesh* mesh,uint num)//, TextureType type
	{
		Material* material = mesh->Material();
		
		Texture* texture1 = material->DiffuseMap();
		ImGui::Text(String::ToString(mesh->MaterialName()).c_str());
		ImGui::SameLine(70.0f);

		function<void(wstring, uint, Material*)> f;
		
		if (ImGui::ImageButton
			(
				texture1 ? texture1->SRV() : nullptr,
				ImVec2(80, 80)
				
				))
		{
			LoadMaterial(f,num,material);
		}
		ImGui::Text("Color");
		ImGui::SameLine(70.0f);
		Color Diffuse = material->Diffuse();
		ImGui::ColorEdit4(String::ToString(mesh->MaterialName()).c_str(), Diffuse);
		material->Diffuse(Diffuse);
		

	};

	const auto ShowTextureSlotRoughness = [&](ModelMesh* mesh, uint num)//, TextureType type
	{
		Material* material = mesh->Material();

		Texture* texture1 = material->RoughnessMap();
		ImGui::Text(String::ToString(mesh->MaterialName()).c_str());
		ImGui::SameLine(70.0f);

		function<void(wstring, uint, Material*)> f;

		if (ImGui::ImageButton
		(
			texture1 ? texture1->SRV() : nullptr,
			ImVec2(80, 80)

		))
		{
			LoadMaterial(f, num, material);
		}
		ImGui::Text("Color");
		ImGui::SameLine(70.0f);
		float roughness = material->Roughness();
		ImGui::SliderFloat(String::ToString(mesh->MaterialName()).c_str(), &roughness,0.0f,20.0f);
	
		material->Roughness(roughness);
		//DragDrop(material, type);

	};

	ImGui::SameLine();
	ImGui::BeginChild("##Edit", ImVec2(size.x*0.25f + 30.0f, 0), true);
	{

		if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
		{

			
			if (model)
			{
				
				auto count = model->previewMeshesCount();
				
				auto mesh = model->previewMeshsData();
				if (ImGui::CollapsingHeader("Diffuse", ImGuiTreeNodeFlags_OpenOnDoubleClick))
				{
					
					for (uint i=0; i<count; i++)
					{
							//Albedo
							ShowTextureSlot(mesh[i],0);
							ImGui::Separator();
										
					}
					if (bBlend)
					{
						count = model->forwardMeshesCount();
						mesh = model->forwardMeshsData();
						for (uint i = 0; i < count; i++)
						{
							//Albedo
							ShowTextureSlot(mesh[i], 0);
							ImGui::Separator();

						}
						
					}
				}
				if (ImGui::CollapsingHeader("Roughness", ImGuiTreeNodeFlags_OpenOnDoubleClick))
				{
					
					for (uint i = 0; i < count; i++)
					{
						

						    ShowTextureSlotRoughness(mesh[i], 1);
							ImGui::Separator();
							

						
					}
					if (bBlend)
					{
						count = model->forwardMeshesCount();
						mesh = model->forwardMeshsData();
						for (uint i = 0; i < count; i++)
						{
							
							ShowTextureSlotRoughness(mesh[i], i);
							ImGui::Separator();

						}

					}
				}


			}

			ImGui::Separator();
		}
	}
	ImGui::EndChild();
}

void Actor::ShowHierarchy()
{
	if (!bEditing || bLoaded) return;


	ImGui::Begin("Hierarchy", &bEditing, ImGuiWindowFlags_HorizontalScrollbar);
	{
		if (model)
		{
			
			switch (renderableType)
			{
			case RenderableType::StaticMesh:
			{
				auto bone = model->BoneData();
				if (bone)
				for (uint i = 0; i < model->BoneCount(); i++)
				{
				   ShowBone(bone[i]);
				}
			}
				break;
			case RenderableType::SkeletalMesh:
			{
				ModelBone* root = nullptr;
				
				auto bone = model->BoneData();
				uint count = model->BoneCount();
				for (uint i = 0; i < count; i++)
				{
					
					if (bone[i]->ChildsData())
					{
						root = bone[i];
						break;
					}

				}
				ShowChild(root);
				
				
			}
				break;
			default:
				break;
			}
			

           
				
			
			
			if (currentBone&&ImGui::IsWindowHovered() && ImGui::IsAnyItemHovered())
			{

				if (ImGui::GetIO().MouseDown[1])
					ImGui::OpenPopup("Hierarchy Popup");
			}
			ShowPopup();
		}
	}
	ImGui::End();
}

void Actor::ShowBone(ModelBone* bone)
{
	
	
	{
		//uint childCount = bone->ChildCcount();
		//auto childs = bone->ChildsData();
		ImGuiTreeNodeFlags  flags =  ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		static wstring temp = L"";
		if (temp == bone->Name())
			flags |= ImGuiTreeNodeFlags_Selected;



		//Matrix world;
		//D3DXMatrixIdentity(&world);
		//auto V = Context::Get()->View();
		//auto P = Context::Get()->Projection();
		//Vector3 projection;
		//Context::Get()->GetViewport()->Projection(&projection, Vector3(bone->Transform()._41,
		//	bone->Transform()._42, bone->Transform()._43), world, V, P);
		//Gui::Get()->RenderText(projection.x, projection.y, 1, 1, 1,
		//	String::ToString(bone->Name()));

		ImGui::PushItemWidth(1.0f);
		
			if (ImGui::TreeNodeEx(String::ToString(bone->Name()).c_str(), flags))
			{
				if (ImGui::IsItemClicked())
				{
					currentBone = bone;
					if (currentBone)
					{
						gizmoType = GizmoType::Bone;
					}
					temp = bone->Name();

				}

								

				ImGui::TreePop();

			}
			ImGui::PopItemWidth();
		}
		
		
	
}

void Actor::ShowChild(ModelBone * bone)
{
	auto child = bone->ChildsData();
	ImGuiTreeNodeFlags  flags = !child ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_OpenOnDoubleClick : ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	static wstring temp = L"";
	if (temp == bone->Name())
		flags |= ImGuiTreeNodeFlags_Selected;
	
	
	
	//Matrix world;
	//D3DXMatrixIdentity(&world);
	//auto V = Context::Get()->View();
	//auto P = Context::Get()->Projection();
	//Vector3 projection;
	//Context::Get()->GetViewport()->Projection(&projection, Vector3(bone->Transform()._41,
	//	bone->Transform()._42, bone->Transform()._43), world, V, P);
	//Gui::Get()->RenderText(projection.x, projection.y, 1, 1, 1,
	//	String::ToString(bone->Name()));

	ImGui::PushItemWidth(1.0f);
	if (ImGui::TreeNodeEx(String::ToString(bone->Name()).c_str(), flags))
	{
		if (ImGui::IsItemClicked())
		{
			currentBone = bone;
			if (currentBone)
			{
				gizmoType = GizmoType::Bone;
			}
			temp = bone->Name();
			
		}
		uint count = bone->ChildCcount();
		for (uint i=0;i< count;i++)
		{
			ShowChild(child[i]);

		}
		ImGui::TreePop();
	}
	ImGui::PopItemWidth();
}

void Actor::ShowPopup()
{
	if (ImGui::BeginPopup("Hierarchy Popup"))
	{
		if (ImGui::MenuItem("Copy")) {}
		if (ImGui::MenuItem("Delete")) 
		{
			model->DeleteBone(currentBone);
		}

		ImGui::Separator();

		if (ImGui::BeginMenu("Create Mesh"))
		{

			if (ImGui::MenuItem("Create"))	CreateWeaponMesh();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("BlendMesh"))
		{
			if (ImGui::MenuItem("Blend"))	BlendMesh();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Create Collider"))
		{
			if (ImGui::MenuItem(" Collider"))	CreateBox();
			
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Create Effect"))
		{
			uint count = EffectSystem::Get()->SimulationsCount();
			for (uint i = 0; i < count; i++)
			{
				const string& temp = "Effect" + to_string(i);
					if (ImGui::MenuItem(temp.c_str()))	CreateEffect(i);
			}
		
			ImGui::EndMenu();
		}
	
			
		



		ImGui::EndPopup();
	}
}



void Actor::BlendMesh()
{
	if (!currentBone) return;
	
	model->BlendModelMeshes(currentBone); 
	switch (renderableType)
	{
	case RenderableType::StaticMesh:
		static_cast<StaticMesh*>(renderable)->BlendModelBone(previewRender->BlendModelBone(currentBone->BoneIndex()));
		break;
	case RenderableType::SkeletalMesh:
		static_cast<SkeletalMesh*>(renderable)->BlendModelBone(previewRender->BlendModelBone(currentBone->BoneIndex()));
		break;
	default:
		break;
	}
	
	bBlend = true;
	previewRender->SetBlend(bBlend);

}

void Actor::CreateBox()
{
	boxCount++;
	if (!currentBone|| boxCount> MAX_ACTOR_BONECOLLIDER-1) return;
	
	
	colliderIndex[boxCount] = currentBone->BoneIndex();
	
	previewRender->SetColliderBoxIndex(colliderIndex[boxCount], boxCount);
	
	
	
	componentList.emplace_back("BoneCollider"+ to_string(boxCount));
	
	//CreateComputeDesc();
}

void Actor::CreateBox(const uint & index)
{
	boxCount++;
	if ( boxCount > MAX_ACTOR_BONECOLLIDER - 1) return;


	colliderIndex[boxCount] = index;

	previewRender->SetColliderBoxIndex(colliderIndex[boxCount], boxCount);



	componentList.emplace_back("BoneCollider" + to_string(boxCount));
}


void Actor::CreateEffect(const uint& index)
{
	particleCount++;
	if (!currentBone || particleCount > MAX_ACTOR_BONECOLLIDER - 1) return;
	Matrix temp;
	D3DXMatrixIdentity(&temp);
	model->AddBone(-1, "EffectBone" + to_string(particleCount), currentBone->ParentIndex(), temp);
	effectData[particleCount].effectIndex = model->BoneByName(L"EffectBone" + to_wstring(particleCount))->BoneIndex();
	effectData[particleCount].particleIndex = index;
	previewRender->SetEffectIndex(effectData[particleCount].effectIndex, particleCount);

	componentList.emplace_back("Effect" + to_string(index));

}
void Actor::ImGizmo(const ImVec2 & size)
{
 if (!bEditing || gizmoType==GizmoType::Default) return;
  
      Gui::Get()->AddTransform(nullptr);
      
      static ImGuizmo::OPERATION operation(ImGuizmo::TRANSLATE);
      //static ImGuizmo::MODE mode(ImGuizmo::WORLD);
     static ImGuizmo::MODE mode(ImGuizmo::LOCAL);
      if (ImGui::IsKeyPressed('T'))//w
      	operation = ImGuizmo::TRANSLATE;
      if (ImGui::IsKeyPressed('Y'))//e
      	operation = ImGuizmo::ROTATE;
      if (ImGui::IsKeyPressed('U'))//r
      	operation = ImGuizmo::SCALE;

	  ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, size.x*0.5f, size.y);
	  
	 
	  previewRender->OrbitView(&orbitView);
	 
	  previewRender->OrbitProj(&orbitProj);
	  D3DXMatrixIdentity(&world);
	  
	  switch (gizmoType)
	  {
	  case GizmoType::Bone:
	  {
		  
		  matrix = previewRender->GetSkinnedBoneTransform(currentBone->BoneIndex());
		
		  world =  matrix*previewRender->PreviewTransform()->World();
 	  }
	  break;

	  case GizmoType::ActorCamera:
	  {
		  
		  
		
		 world = cameraWorld;
	  }
	  break;
	/*  case GizmoType::Effect1:
	  {
		
		  world = previewRender->GetEffectBoneMatrix(0)* effectData[0].effectWorld;

		 
	  }
	  break;
	  case GizmoType::Effect2:
	  {
		 
		  world = previewRender->GetEffectBoneMatrix(1)* effectData[1].effectWorld;
		 
	  }
	  break;*/

	  default:
	  {
		  previewRender->GetBoxWorld(&world, (uint)gizmoType);
	  }
	  break;
	  }
	 
  	  ImGuizmo::Manipulate(orbitView, orbitProj, operation, mode,
	  world, nullptr, &snap.x);
  
	  switch (gizmoType)
	  {
	  case GizmoType::Bone:
	  {
		  local = model->BoneByIndex(currentBone->BoneIndex())->Transform();
		  D3DXMatrixInverse(&invLocal, nullptr, &local);
		  parent = invLocal*matrix* previewRender->PreviewTransform()->World();
		  D3DXMatrixInverse(&parentInv, nullptr, &parent);
			   
		  Matrix newLocal =  world* parentInv;
	
		 
		  //if (Keyboard::Get()->Down('N'))
		  //{
			
				//  

			 // FILE* file = fopen("Test.csv", "w");
			 //// for (UINT i = 0; i < 4; i++)
			 // {
				//
				//  fprintf(file,
				//	  "%.0f,%.0f,%.0f,%.0f\n", 
				//	  newLocal._11, newLocal._12, newLocal._13, newLocal._14
				//	 
				//  );
				//  fprintf(file,
				//	  "%.0f,%.0f,%.0f,%.0f\n",
				//	   newLocal._21, newLocal._22, newLocal._23, newLocal._24
				//	 
				//  );
				//  fprintf(file,
				//	  "%.0f,%.0f,%.0f,%.0f\n",
				//	  newLocal._31, newLocal._32, newLocal._33, newLocal._34
				//	 
				//  );
				//  fprintf(file,
				//	  "%.0f,%.0f,%.0f,%.0f\n",
				//	  
				//	  newLocal._41, newLocal._42, newLocal._43, newLocal._44
				//  );
			 // }
			 // fclose(file);
			 // int a = 0;
		  //}
	  	  if (currentBone->BoneIndex() > 0)
	  	  {
	  	   	
	  	   	model->ChangeBone(newLocal, currentBone->BoneIndex());
				
	  	  }
	  		
	  }
	  break;
	 
	  case GizmoType::ActorCamera:
	  {
		  cameraWorld = world;
	  }
	  break;
	  //case GizmoType::Effect1:
	  //{
		 // parent = previewRender->GetEffectBoneMatrix(0);
		 // D3DXMatrixInverse(&parentInv, nullptr, &parent);
		 // effectData[0].effectWorld = parentInv* world ;
		 //// D3DXMatrixInverse(&effectData[0].effectWorld, nullptr, &effectData[0].effectWorld);
		 //// sharedData->effectTranslation[0] = Vector3(effectData[0].effectWorld._41, effectData[0].effectWorld._42, effectData[0].effectWorld._43);
	  //    
	  //}
	  //break;
	  //case GizmoType::Effect2:
	  //{
		 // parent = previewRender->GetEffectBoneMatrix(1);
		 // D3DXMatrixInverse(&parentInv, nullptr, &parent);
		 // effectData[1].effectWorld = parentInv * world;
		 //// D3DXMatrixInverse(&effectData[1].effectWorld, nullptr, &effectData[1].effectWorld);
		 // //sharedData->effectTranslation[1] = Vector3(effectData[1].effectWorld._41, effectData[1].effectWorld._42, effectData[1].effectWorld._43);
		 //
	  //}
	  //break;
	  
	  default:
	  {
		  previewRender->SetBoxWorld(world, (uint)gizmoType);
	  }
	  break;
	  }
	
     
			
}


void Actor::ShowComponentPopUp()
{
	if (ImGui::BeginPopup("Component Popup"))
	{
		if (ImGui::MenuItem("SkeletalMesh"))
		{
			HWND hWnd = NULL;
			function<void(wstring)> f = bind(&Actor::LoadSkeletalMesh, this, placeholders::_1);
			Path::OpenFileDialog(L"", Path::MeshFilter, L"../../_Models/SkeletalMeshes/", f, hWnd);
		}
		if (ImGui::MenuItem("StaticMesh"))
		{
			HWND hWnd = NULL;
			function<void(wstring)> f = bind(&Actor::LoadStaticMesh, this, placeholders::_1);
			Path::OpenFileDialog(L"", Path::MeshFilter, L"../../_Models/StaticMeshes/", f, hWnd);
		}
		if (ImGui::MenuItem("ActorCamera"))
		{
			if (!components[ComponentType::ActorCamera])
			{
				
				//components.emplace_back(animator);
				CreateActorCamera();
			}

		


		}
		if (ImGui::MenuItem("ActorAI"))
		{
			if (!components[ComponentType::ActorAi])
			{

				CreateActorAI();
			}




		}
	
		ImGui::EndPopup();
	}
	
	
}

void Actor::ShowComponentListPopUp(string componentName)
{
	if (ImGui::BeginPopup("ComponentList Popup"))
	{
		if (ImGui::MenuItem("Delete"))
		{
			if (componentName == "ActorCamera")
			{
				
				for (uint i = 0; i < componentList.size(); i++)
				{
					if (componentList[i] == "ActorCamera")
					{
						componentList.erase(componentList.begin() + i);
					}
				}
				
				for (auto& iter = componentTypeList.begin(); iter != componentTypeList.end();)
				{
					auto type = *iter;
					if (type == ComponentType::ActorCamera)
						
					{

						iter = componentTypeList.erase(iter);

					}
					else
						iter++;
				}
				
				SafeDelete(components[ComponentType::ActorCamera]);
				
				
			}
			
		
			

		}
		if (componentName == "ActorAi")
		{
			//if (ImGui::MenuItem("BehaviorTree"))
			{

				uint count = EventSystem::Get()->GetBTData();
				for (uint i = 0; i < count; i++)
				{
					const string& temp = "BehaviorTree" + to_string(i);
					if (ImGui::MenuItem(temp.c_str()))
					{
						
						BehaviorTree(i);
					}
						
				}
				

			}
		}

		ImGui::EndPopup();
	}
}



void Actor::CreateWeaponMesh()
{
	HWND hWnd = NULL;
	function<void(wstring)> f = bind(&Actor::LoadWeapon, this, placeholders::_1);
	Path::OpenFileDialog(L"", Path::EveryFilter, L"../../_Models/", f, hWnd);
}

void Actor::LoadWeapon(wstring name)
{
	bCompiled = false;
	name = Path::GetFileNameWithoutExtension(name);
	cout << String::ToString(name) << endl;
	auto path = name + L"/" + name;

	auto createModel = new Model();
	createModel->ReadMaterial(path);
	createModel->ReadMesh(name);
	
	//if (String::ToString(name) == "M4")
	//{
	//	Matrix S, R, T;
	//	
	//	D3DXMatrixTranslation(&T, 4.0f, 28.998f,-5.99f);
	//	D3DXMatrixScaling(&S, 14.0473f, 14.0473f, 14.0473f);
	//	//D3DXMatrixScaling(&S, 5.0f, 5.0f, 5.0f);
	//	D3DXMatrixRotationQuaternion(&R, &Quaternion(-0.526194f, 0.364066f, 0.555006f, 1.0f));
	//	

	//	Matrix newLocal = S *T*R; //pureLocal*delta
	//			
	//	 
	//
	//	createModel->ChangeBone(newLocal, createModel->BoneByName(L"4_Cube.007")->BoneIndex());
	//	
	//	
	//}

	model->Attach(shader, createModel, currentBone->BoneIndex());
	

	
	
	
	
	
	//static_cast<SkeletalMesh*>(components[ComponentType::SkeletalMesh])->Attach(model, currentBone);
	
}

void Actor::LoadSkeletalMesh(wstring & file)
{
	bCompiled = false;
	auto name = Path::GetFileNameWithoutExtension(file);

	model = new Model();
	model->ReadMaterial(name + L"/" + name);
	//model->ReadMesh(name + L"/" + name);
	model->ReadPreviewMesh(name,0);
	modelName = name;
	ClipFinder(name);
	for (auto& clip : clipList)
	{
		model->ReadClip(name + L"/" + String::ToWString(clip));
	}

	
	this->name = String::ToString(name);

	previewRender->SetModel( model);
	if (!clipList.empty())
	{
		auto animator=new Animator(shader, model, sharedData);
		components[ComponentType::Animator] = animator;
		componentTypeList.emplace_back(ComponentType::Animator);
		EventSystem::Get()->ResidenceAnimator(animator);
		
		
	}
	auto skeltalMesh = new SkeletalMesh(shader,sharedData);
	skeltalMesh->SetModel(model);
	skeltalMesh->SetAnimator(static_cast<Animator*>(components[ComponentType::Animator]));
	renderableType = RenderableType::SkeletalMesh;
	
	renderable = skeltalMesh;
	componentList.emplace_back("SkeltalMesh");
	CreateColliderComponent();
	bModelLoaded = true;
}


void Actor::LoadStaticMesh(wstring & file)
{
	bCompiled = false;
	auto name = Path::GetFileNameWithoutExtension(file);

	model = new Model();
	model->ReadMaterial(name + L"/" + name);
	//model->ReadMesh(name + L"/" + name);
	model->ReadPreviewMesh(name, 1);


	this->name = String::ToString(name);

	previewRender->SetModel(model);
	auto staticMesh = new StaticMesh(shader,  sharedData);
	staticMesh->SetModel(model);
	renderableType = RenderableType::StaticMesh;
	
	renderable = staticMesh;
	componentList.emplace_back("StaticMesh");
	CreateColliderComponent();
	static_cast<ActorCollider*>(components[ComponentType::ActorCollider])->CalcInstBuffer();
	EventSystem::Get()->ResidenceAnimator(nullptr);
	bModelLoaded = true;
}

void Actor::CreateColliderComponent()
{
	components[ComponentType::ActorCollider] = new ActorCollider(shader, model, sharedData);
	componentTypeList.emplace_back(ComponentType::ActorCollider);
	componentList.emplace_back("Collider");
}

void Actor::CreateActorAI()
{
	components[ComponentType::ActorAi] = new ActorAi(shader, model, sharedData);
	componentTypeList.emplace_back(ComponentType::ActorAi);
	componentList.emplace_back("ActorAi");
}

void Actor::BehaviorTree(const uint& num)
{
	IsBindedTree = true;
	static_cast<ActorAi*>(components[ComponentType::ActorAi])->SetBehaviorTree(num);
}

void Actor::CreateActorCamera()
{

	components[ComponentType::ActorCamera] = new ActorCamera(shader, model, sharedData);
	componentTypeList.emplace_back(ComponentType::ActorCamera);
	componentList.emplace_back("ActorCamera");
}

void Actor::LoadMaterial(function<void(wstring, uint, Material*)> f,uint num,Material * material)
{
	HWND hWnd = NULL;
	
	f= bind(&Actor::SetMaterial, this, placeholders::_1, placeholders::_2, placeholders::_3);
	Path::OpenFileDialog(L"", Path::EveryFilter, L"../../_Textures/", num, material, f, hWnd);


}

void Actor::SetMaterial(wstring & file, uint textureType, Material * material)
{
	switch (textureType)
	{
	case 0:
	{
		material->DiffuseMap(file);
	}
	break;
	case 1:
	{
		material->RoughnessMap(file);
	}
	break;
	case 2:
	{

	}
	break;
	default:
		break;
	}
}



void Actor::AddTransform(  const Matrix& transform)
{
	if (!bDrag||!model||!bCompiled) return;
	if (Mouse::Get()->Up(0) && bDrag)
	{
		
		
		sharedData->transforms.emplace_back(transform);
		Gui::Get()->AddTransform(&sharedData->transforms.back());
		
		
		sharedData->Index.emplace_back(sharedData->culledCount+sharedData->drawCount++);
	
		/*for (auto& list : componentTypeList)
		{
			components[list]->DrawCount(drawCount);
			components[list]->AddTransform(transforms.back());
		}*/
		//if(renderableType==RenderableType::SkeletalMesh)
		ColliderSystem::Get()->AddDrawCount();

		
		
		bActive = true;
		
		bDrag = false;
		
	}

}


void Actor::UpdateTransforms()
{

	for (uint i = 0; i < sharedData->drawCount; i++)
	{
		memcpy(worlds[i], sharedData->transforms[i], sizeof(Matrix));
	}
	
	D3D::GetDC()->Map(instanceBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, worlds, sizeof(Matrix) * sharedData->drawCount);
	}
	D3D::GetDC()->Unmap(instanceBuffer->Buffer(), 0);


	
}






void Actor::SetDragDropPayload( const string & data)
{
	
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		bDrag = true;
		ImGui::Text(Path::GetFileNameWithoutExtension(data).c_str());
		ImGui::EndDragDropSource();
	}
}






void Actor::ImageButton()
{
	
	if (ImGui::ImageButton(previewRender->SRV()? previewRender->SRV():nullptr, ImVec2(80, 80)))
	{
		bLoaded = false;
		bEditing = true;
	}
	SetDragDropPayload( name);

	EditingMode();
	Editor();
	ShowHierarchy();
	
}

Transform * Actor::PreviewTreansform()
{
	return previewRender?previewRender->PreviewTransform():nullptr;
}







