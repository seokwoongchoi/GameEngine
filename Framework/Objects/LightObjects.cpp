#include "Framework.h"
#include "LightObjects.h"



LightObjects::LightObjects(DeferredPointLight * pointLight, DeferredSpotLight * spotLight)
{
	
	lightType = LightType::unKnown;
	this->pointLight = pointLight;
	this->spotLight = spotLight;
	pointIndex = -1;
	spotIndex = -1;
	capsuleIndex = -1;
	bDrag = false;
	
	pointTexture = new Texture(L"LightIcons/PointLight.png");//
	spotTexture = new Texture(L"LightIcons/SpotLight.png");//
	capsuleTexture = new Texture(L"LightIcons/CapsuleLight.png");//

	D3DXMatrixIdentity(&world);

	


}

LightObjects::~LightObjects()
{
}

void LightObjects::Update(Vector3& mousePos)
{
	
	if (Mouse::Get()->Up(0)&& bDrag)
		{
			switch (lightType)
			{
			case LightType::unKnown:
			{
			
			}
				
				break;
			case LightType::pointLight:
			{
				float pointRange = 50;
				Vector3 pointPosition = Vector3(mousePos.x, mousePos.y + 10.0f, mousePos.z);
				
			
				PointLight light;
				light =
				{
					Color(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
					Color(1.0f, 1.0f, 1.0f, 1.0f), //specular
					pointPosition, pointRange,1.50f
				};
				pointLight->AddPointLight(light);
				
				//modelRender->GetTransform(0)->Position(-15, 0.0f, 0);
				
				
			}
			break;
			case LightType::spotLight:
			{
				float spotRange = 60;
				float fInnerAngle=45;
				float fOuterAngle=45;
				
				Vector3 spotPosition = Vector3(mousePos.x, mousePos.y + spotRange, mousePos.z);
				Vector3 dir = Vector3(0, -1, 0);
				SpotLight light;
				light =
				{
					Color(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
					Color(1.0f, 1.0f, 1.0f, 1.0f), //specular
					spotPosition,dir, spotRange,5.00f,fInnerAngle,fOuterAngle
				};
				spotLight->AddSpotLight(light);
			}
				
			break;
			/*case LightType::capsuleLight:
				capsuleLight.Position = Vector3(mousePos.x, mousePos.y + 3.0f, mousePos.z);

				Context::Get()->AddCapsuleLight(capsuleLight);
				break;*/

			}

			bDrag = false;
	}
	
	

	

	if (Mouse::Get()->Down(0) && bDrag == false)
	{
		if (pointLight->GetPointLightCount() > 0)
			for (uint i = 0; i < pointLight->GetPointLightCount(); i++)
			{
				Vector3 position = pointLight->GetPointLight(i).Position;
				
				if (CheckIsPicked(Vector3(position.x, position.y+(pointLight->GetPointLight(i).Range*0.5f)
					, position.z), Mouse::Get()->GetPosition(), LightType::pointLight))
				{
					
					
					pointIndex = i;
				}
			}

		if (spotLight->GetSpotLightCount() > 0)
			for (uint i = 0; i < spotLight->GetSpotLightCount(); i++)
			{
				if (CheckIsPicked(spotLight->GetSpotLight(i).Position, Mouse::Get()->GetPosition(), LightType::spotLight))
				{

					spotIndex = i;
				}
			}

		//if (Context::Get()->GetCapsuleLightCount() > 0)
		//	for (uint i = 0; i < Context::Get()->GetCapsuleLightCount(); i++)
		//	{
		//		if (CheckIsPicked(Context::Get()->GetCapsuleLight(i).Position, Mouse::Get()->GetPosition(), LightType::capsuleLight))
		//		{

		//			capsuleIndex = i;
		//		}
		//	}
	}

	
}

void LightObjects::Render()
{
	if (bStart) return;
	ImGui::Begin("Light", 0, ImGuiWindowFlags_NoMove);
	//	if (!ImGui::IsAnyItemHovered())
	{
		ImGui::ImageButton(pointTexture->SRV(), ImVec2(50.0f, 50.0f));
		SetDragDropPayload(LightType::pointLight, String::ToString(pointTexture->GetFile()));
		ImGui::SameLine();
		ImGui::ImageButton(spotTexture->SRV(), ImVec2(50.0f, 50.0f));
		SetDragDropPayload(LightType::spotLight, String::ToString(spotTexture->GetFile()));
		ImGui::SameLine();
		ImGui::ImageButton(capsuleTexture->SRV(), ImVec2(50.0f, 50.0f));
		SetDragDropPayload(LightType::capsuleLight, String::ToString(capsuleTexture->GetFile()));
	}


		PointLightControll(pointIndex);
		SpotLightControll(spotIndex);
		
	
	
	//CapsuleLightControll(capsuleIndex);


	ImGui::End();
	PointLightRender();
	SpotLightRender();
	//CapsuleLightRender();
}

void LightObjects::PointLightControll(int index)
{
	if (index < 0)return;
	auto ShowFloat = [](const char* label, float* value)
	{
		float step = 1.0f;
		float step_fast = 1.0f;
		char* format = const_cast<char*>("%.3f");
		auto flags = ImGuiInputTextFlags_CharsDecimal;

		ImGui::PushItemWidth(100.0f);
		ImGui::InputFloat(label, value, step, step_fast, format, flags);
		ImGui::PopItemWidth();
	};

	if (ImGui::CollapsingHeader("PointLight"))
	{
		ImGui::ColorEdit3("Point Specular", (float*)&pointLight->GetPointLight(index).Specular);
		ImGui::ColorEdit3("Point Diffuse", (float*)&pointLight->GetPointLight(index).Diffuse);
		ImGui::Text("Light Position");
		ImGui::SameLine(70.0f); ShowFloat("##LightPositionX", &pointLight->GetPointLight(index).Position.x);
		ImGui::SameLine();      ShowFloat("##LightPositionY", &pointLight->GetPointLight(index).Position.y);
		ImGui::SameLine();      ShowFloat("##LightPositionZ", &pointLight->GetPointLight(index).Position.z);
		ImGui::Text("Point Intensity");
		ImGui::SameLine(70.0f); ShowFloat("##Point Intensity", &pointLight->GetPointLight(index).Intensity);
		ImGui::Text("Point Range");
		ImGui::SameLine(70.0f); ShowFloat("##Point Range", &pointLight->GetPointLight(index).Range);
		
	}

	
	//pointTransform->Position(&pointLight->GetPointLight(index).Position);
	
}

void LightObjects::SpotLightControll(int index)
{
	if (index < 0)return;

	if (ImGui::CollapsingHeader("SpotLight"))
	{
		ImGui::ColorEdit3("Spot Diffuse", (float*)&spotLight->GetSpotLight(index).Diffuse);
		ImGui::ColorEdit3("Spot Specular", (float*)&spotLight->GetSpotLight(index).Specular);
		ImGui::SliderFloat3("Spot Position", (float*)&spotLight->GetSpotLight(index).Position, -180, 180, "%.03f");
		ImGui::SliderFloat3("Spot Direction", (float*)&spotLight->GetSpotLight(index).Direction, -1, 1, "%.3f");
		ImGui::SliderFloat("Spot Range", (float*)&spotLight->GetSpotLight(index).Range, 1.0f, 1000.0f);
		ImGui::SliderFloat("Spot Intensity", (float*)&spotLight->GetSpotLight(index).Intensity, 0.01f, 10.0f);
		
		ImGui::SliderFloat("Spot InnerAnlge", (float*)&spotLight->GetSpotLight(index).InnerAngle, 0.0f, 100.0f);
		
		ImGui::SliderFloat("Spot OuterAngle", (float*)&spotLight->GetSpotLight(index).OuterAngle, 0.0f, 100.0f);
		
		
	}
}

void LightObjects::CapsuleLightControll(int index)
{
}

void LightObjects::PointLightRender()
{
	if (pointLight->GetPointLightCount() > 0)
	{
		V = Context::Get()->View();
		P = Context::Get()->Projection();
		
		for (uint i = 0; i < pointLight->GetPointLightCount(); i++)
		{

			Vector3 position = Vector3(pointLight->GetPointLight(i).Position.x,
				pointLight->GetPointLight(i).Position.y+ (pointLight->GetPointLight(i).Range*0.5f),
				pointLight->GetPointLight(i).Position.z);
			
			Vector3 projection;
			Context::Get()->GetViewport()->Projection(&projection, position, world, V, P);


			Gui::Get()->RenderText(projection.x +30, projection.y, 0, 0, 1,
				"X:" + to_string(projection.x) + "Y:" + to_string(projection.y));
			Gui::Get()->RenderTexture(D3DXVECTOR2(projection.x, projection.y), D3DXVECTOR2(100.0f, 100.0f), pointLight->GetPointLight(i).Diffuse, pointTexture ? pointTexture : nullptr);

			//GridPointLight(pointLight->GetPointLight(i).Position, pointLight->GetPointLight(i).Range, pointLight->GetPointLight(i).Diffuse);
		}


	}
}

void LightObjects::SpotLightRender()
{
	if (spotLight->GetSpotLightCount() > 0)
	{
		
		V = Context::Get()->View();
		P = Context::Get()->Projection();

		for (uint i = 0; i < spotLight->GetSpotLightCount(); i++)
		{

			Vector3 position = Vector3(spotLight->GetSpotLight(i).Position.x,
				spotLight->GetSpotLight(i).Position.y,
				spotLight->GetSpotLight(i).Position.z);
			Vector3 projection;
			Context::Get()->GetViewport()->Projection(&projection, position, world, V, P);


			Gui::Get()->RenderText(projection.x + 30.0f, projection.y, 0, 0, 1,
				"X:" + to_string(projection.x) + "Y:" + to_string(projection.y));
			Gui::Get()->RenderTexture(D3DXVECTOR2(projection.x, projection.y), D3DXVECTOR2(80.0f, 80.0f), spotLight->GetSpotLight(i).Diffuse, spotTexture ? spotTexture : nullptr);

		//	GridSpotLight(i, spotLight->GetSpotLight(i).Diffuse);
		}


	}
}

void LightObjects::CapsuleLightRender()
{
}

void LightObjects::GridPointLight(Vector3 & pos, float & Range, Color color)
{
	float Range1 = Range - 5.0f;
	for (int i = 0; i < 360; i++)
	{
		xy[i] = pos + Vector3(Range1 * cosf(D3DXToRadian(i)), Range1 * sinf(D3DXToRadian(i)), 0);
		xz[i] = pos + Vector3(Range1 * cosf(D3DXToRadian(i)), 0, Range1 * sinf(D3DXToRadian(i)));
		yz[i] = pos + Vector3(0, Range1 * cosf(D3DXToRadian(i)), Range1 * sinf(D3DXToRadian(i)));
	}
	for (int i = 0; i < 359; i++)
	{
		DebugLine::Get()->RenderLine(xy[i], xy[i + 1], color);
		DebugLine::Get()->RenderLine(xz[i], xz[i + 1], color);
		DebugLine::Get()->RenderLine(yz[i], yz[i + 1], color);
	}
}

void LightObjects::GridSpotLight(uint index, Color color)
{
	UINT stackCount = 60;
	float thetaStep = 2.0f * Math::PI / stackCount;
	Vector3 pos =spotLight->GetSpotLight(index).Position;
	float dist = spotLight->GetSpotLight(index).Range;
	float angle = spotLight->GetSpotLight(index).OuterAngle;
	float radius = dist * tan(Math::ToRadian(angle));
	Vector3 dir = spotLight->GetSpotLight(index).Direction;

	Vector3 axis;
	D3DXVec3Cross(&axis, &Vector3(0, 0, 1), &dir);
	float radian = D3DXVec3Dot(&Vector3(0, 0, 1), &dir) - Math::PI*0.5f;

	Matrix R;
	D3DXMatrixRotationAxis(&R, &axis, radian);

	vector<Vector3> v;
	for (UINT i = 0; i <= stackCount; i++)
	{
		float theta = i * thetaStep;

		Vector3 p = Vector3
		(
			(radius * cosf(theta)),
			(radius * sinf(theta)),
			0
		);

		D3DXVec3TransformCoord(&p, &p, &R);
		p += pos;
		p += dir * (dist - 1.5f);
		v.emplace_back(p);
	}

	DebugLine::Get()->RenderLine(pos, v[stackCount / 4], color);
	DebugLine::Get()->RenderLine(pos, v[stackCount / 2], color);
	DebugLine::Get()->RenderLine(pos, v[stackCount * 3 / 4], color);
	DebugLine::Get()->RenderLine(pos, v[stackCount], color);


	for (UINT i = 0; i < stackCount; i++)
	{
		DebugLine::Get()->RenderLine(v[i], v[i + 1], color);
	}
}

void LightObjects::GridCapsuleLight(uint index, Color color)
{
}

void LightObjects::SetDragDropPayload(const LightType & type, const string & data)
{
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		ImGui::SetDragDropPayload
		(
			reinterpret_cast<const char*>(&type),
			data.c_str(),
			data.length() + 1

		);
		lightType = type;
		bDrag = true;
		dragLightName = String::ToWString(Path::GetFileNameWithoutExtension(data));
		ImGui::Text(Path::GetFileNameWithoutExtension(data).c_str());
		ImGui::EndDragDropSource();
	}
}

bool LightObjects::CheckIsPicked(Vector3 & proj, Vector3 & mousePos, LightType type)
{
	Matrix world1;
	D3DXMatrixIdentity(&world1);
	Matrix V1 = Context::Get()->View();
	Matrix P1 = Context::Get()->Projection();

	Vector3 projection1;
	switch (type)
	{
	case LightType::unKnown:
		break;
	case LightType::pointLight:
		Context::Get()->GetViewport()->Projection(&projection1, Vector3(proj.x, proj.y , proj.z), world1, V1, P1);
		break;
	case LightType::spotLight:
		Context::Get()->GetViewport()->Projection(&projection1, Vector3(proj.x, proj.y + 0.0f, proj.z), world1, V1, P1);
		break;
	case LightType::capsuleLight:
		Context::Get()->GetViewport()->Projection(&projection1, Vector3(proj.x, proj.y , proj.z), world1, V1, P1);
		break;

	}

	auto min = Vector3(projection1.x - 30.0f, projection1.y - 30.0f, projection1.z);
	auto max = Vector3(projection1.x + 30.0f, projection1.y + 30.0f, projection1.z);
	if (mousePos.x < min.x || mousePos.x>max.x ||
		mousePos.y< min.y || mousePos.y  > max.y)
	{
		return false;
	}
	return true;
}


