#include "Framework.h"
#include "ActorCollider.h"
#include "Model/ModelMesh.h"

ActorCollider::ActorCollider(Shader* shader, Model* model, SharedData* sharedData)
	: Component(shader, model, sharedData), selectBoxNum(-1), bClicked(false)
    
{
	
	frustum = new Frustum();
	D3DXMatrixIdentity(&world);
	
	computeBoxBuffer = new ConstantBuffer(&boxDesc, sizeof(BoxDesc));
	sComputeBoxBuffer = sharedData->computeShader->AsConstantBuffer("CB_Box");

	
	sUav = sharedData->computeShader->AsUAV("Output");

	//lledTransforms.resize(20);
}

ActorCollider::~ActorCollider()
{
	SafeDelete(frustum);
	SafeDelete(computeBoxBuffer);
	
}

void ActorCollider::OnDestroy()
{
	
}


void ActorCollider::OnStart()
{
	bStart = true;
}

void ActorCollider::OnUpdate()
{
 	V = Context::Get()->View();
	P = Context::Get()->Projection();
	Context::Get()->GetViewport()->GetRay(&org, &dir, world, V, P);
	frustum->Update();
	

	
	for (uint i = 0; i < sharedData->drawCount; i++)
	//for (auto& i : sharedData->Index)
	{
	
		
		for (uint b = 0; b < 8; b++)
		{
			D3DXVec3TransformCoord(&dest[b], &temp[b], &sharedData->transforms[i]);
		}

		bool result = frustum->ContainCube((dest[4] + dest[3])*0.5f, (dest[3].x - dest[4].x)*0.5f);
		//bool result = Frustum(i);
		if (!result)
		{
			if (sharedData->IsPlayer&&i == 0)
			{
				continue;
			}
    		culledTransforms.emplace_back(make_pair(sharedData->transforms[i], sharedData->Index[i]));
			sharedData->PopTransform(i);
			if (sharedData->actorIndex > 0)
			{
				int a = 0;
			}
		}

		if (!bStart)
		{
			DebugLine::Get()->Pass(0);
			//Front
			DebugLine::Get()->RenderLine(dest[0], dest[1], color);
			DebugLine::Get()->RenderLine(dest[1], dest[3], color);
			DebugLine::Get()->RenderLine(dest[3], dest[2], color);
			DebugLine::Get()->RenderLine(dest[2], dest[0], color);

			//Backward
			DebugLine::Get()->RenderLine(dest[4], dest[5], color);
			DebugLine::Get()->RenderLine(dest[5], dest[7], color);
			DebugLine::Get()->RenderLine(dest[7], dest[6], color);
			DebugLine::Get()->RenderLine(dest[6], dest[4], color);

			//Side
			DebugLine::Get()->RenderLine(dest[0], dest[4], color);
			DebugLine::Get()->RenderLine(dest[1], dest[5], color);
			DebugLine::Get()->RenderLine(dest[2], dest[6], color);
			DebugLine::Get()->RenderLine(dest[3], dest[7], color);
		}
	
	}
	

	
	
	
	if (!culledTransforms.empty())
	{
		for (uint i = 0; i < culledTransforms.size(); i++)
		{
			for (uint b = 0; b < 8; b++)
			{
				D3DXVec3TransformCoord(&cdest[b], &temp[b], &culledTransforms[i].first);
			}

			bool result = frustum->ContainCube((cdest[4] + cdest[3])*0.5f, (cdest[3].x - cdest[4].x)*0.5f);
			//bool result = Frustum(i);
			if (!result) continue;


			sharedData->PushTransform(culledTransforms[i].second, culledTransforms[i].first);
			culledTransforms.erase(culledTransforms.begin() + i);
			//culledTransforms.shrink_to_fit();

			if (sharedData->actorIndex > 0)
			{
				int a = 0;
			}

		}
	}
		

	if (sharedData->bNeedSortCulledTransform)
	{
		for (uint i = 0; i < culledTransforms.size(); i++)
		{

			if (static_cast<uint>(sharedData->CulledTransformIndex) < culledTransforms[i].second)
			{
				culledTransforms[i].second = culledTransforms[i].second - 1;
				
			}
		}
		sharedData->bNeedSortCulledTransform = false;
		//sharedData->CulledTransformIndex = 100;
	}

	

	if(IsCalc)
	CS_CalcBoxTransform();
	
	if (bStart) return;

	if (Keyboard::Get()->Down(1))
	{
		bClicked = true;
	}
	if (!bClicked || sharedData->drawCount == 0) return;




	Vector3 pos;


	float d = 0;


	for (uint i = 0; i < sharedData-> drawCount; i++)
	{
		for (uint b = 0; b < 8; b++)
		{
			D3DXVec3TransformCoord(&dest[b], &temp[b], &sharedData->transforms[i]);
		}
		

		if (IntersectionAABB(org, dir, pos, dest[4], dest[3], d))
		{
			selectBoxNum = i;
			Gui::Get()->AddTransform(&sharedData->transforms[i]);


		}
	}
	bClicked = false;
		
}



void ActorCollider::TranslationBoxWorld(const Vector3& min,const Vector3& max)
{
	boundsMin = min;
	boundsMax = max;


	temp[0] = Vector3(boundsMin.x, boundsMin.y, boundsMax.z);
	temp[1] = Vector3(boundsMax.x, boundsMin.y, boundsMax.z);
	temp[2] = Vector3(boundsMin.x, boundsMax.y, boundsMax.z);
	temp[3] = Vector3(boundsMax);
	temp[4] = Vector3(boundsMin);
	temp[5] = Vector3(boundsMax.x, boundsMin.y, boundsMin.z);
	temp[6] = Vector3(boundsMin.x, boundsMax.y, boundsMin.z);
	temp[7] = Vector3(boundsMax.x, boundsMax.y, boundsMin.z);

	
	Vector3 size = (max- min );
	Matrix S;
	D3DXMatrixScaling(&S, size.x, size.y , size.z );
	Vector3 center = (max + min)*0.5f;
	Matrix T;
	D3DXMatrixTranslation(&T, center.x, center.y, center.z);

	boxDesc.BoxBound = S*T;


	computeBoxBuffer->Apply();
	sComputeBoxBuffer->SetConstantBuffer(computeBoxBuffer->Buffer());
}




void ActorCollider::CS_CalcBoxTransform()
{
		
	sUav->SetUnorderedAccessView(ColliderSystem::Get()->UAV());
	sharedData->Update(1);
}



bool ActorCollider::IntersectionAABB(Vector3 org, Vector3 dir, Vector3 & Pos, Vector3 boundsMin, Vector3 boundsMax, float & d)
{
	float t_min = FLT_MIN;
	float t_max = FLT_MAX;


	for (int i = 0; i < 3; i++)
	{
		if (abs(dir[i]) < Math::EPSILON)
		{
			if (org[i] < boundsMin[i] ||
				org[i] >boundsMax[i])
			{

				return false;
			}

		}
		else
		{
			float denom = 1.0f / dir[i];
			float t1 = (boundsMin[i] - org[i]) * denom;
			float t2 = (boundsMax[i] - org[i]) * denom;

			if (t1 > t2)
			{
				swap(t1, t2);
			}

			t_min = max(t_min, t1);
			t_max = min(t_max, t2);

			if (t_min > t_max)
			{

				return false;
			}


		}
	}

	Vector3 hit = org + t_min * dir;



	d = t_min;
	return true;
}

//bool ActorCollider::Frustum(const uint& index)
//{
//
//	planeNormals= Context::Get()->PlaneNormals();
//	
//		for (uint b = 0; b < 4; b++)
//		{
//			
//			int culled = 1;
//			culled &= D3DXVec3Dot(&(dest[0] - org), &Vector3(planeNormals[b].x, planeNormals[b].y, planeNormals[b].z) ) > 0;
//			culled &= D3DXVec3Dot(&(dest[1] - org), &Vector3(planeNormals[b].x, planeNormals[b].y, planeNormals[b].z) ) > 0;
//			culled &= D3DXVec3Dot(&(dest[2] - org), &Vector3(planeNormals[b].x, planeNormals[b].y, planeNormals[b].z) ) > 0;
//			culled &= D3DXVec3Dot(&(dest[3] - org), &Vector3(planeNormals[b].x, planeNormals[b].y, planeNormals[b].z) ) > 0;
//			if (culled)
//				return false;
//		}
//
//		return true;
//}



void ActorCollider::OnStop()
{
	bStart = false;
}



