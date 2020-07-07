#include "Framework.h"
#include "EffectSystem.h"
#include "Particle/ParticleSimulation.h"
EffectSystem* EffectSystem::instance = NULL;

EffectSystem * EffectSystem::Get()
{
	return instance;
}

void EffectSystem::Create()
{
	assert(instance == NULL);

	instance = new EffectSystem();
}

void EffectSystem::Delete()
{
	SafeDelete(instance);
}

EffectSystem::EffectSystem()
	:index(0)
{
	bStart = true;
	
	position = Vector3(-100, 50, 0);
	g_iNumBodies = 1024;
	g_bShowHelp = false;
	g_bPaused = false;

	g_pointSize = DEFAULT_POINT_SIZE;
	g_fFrameTime = 0.0f;
	//g_iNumBodies = 1;

	g_clusterScale = 1.54f;
	g_velocityScale = 1.0f;
	
	/*
	simulation = new ParticleSimulation();
	Boom_InitBodies(g_iNumBodies);
	simulation->ResetBodies(g_BodyData);
	simulation->SetPointSize(g_pointSize);
*/
//simulation2 = new ParticleSimulation();
//Boom_InitBodies(g_iNumBodies);
//simulation2->ResetBodies(g_BodyData);
//simulation2->SetPointSize(10);
}

EffectSystem::~EffectSystem()
{
}

ID3D11UnorderedAccessView * EffectSystem::UAV(const uint & index)
{
	if (simulations.empty() || simulations.size() <= index)
		return nullptr;

	return simulations[index]->UAV();

	
}

void EffectSystem::ResidenceSharedData(const uint & actorIndex, const uint & instance,class SharedData * sharedData)
{
	if (simulations.empty()|| simulations.size()<= actorIndex)
		return;

	//if (simulations[actorIndex])
	{
		simulations[actorIndex]->ResidenceSharedData(sharedData);
		simulations[actorIndex]->ResetBodies(instance);

	}
	
}


void EffectSystem::Update()
{
	if (!bStart) return;
	ImGui::Begin("Particle", 0, ImGuiWindowFlags_NoMove);
	{


	/*	for (auto& list : simulationList)
		{
			simulations[list]->ImageButton();
			ImGui::SameLine();
		}*/
		for (auto& simulation : simulations)
		{
			simulation->ImageButton();
			ImGui::SameLine();
		}
		if (ImGui::IsWindowHovered())
		{
			if (ImGui::GetIO().MouseDown[1])
				ImGui::OpenPopup("Popup");
		}
		ShowPopUp();
	}
	ImGui::End();

	//for (auto& list : simulationList)
	//{
	//	simulations[list]->Update();

	//}
	//for (auto& simulation : simulations)
	//{
	//	simulation->Update();
	//}

}

void EffectSystem::Render()
{
	/*for (auto& list : simulationList)
	{ 
		simulationsMap[list]->Render();

	}*/
	if (!bActivated) return;
	for (auto& simulation : simulations)
	{
		simulation->Render();
	}


}

void EffectSystem::PreviewRender()
{
	/*for (auto& list : simulationList)
	{
		simulations[list]->PreviewRender();
	}*/
	for (auto& simulation : simulations)
	{
		
		simulation->PreviewRender();
	}
}



void EffectSystem::ShowPopUp()
{
	if (ImGui::BeginPopup("Popup"))
	{
		if (ImGui::MenuItem("Copy")) {}
		if (ImGui::MenuItem("Delete"))
		{
			//DeleteActor();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Add Particle"))
		{
			auto simul = new ParticleSimulation(index++);
			simulations.emplace_back(simul);

		}
		ImGui::EndPopup();
	}
}

