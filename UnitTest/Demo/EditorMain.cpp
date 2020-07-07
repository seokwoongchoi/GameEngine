#include "stdafx.h"
#include "EditorMain.h"
#include "Renders/GBuffer/GBufferData.h"
#include "ScreenSpaceEffects/SSAO.h"
#include "ScreenSpaceEffects/SSLR.h"
#include "PostEffects/PreFilter.h"
#include "PostEffects/HDR.h"

//#include "PostEffects/HDRBloom.h"
#include "Environment/TerrainLod.h"
#include "Environment/Sky/Atmosphere.h"
#include "Environment/Sky/Cloud.h"
#include "Environment/Fog.h"
#include "Environment/Ocean/OceanRenderer.h"
#include "Objects/LightObjects.h"
#include "Actor/Actor.h"
#include "ImGui/Blueprints/Blueprints.h"
#include "ProgressBar/ProgressBar.h"
#include "ProgressBar/ProgressReport.h"

#include "Environment/Island11.h"

class GBufferData GBuffer;

void EditorMain::Initialize()
{
	mainDSV = nullptr;
	depthSRV = nullptr;
	Application_Initialize();
	actorIndex = 0;
	pos = Vector3(0, 0, 0);
	//	selectT = new Transform();

	float width = D3D::Width();
	float height = D3D::Height();

	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 46, -85);


	{
		deferredShader = new Shader(L"Deferred/DeferredPBRoptimizition2.fx");
		GBuffer.Init(deferredShader);
		Context::Get()->SetShader(deferredShader);
		dirLight = new DeferredDirLight(deferredShader);
	
		


		//deferredShader->AsSRV("CascadeShadowMapTexture")->SetResource(dirLight->SRV());
		dirLight->InitShadowMap(1280);
        pointLight = new DeferredPointLight(deferredShader);
		spotLight = new DeferredSpotLight(deferredShader);

		ssao = new SSAO(width, height);


		sslr = new SSLR(width, height);
		//hdr = new HDRBloom(width, height);
		hdr = new HDR(width, height);


		CreateHDRRTV(width, height);
		CreateRender2DTarget(width, height);
		CreateObjects();
		//CreateLights();
	}


	//Environment
	{
		fog = new Fog(deferredShader);
		sky = new Atmosphere(deferredShader);
		cloud = new Cloud(deferredShader);
		island11 = new Island11(deferredShader);
		TerrainLOD::InitializeInfo info =
		{
		   deferredShader,
		   L"HeightMaps/HeightMap512.png",
		   1.0f, 32, 20
		};
		////D3D11_QUERY_DESC query;
		//
		terrain = new TerrainLOD(info);
		terrain->BaseTexture(L"[2K]PavingStones36/PavingStones36_col.jpg");
		terrain->PBRTextures(L"[2K]PavingStones36/PavingStones36_nrm.jpg", L"[2K]PavingStones36/PavingStones36_rgh.jpg",
			L"[2K]PavingStones36/PavingStones36_disp.jpg");

		//terrain->BaseTexture(L"Road/Four_lane_road_wet_01_1K_Base_Color.png");
		//terrain->PBRTextures(L"Road/Four_lane_road_wet_01_1K_Normal.png", L"Road/Four_lane_road_wet_01_1K_Roughness.png",
			//L"Road/Four_lane_road_wet_01_1K_Mask.png");
		/*terrain->BaseTexture(L"Terrain/Ground13/Ground13_col.jpg");
		terrain->PBRTextures(L"Terrain/Ground13/Ground13_nrm.jpg", L"Terrain/Ground13/CamoFabric_Roughness.png",
			L"Terrain/Ground13/Ground13_disp.jpg");*/

			/*terrain->BaseTexture(L"[2K]Bricks28/Bricks28_col.jpg");
			terrain->PBRTextures(L"[2K]Bricks28/Bricks28_nrm.jpg", L"[2K]Bricks28/Bricks28_rgh.jpg",
				L"[2K]Bricks28/Bricks28_disp.jpg");*/


	//			oceanRenderer = new OceanRenderer(nullptr,Vector3(0,0,2));

	}




	//Cube Map
	{

		preintegratedFG = new Texture(L"PBR/PreintegratedFG.bmp");
		sBRDF = deferredShader->AsSRV("PreintegratedFG");

		wstring temp = L"../../_Textures/Environment/SunsetCube1024.dds";
		//wstring temp = L"../../_Textures/Environment/sky_ocean.dds";
		D3DX11CreateShaderResourceViewFromFile
		(
			D3D::GetDevice(), temp.c_str(), NULL, NULL, &skyIRSRV, NULL
		);
		sSkyMap = deferredShader->AsSRV("skyIR");
		sSkyMap->SetResource(skyIRSRV);
		//sPreviewSkyMap = deferredShader->AsSRV("PreviewskyIR");
		//sPreviewSkyMap->SetResource(skyIRSRV);
		sBRDF->SetResource(preintegratedFG->SRV());

	}

	cubeMap = new TextureCube(Vector3(0, 0, 0), 256, 256);
	//perFrame = new PerFrame(deferredShader);

	buttonTextures[0] = new Texture(L"playButton.png");
	buttonTextures[1] = new Texture(L"pauseButton.png");
	buttonTextures[2] = new Texture(L"stopButton.png");
	behaviorTreeTexture = new Texture(L"BehaviorTree.jpg");
	bStart = false;
	bPause = false;

	progress = new ProgressBar();



	D3D11_QUERY_DESC queryDesc;
	queryDesc.Query = D3D11_QUERY_OCCLUSION_PREDICATE;
	queryDesc.MiscFlags = D3D11_QUERY_MISC_PREDICATEHINT;
	//ID3D11Query * pQuery;

	D3D::GetDevice()->CreatePredicate(&queryDesc, &predicate);

	//UINT64 queryData; // This data type is different depending on the query type
	/*while (S_OK != context->GetData(pQuery, &queryData, sizeof(UINT64), 0))
	{
	}*/
}



void EditorMain::Destroy()
{
	SafeDelete(cloud);
	GBuffer.Destroy();
}

void EditorMain::Update()
{

	//perFrame->Update();
	//renderTarget
	//{
	//	diffuse->Update();
	//	normal->Update();
	//	specular->Update();
	//	bump->Update();
	//}


	//Environment
	{
		sky->Update();
		cloud->Update();
		island11->Update();
		terrain->Update();
	
		//fog->Update();

	}

	if (!bStart&&terrain->Intersection(pos))
	{
		AddTrasform(pos);
		lights->Update(pos);
	}

	for (uint i = 0; i < MAX_OBJECT_COUNT; i++)
	{
		sphere[i]->Update();
		cylinder[i]->Update();
	}
	for (uint i = 0; i < 8; i++)
		materialSphere[i]->Update();


	cube->Update();


	for_each(actors.begin(), actors.end(), [](Actor* actor) {actor->Update(); });
	for_each(actors.begin(), actors.end(), [](Actor* actor) {actor->ActorPos(); });
	






	GBuffer.Update();
	dirLight->Update();

	/*QueryPerformanceCounter((LARGE_INTEGER *)&currentTime);
		if (chrono::_Is_even<uint>(currentTime))*/



	lights->Render();



	//ImGui::SliderFloat("Point Intensity", &pointLight->GetPointLight(0).Intensity, 0.01f, 10.0f);
}

///////////////////////////////////////////////////////////////////////////////////////////////
void EditorMain::Depth_PreRender()
{
	meshPass = 0;
	modelPass = 1;
	animPass = 2;


	dirLight->SetShadowMap();
	//terrain->Pass(17);
	//terrain->Render();
	//cout << Time::Get()->FrameCount() << endl;
	for (auto& actor : actors)
	{
		actor->Pass(0);
		actor->Render();
	}

	/*for (auto& actor : actors)
			actor->Render(1);*/



	for (UINT i = 0; i < MAX_OBJECT_COUNT; i++)
	{
		sphere[i]->Pass(meshPass);
		sphere[i]->Render();
	}

	for (UINT i = 0; i < MAX_OBJECT_COUNT; i++)
	{
		cylinder[i]->Pass(meshPass);
		cylinder[i]->Render();
	}


	cube->Pass(meshPass);
	cube->Render();


	
}

void EditorMain::Reflection_PreRender()
{
	//meshPass = 5;
	//modelPass = 6;
	//animPass = 7;
	//terrainPass = 8;

	////water->SetReflection();
	////mirror->SetReflection();
	//sky->Pass(3);
	//sky->Render();
	//terrain->Pass(terrainPass);
	//terrain->Render();
	//

	//for (uint i = 0; i < 8; i++)
	//{
	//	mat[i]->Render();
	//	materialSphere[i]->Pass(meshPass);

	//	materialSphere[i]->Render();
	//}

	//wall->Render();
	//for (UINT i = 0; i < MAX_OBJECT_COUNT; i++)
	//{

	//	sphere[i]->Pass(meshPass);
	//	sphere[i]->Render();
	//}
	//brick->Render();
	//for (UINT i = 0; i < MAX_OBJECT_COUNT; i++)
	//{
	//	cylinder[i]->Pass(meshPass);
	//	cylinder[i]->Render();
	//}

	//stone->Render();
	//cube->Pass(meshPass);
	//cube->Render();

	for (UINT i = 0; i < MAX_OBJECT_COUNT; i++)
	{
		sphere[i]->Pass(27);
		sphere[i]->Render();
	}

	for (UINT i = 0; i < MAX_OBJECT_COUNT; i++)
	{
		cylinder[i]->Pass(27);
		cylinder[i]->Render();
	}


	for (auto& actor : actors)
	{
		actor->Pass(24);
		actor->Render();
	}

}


void EditorMain::GbufferPacking()
{

	meshPass = 5;
	terrainPass = 8;

	//water->SetReflection();
	//mirror->SetReflection();
	
	//
	/*terrain->Pass(terrainPass);
	terrain->Render();*/
	//

	for (uint i = 0; i < 8; i++)
	{
		mat[i]->Render();
		materialSphere[i]->Pass(meshPass);

		materialSphere[i]->Render();
	}

	wall->Render();
	for (UINT i = 0; i < MAX_OBJECT_COUNT; i++)
	{

		sphere[i]->Pass(meshPass);
		sphere[i]->Render();
	}
	brick->Render();
	for (UINT i = 0; i < MAX_OBJECT_COUNT; i++)
	{
		cylinder[i]->Pass(meshPass);
		cylinder[i]->Render();
	}

	stone->Render();
	cube->Pass(meshPass);
	cube->Render();
	island11->Terrain();

	

	for (auto& actor : actors)
	{
		actor->Pass(1);
		actor->Render();
	}
	
}


void EditorMain::GbufferUnPacking()
{
	
	{
		//static bool onfFog = false;
	//if (Keyboard::Get()->Down('F'))
	//{
		//onfFog = onfFog ? false : true;
	//}
	//if (onfFog)
	//{
		//fog->Render();

     //	}

	}
	GBuffer.Render(ssao->GetMiniDepthSRV());
    	
	
	dirLight->Render();
	pointLight->Render();
	spotLight->Render();
	
}


void EditorMain::Render()
{
	mainDSV = GBuffer.DepthstencilDSV();
	depthSRV = GBuffer.DepthstencilSRV();
	//Preview Render
	{
		D3D::GetDC()->OMGetRenderTargets(1, &old_target, &old_depth);
		UINT num = 1;
		D3D::GetDC()->RSGetViewports(&num, &oldvp);


		progress->Begin();
		progress->Render();

		MenuBar();
		Assets();
		ToolBar();
		if (!bStart)
		{
			sslr->ImGui();
			sky->ImGui();
			cloud->ImGui();
			hdr->ImGui();

		}
		for_each(actors.begin(), actors.end(), [](Actor* actor) {actor->PrevRender(); });
		EffectSystem::Get()->PreviewRender();
	}
	
	
	//Depth and Reflection
	{
		
		Depth_PreRender();
		island11->Reflection();
		Reflection_PreRender();
    	Context::Get()->PushViewMatrix();
	}
	//Packing
	{
		D3D::GetDC()->RSSetViewports(1, &oldvp);
		ID3D11RenderTargetView* rt[3] = { GBuffer.DiffuseRTV(),  GBuffer.SpecularRTV(),
			 GBuffer.NormalRTV() };
		D3D::GetDC()->ClearDepthStencilView(mainDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
		for (uint i = 0; i < 3; i++)
		{
			D3D::GetDC()->ClearRenderTargetView(rt[i], ClearColor);
		}
		D3D::GetDC()->OMSetRenderTargets(3, rt, mainDSV);
		GbufferPacking();
		
	}

	{//SSAO
		
	}
	
	//Unpacking
	{
	//	D3D::GetDC()->Begin(predicate);
		D3D::GetDC()->OMSetRenderTargets(1, &g_HDRRTV, GBuffer.DepthstencilDSVReadOnly());
		D3D::GetDC()->ClearRenderTargetView(g_HDRRTV, ClearColor);
		
		ssao->Compute(depthSRV, GBuffer.NormalSRV());
		GbufferUnPacking();
		//D3D::GetDC()->SetPredication(predicate, true);
		//D3D::GetDC()->End(predicate);
		
	}
	{//SSLR
		//D3D::GetDC()->SetPredication(predicate, false);
		sslr->Render(ssao->GetMiniDepthSRV(), g_HDRRTV);
	}
	


	{
		island11->Refraction(depthSRV, GBuffer.DiffuseTexture());
	}


	{//Blending
		D3D::GetDC()->RSSetViewports(1, &oldvp);
		D3D::GetDC()->OMSetRenderTargets(1, &g_HDRRTV, GBuffer.DepthstencilDSVReadOnly());
		D3D::GetDC()->ClearDepthStencilView(GBuffer.DepthstencilDSVReadOnly(), D3D11_CLEAR_DEPTH, 1.0, 0);

		
		for_each(actors.begin(), actors.end(), [](Actor* actor) {actor->ForwardRender(); });
		
		//oceanRenderer->SetCubeMap(cubeMap->SRV());
		//oceanRenderer->renderShaded();
		{
			sky->Pass(3);
			sky->Render();
			cloud->Render();
		}
		island11->Water();
		EffectSystem::Get()->Render();

		//D3D::GetDC()->SetPredication(nullptr, false);
	}


	{//PostEfect
		hdr->PostProcessing(g_HDRSRV, old_target, depthSRV);
		D3D::GetDC()->OMSetRenderTargets(1, &old_target, mainDSV);
		//D3D::GetDC()->SetPredication(predicate, false);
	}

	{
		//diffuse->SRV(dirLight->SRV());
		//diffuse->SRV(ocean->getD3D11DisplacementMap());
		//diffuse->SRV(diffuseTarget->SRV());
		//diffuse->Render();

		//normal->SRV(ocean->getD3D11DisplacementMap());
		//normal->Render();

		//specular->SRV(specularTarget->SRV());
		//specular->Render();

		//bump->SRV(reflection->GetSRV());
		//bump->SRV(ssao->GetSSAOSRV());
		//bump->Render();
	}

	

}






void EditorMain::Assets()
{
	if (bStart) return;
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;


	ImGui::Begin("Node", 0, flags);
	{
		if (bCreatedBehaviorTree)
			if (ImGui::ImageButton(behaviorTreeTexture->SRV(), ImVec2(80, 80)))
			{
				bShowBehaviorTree = true;
			}
		if (bShowBehaviorTree)
		{
			Application_Frame(&bShowBehaviorTree);
		}

		if (ImGui::IsWindowHovered())
		{
			if (ImGui::GetIO().MouseDown[1])
				ImGui::OpenPopup("Popup");
		}
		ShowPopUp(false);

	}
	ImGui::End();
	ImGui::Begin("Assets", 0, flags);
	{

		for (auto& actor : actors)
		{
			actor->ImageButton();
			ImGui::SameLine();
		}
		if (ImGui::IsWindowHovered())
		{
			if (ImGui::GetIO().MouseDown[1])
				ImGui::OpenPopup("Popup");
		}
		ShowPopUp(true);


	}
	ImGui::End();


}

void EditorMain::ShowPopUp(bool bAsset)
{
	if (ImGui::BeginPopup("Popup"))
	{
		if (ImGui::MenuItem("Copy")) {}
		if (ImGui::MenuItem("Delete"))
		{
			DeleteActor();
		}

		ImGui::Separator();

		if (bAsset)
		{
			if (ImGui::MenuItem("Add Actor"))
			{
				Actor* actor = new Actor(deferredShader);

				actor->SetIndex(actorIndex);
				actorIndex++;
				actor->SetTerrain(terrain);

				actors.emplace_back(actor);

			}
		}
		else
		{
			if (ImGui::MenuItem("Add BehaviorTree"))
			{



				bCreatedBehaviorTree = true;


			}
		}



		ImGui::EndPopup();
	}
}

void EditorMain::AddTrasform(const Vector3& pos)
{
	//for_each(actors.begin(), actors.end(), [&pos](Actor* actor) {actor->AddTransform(pos); });
	if (bStart) return;

	auto darggedActor = std::find_if(actors.begin(), actors.end(),
		[](Actor* actor) {return actor->IsDragged() == true; });
	if (darggedActor != actors.end())
	{

		auto actor = *darggedActor;


		auto selectT = new Matrix(actor->PreviewTreansform()->World());

		selectT->_41 = pos.x;
		selectT->_42 = pos.y;
		selectT->_43 = pos.z;
		//transforms.emplace_back(selectT);
		//selectT->Set(actor->PreviewTreansform());
		//selectT->Scale(0.04f, 0.04f, 0.04f);
		//selectT->Position(pos);



		actor->AddTransform(*selectT);
		actor->SetIsDragged(false);
		SafeDelete(selectT);


	}



}


void EditorMain::CreateHDRRTV(uint width, uint height)
{
	SafeRelease(g_pHDRTexture);
	SafeRelease(g_HDRRTV);
	SafeRelease(g_HDRSRV);

	// Create the HDR render target
	D3D11_TEXTURE2D_DESC dtd = {
		width, //UINT Width;
		height, //UINT Height;
		1, //UINT MipLevels;
		1, //UINT ArraySize;
		DXGI_FORMAT_R16G16B16A16_TYPELESS, //DXGI_FORMAT Format;
		1, //DXGI_SAMPLE_DESC SampleDesc;
		0,
		D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
		0,//UINT CPUAccessFlags;
		0//UINT MiscFlags;    
	};
	Check(D3D::GetDevice()->CreateTexture2D(&dtd, NULL, &g_pHDRTexture));


	D3D11_RENDER_TARGET_VIEW_DESC rtsvd =
	{
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		D3D11_RTV_DIMENSION_TEXTURE2D
	};
	Check(D3D::GetDevice()->CreateRenderTargetView(g_pHDRTexture, &rtsvd, &g_HDRRTV));


	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd =
	{
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		0,
		0
	};
	dsrvd.Texture2D.MipLevels = 1;
	Check(D3D::GetDevice()->CreateShaderResourceView(g_pHDRTexture, &dsrvd, &g_HDRSRV));



}

void EditorMain::CreateRender2DTarget(uint width, uint height)
{



	diffuse = new Render2D();
	normal = new Render2D();
	specular = new Render2D();
	bump = new Render2D();



	diffuse->GetTransform()->Scale(150, 150, 1);
	diffuse->GetTransform()->Position(75, height - 720 + 75, 0);
	normal->GetTransform()->Scale(150, 150, 1);
	normal->GetTransform()->Position(225, height - 720 + 75, 0);
	specular->GetTransform()->Scale(150, 150, 1);
	specular->GetTransform()->Position(225 + 150, height - 720 + 75, 0);
	bump->GetTransform()->Scale(150, 150, 1);
	bump->GetTransform()->Position(375 + 150, height - 720 + 75, 0);


}

void EditorMain::SetMaterial()
{
	Color spec = Color(0.0f, 0.0f, 0.0f, 1);
	mat[0]->DiffuseMap("PBRTextures/GunMetal_Albedo.png");
	mat[0]->NormalMap("PBRTextures/GunMetal_Normal.png");
	mat[0]->RoughnessMap("PBRTextures/GunMetal_Roughness.png");
	mat[0]->MatallicMap("PBRTextures/GunMetal_Metallic.png");
	mat[0]->Specular(spec);

	mat[1]->DiffuseMap("PBRTextures/AluminiumInsulator_Albedo.png");
	mat[1]->NormalMap("PBRTextures/AluminiumInsulator_Normal.png");
	mat[1]->RoughnessMap("PBRTextures/AluminiumInsulator_Roughness.png");
	mat[1]->MatallicMap("PBRTextures/AluminiumInsulator_Metallic.png");
	mat[1]->Specular(spec);
	mat[2]->DiffuseMap("PBRTextures/CamoFabric_Albedo.png");
	mat[2]->NormalMap("PBRTextures/CamoFabric_Normal.png");
	mat[2]->RoughnessMap("PBRTextures/CamoFabric_Roughness.png");
	mat[2]->MatallicMap("PBRTextures/CamoFabric_Metallic.png");
	mat[2]->Specular(spec);
	mat[3]->DiffuseMap("PBRTextures/GlassVisor_Albedo.png");
	mat[3]->NormalMap("PBRTextures/GlassVisor_Normal.png");
	mat[3]->RoughnessMap("PBRTextures/GlassVisor_Roughness.png");
	mat[3]->MatallicMap("PBRTextures/GlassVisor_Metallic.png");
	mat[3]->Diffuse(1, 1, 1, 20);
	mat[3]->Specular(spec);
	//mat[4]->DiffuseMap("PBRTextures/Gold_Albedo.png");
	//mat[4]->NormalMap("PBRTextures/Gold_Normal.png");
	//mat[4]->RoughnessMap("PBRTextures/Gold_Roughness.png");
	//mat[4]->MatallicMap("PBRTextures/Gold_Metallic.png");

	mat[4]->DiffuseMap("[4K]Metal07/Metal07_col.jpg");
	mat[4]->NormalMap("[4K]Metal07/Metal07_nrm.jpg");
	mat[4]->RoughnessMap("[4K]Metal07/Metal07_rgh.jpg");
	mat[4]->MatallicMap("[4K]Metal07/Metal07_met.jpg");
	mat[4]->Specular(spec);

	mat[5]->DiffuseMap("PBRTextures/IronOld_Albedo.png");
	mat[5]->NormalMap("PBRTextures/IronOld_Normal.png");
	mat[5]->RoughnessMap("PBRTextures/IronOld_Roughness.png");
	mat[5]->MatallicMap("PBRTextures/IronOld_Metallic.png");
	mat[5]->Specular(spec);

	mat[6]->DiffuseMap("PBRTextures/Leather_Albedo.png");
	mat[6]->NormalMap("PBRTextures/Leather_Normal.png");
	mat[6]->RoughnessMap("PBRTextures/Leather_Roughness.png");
	mat[6]->MatallicMap("PBRTextures/Leather_Metallic.png");
	mat[6]->Specular(spec);

	mat[7]->DiffuseMap("PBRTextures/Rubber_Albedo.png");
	mat[7]->NormalMap("PBRTextures/Rubber_Normal.png");
	mat[7]->RoughnessMap("PBRTextures/Rubber_Roughness.png");
	mat[7]->MatallicMap("PBRTextures/Rubber_Metallic.png");
	mat[7]->Specular(spec);



	////////////////////////////////////////////////////////////////////////////////
	stone = new Material(deferredShader);
	stone->DiffuseMap("PBRTextures/SuperHeroFabric_Albedo.png");
	stone->NormalMap("PBRTextures/SuperHeroFabric_Normal.png");
	stone->RoughnessMap("PBRTextures/SuperHeroFabric_Roughness.png");
	stone->MatallicMap("PBRTextures/SuperHeroFabric_Metallic.png");
	stone->Specular(spec);

	brick = new Material(deferredShader);
	//brick->DiffuseMap("[4K]Metal07/Metal07_col.jpg");
	//brick->NormalMap("[4K]Metal07/Metal07_nrm.jpg");
	//brick->RoughnessMap("[4K]Metal07/Metal07_rgh.jpg");
	//brick->MatallicMap("[4K]Metal07/Metal07_met.jpg");

	//brick->DiffuseMap("PBRTextures/Gold_Albedo.png");
	//brick->NormalMap("PBRTextures/Gold_Normal.png");
	//brick->RoughnessMap("PBRTextures/Gold_Roughness.png");
	//brick->MatallicMap("PBRTextures/Gold_Metallic.png");

	//brick->DiffuseMap("PBRTextures/SuperHeroFabric_Albedo.png");
	//brick->NormalMap("PBRTextures/SuperHeroFabric_Normal.png");
	//brick->RoughnessMap("PBRTextures/SuperHeroFabric_Roughness.png");
	//brick->MatallicMap("PBRTextures/SuperHeroFabric_Metallic.png");

	//brick->DiffuseMap("PBRTextures/Wood_Albedo.png");
	//brick->NormalMap("PBRTextures/Wood_Normal.png");
	//brick->RoughnessMap("PBRTextures/Wood_Roughness.png");
	//brick->MatallicMap("PBRTextures/Wood_Metallic.png");
	brick->DiffuseMap("[2K]Bricks28/Bricks28_col.jpg");
	brick->NormalMap("[2K]Bricks28/Bricks28_nrm.jpg");
	brick->RoughnessMap("[2K]Bricks28/Bricks28_rgh.jpg");
	brick->Specular(spec);
	//brick->Specular(0.8f, 0.8f, 0.8f, 1);
	//brick->MatallicMap("[2K]Bricks28/Wood_Metallic.png");

	//brick->Diffuse(1, 1, 1, 1);
	//brick->Specular(1, 0.3f, 0.3f, 20);

	wall = new Material(deferredShader);

	//wall->DiffuseMap("[4K]Metal07/Metal07_col.jpg");
	//wall->NormalMap("[4K]Metal07/Metal07_nrm.jpg");
	//wall->RoughnessMap("[4K]Metal07/Metal07_rgh.jpg");
	//wall->MatallicMap("[4K]Metal07/Metal07_met.jpg");

	//wall->DiffuseMap("PBRTextures/SuperHeroFabric_Albedo.png");
	//wall->NormalMap("PBRTextures/SuperHeroFabric_Normal.png");
	//wall->RoughnessMap("PBRTextures/SuperHeroFabric_Roughness.png");
	//wall->MatallicMap("PBRTextures/SuperHeroFabric_Metallic.png");

	wall->DiffuseMap("[2K]Bricks28/Bricks28_col.jpg");
	wall->NormalMap("[2K]Bricks28/Bricks28_nrm.jpg");
	wall->RoughnessMap("[2K]Bricks28/Bricks28_rgh.jpg");
	wall->Specular(spec);
	//wall->Specular(0.8f, 0.8f, 0.8f, 1);
	//wall->Specular(0.3f, 0.3f, 0.3f, 20);
	//wall->DiffuseMap("PBRTextures/Wood_Albedo.png");
	//wall->NormalMap("PBRTextures/Wood_Normal.png");
	//wall->RoughnessMap("PBRTextures/Wood_Roughness.png");
	//wall->MatallicMap("PBRTextures/Wood_Metallic.png");

	//wall->DiffuseMap("PBRTextures/Gold_Albedo.png");
	//wall->NormalMap("PBRTextures/Gold_Normal.png");
	//wall->RoughnessMap("PBRTextures/Gold_Roughness.png");
	//wall->MatallicMap("PBRTextures/Gold_Metallic.png");

	/*wall->DiffuseMap("[4K]Metal07/Metal07_col.jpg");
	wall->NormalMap("[4K]Metal07/Metal07_nrm.jpg");
	wall->RoughnessMap("[4K]Metal07/Metal07_rgh.jpg");
	wall->MatallicMap("[4K]Metal07/Metal07_met.jpg");*/


}

void EditorMain::CreateObjects()
{
	///////////////////////////////////////////////////////////////////////////
	for (uint i = 0; i < 4; i++)
	{
		materialSphere[i] = new MeshSphere(deferredShader, 0.5f, 20, 20);
		materialSphere[i]->GetTransform()->Position(i * 10, 50, 0);
		materialSphere[i]->GetTransform()->Scale(10, 10, 10);
	}

	for (uint i = 4; i < 8; i++)
	{
		materialSphere[i] = new MeshSphere(deferredShader, 0.5f, 20, 20);
		materialSphere[i]->GetTransform()->Position((i - 4) * 10, 40, 0);
		materialSphere[i]->GetTransform()->Scale(10, 10, 10);
	}

	//CreateMesh
	{

		for (uint i = 0; i < 8; i++)
			mat[i] = new Material(deferredShader);

		SetMaterial();




		cube = new MeshCube(deferredShader);
		cube->GetTransform()->Position(0 ,0.5f, 0);
		cube->GetTransform()->Scale(20.0f, 15.0f, 25.0);


		for (UINT i = 0; i < 5; i++)
		{
			cylinder[i * 2] = new MeshCylinder(deferredShader, 0.5f, 3.0f, 20, 20);
			cylinder[i * 2]->GetTransform()->Position(-30, 8.0f, -15.0f + (float)i * 15.0f);
			cylinder[i * 2]->GetTransform()->Scale(5, 5, 5);

			cylinder[i * 2 + 1] = new MeshCylinder(deferredShader, 0.5f, 3.0f, 20, 20);
			cylinder[i * 2 + 1]->GetTransform()->Position(30, 8.0f, -15.0f + (float)i * 15.0f);
			cylinder[i * 2 + 1]->GetTransform()->Scale(5, 5, 5);


			sphere[i * 2] = new MeshSphere(deferredShader, 0.5f, 20, 20);
			sphere[i * 2]->GetTransform()->Position(-30.0f, 17.5f, -15.0f + i * 15.0f);
			sphere[i * 2]->GetTransform()->Scale(5, 5, 5);

			sphere[i * 2 + 1] = new MeshSphere(deferredShader, 0.5f, 20, 20);
			sphere[i * 2 + 1]->GetTransform()->Position(30.0f, 17.5f, -15.0f + i * 15.0f);
			sphere[i * 2 + 1]->GetTransform()->Scale(5, 5, 5);
		}
	}
	cylinder[10] = new MeshCylinder(deferredShader, 0.5f, 3.0f, 20, 20);
	cylinder[10]->GetTransform()->Position(240, 6.0f, 240);
	cylinder[10]->GetTransform()->Scale(5, 5, 5);

	sphere[10] = new MeshSphere(deferredShader, 0.5f, 20, 20);
	sphere[10]->GetTransform()->Position(240, 15.5f, 240);
	sphere[10]->GetTransform()->Scale(5, 5, 5);

	cylinder[11] = new MeshCylinder(deferredShader, 0.5f, 3.0f, 20, 20);
	cylinder[11]->GetTransform()->Position(-240, 6.0f, 240);
	cylinder[11]->GetTransform()->Scale(5, 5, 5);

	sphere[11] = new MeshSphere(deferredShader, 0.5f, 20, 20);
	sphere[11]->GetTransform()->Position(-240, 15.5f, 240);
	sphere[11]->GetTransform()->Scale(5, 5, 5);

	cylinder[12] = new MeshCylinder(deferredShader, 0.5f, 3.0f, 20, 20);
	cylinder[12]->GetTransform()->Position(-240, 6.0f, -240);
	cylinder[12]->GetTransform()->Scale(5, 5, 5);

	sphere[12] = new MeshSphere(deferredShader, 0.5f, 20, 20);
	sphere[12]->GetTransform()->Position(-240, 15.5f, -240);
	sphere[12]->GetTransform()->Scale(5, 5, 5);

	cylinder[13] = new MeshCylinder(deferredShader, 0.5f, 3.0f, 20, 20);
	cylinder[13]->GetTransform()->Position(240, 6.0f, -240);
	cylinder[13]->GetTransform()->Scale(5, 5, 5);

	sphere[13] = new MeshSphere(deferredShader, 0.5f, 20, 20);
	sphere[13]->GetTransform()->Position(240, 15.5f, -240);
	sphere[13]->GetTransform()->Scale(5, 5, 5);


	lights = new LightObjects(pointLight, spotLight);


}



void EditorMain::MenuBar()
{


	if (bStart) return;
	if (ImGui::BeginMainMenuBar())
	{
		//File Save & Load
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Load"))
			{
				HWND hWnd = NULL;
				function<void(wstring)> f = bind(&EditorMain::Load, this, placeholders::_1);
				Path::OpenFileDialog(L"", Path::LevelFilter, L"../../_Levels/", f, hWnd);
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Save")) {}
			if (ImGui::MenuItem("Save as..."))
			{
				HWND hWnd = NULL;
				function<void(wstring)> f = bind(&EditorMain::Save, this, placeholders::_1);
				Path::SaveFileDialog(L"", Path::LevelFilter, L"../../_Levels/", f, hWnd);
			}

			ImGui::EndMenu();
		}

		//Tools
		if (ImGui::BeginMenu("Tools"))
		{
			ImGui::MenuItem("Metrics", nullptr, &bShowMetricsWindow);
			ImGui::MenuItem("Style", nullptr, &bShowStyleEditor);
			ImGui::MenuItem("Demo", nullptr, &bShowDemoWindow);
			ImGui::MenuItem("Script", nullptr, &bShowScriptTool);

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}


}

void EditorMain::ToolBar()
{
	//ImGui::Begin("Tool Bar", nullptr,ImGuiWindowFlags_NoTitleBar);
	{
		if (ImGui::ImageButton(buttonTextures[0]->SRV(), ImVec2(20.0f, 20.0f)))//play
		{
			Gui::Get()->AddTransform(nullptr);
			for (auto& actor : actors)
			{
				actor->Start();
			}
			bStart = true;
			terrain->SetStart(bStart);
			lights->SetStart(bStart);

			ColliderSystem::Get()->SetStart(bStart);
			EffectSystem::Get()->SetStart(!bStart);

		}
		ImGui::SameLine();
		//if (ImGui::ImageButton(buttonTextures[1]->SRV(), ImVec2(20.0f, 20.0f)))//pause
		//{

		//	bPause = true;
		//	bStart = false;
		//}
		//ImGui::SameLine();
		if (ImGui::ImageButton(buttonTextures[2]->SRV(), ImVec2(20.0f, 20.0f)))//stop
		{
			for (auto& actor : actors)
			{
				actor->Stop();
			}
			bStart = false;
			terrain->SetStart(bStart);
			lights->SetStart(bStart);
			ColliderSystem::Get()->SetStart(bStart);
			EffectSystem::Get()->SetStart(!bStart);

		}
	}
	//ImGui::End();
}

void EditorMain::DeleteActor()
{
	/*for (auto iter = actors.begin(); iter != actors.end();)
	{
		auto previewMesh = *iter;
		if (previewMesh->BoneIndex() == bone->BoneIndex())
		{

			iter = previewMeshes.erase(iter);

		}
		else
			iter++;
	}*/
}

void EditorMain::Save(wstring fileName)
{

	auto ext = ".Level";
	string sname = String::ToString(fileName);
	if (String::Contain(String::ToString(fileName), ext))
	{
		auto lastIndex = fileName.find_last_of('.');
		sname = String::ToString(fileName).substr(0, lastIndex);
	}

	EventSystem::Get()->CreateWriter(String::ToWString(sname + ext));
	//EventSystem::Get()->CreateDoc();


	if (bCreatedBehaviorTree)
	{
		EventSystem::Get()->Writer()->Bool(bCreatedBehaviorTree);
		//EventSystem::Get()->CreateNode("BehaviorTree");
		//SaveAllNodes();
	}

	if (actors.empty())
	{
		EventSystem::Get()->Writer()->UInt(0);
	}
	else
	{

		EventSystem::Get()->Writer()->UInt(actors.size());
		for (uint i = 0; i < actors.size(); i++)
		{
			/*	string name = "Actor" + to_string(index);
				EventSystem::Get()->CreateNode(name);*/

			actors[i]->Save();


		}
	}


	EventSystem::Get()->CloseWriter();
	//EventSystem::Get()->MakeFile(String::ToString(fileName));



}

void EditorMain::Load(wstring fileName)
{

	this->fileName = fileName;

	Thread::Get()->AddTask([&]()
	{

		EventSystem::Get()->CreateReader(this->fileName);

		bCreatedBehaviorTree = EventSystem::Get()->Reader()->Bool();
		if (bCreatedBehaviorTree)
			LoadAllNodes();


		uint actorCount = EventSystem::Get()->Reader()->UInt();
		ProgressReport::Get().SetJobCount(ProgressReport::Model, (200000 * actorCount) + (450000 * actorCount) + 350009 + (actorCount - 1));
		ProgressReport::Get().IncrementJobsDone(ProgressReport::Model);

		for (uint i = 0; i < 100000; i++)
		{
			ProgressReport::Get().IncrementJobsDone(ProgressReport::Model);
		}
		if (actorCount > 0)
		{
			for (uint i = 0; i < actorCount; i++)
			{
				Actor* actor = new Actor(deferredShader);
				ProgressReport::Get().IncrementJobsDone(ProgressReport::Model);
				for (uint i = 0; i < 200000; i++)
				{
					ProgressReport::Get().IncrementJobsDone(ProgressReport::Model);
				}
				actor->SetIndex(i);
				actorIndex++;
				actor->SetTerrain(terrain);
				actor->Load();
				for (uint i = 0; i < 450000; i++)
				{
					ProgressReport::Get().IncrementJobsDone(ProgressReport::Model);
				}
				actors.emplace_back(actor);
			}

		}




		EventSystem::Get()->CloseReader();
		ProgressReport::Get().IncrementJobsDone(ProgressReport::Model);

		for (uint i = 0; i < 250000; i++)
		{
			ProgressReport::Get().IncrementJobsDone(ProgressReport::Model);
		}
		for (auto& actor : actors)
		{
			actor->LoadComplete();
		}
	});



}
