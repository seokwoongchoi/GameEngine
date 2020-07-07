#include "Framework.h"
#include "ActorAi.h"


ActorAi::ActorAi(Shader * shader, Model * model, SharedData * sharedData)
	:Component(shader, model, sharedData),count(0),angle(0.0f)

{
	

}

void ActorAi::OnDestroy()
{
}

void ActorAi::OnStart()
{
	bStart = true;
}

void ActorAi::OnUpdate()
{
	if (!bStart||!IsSetBt) return;
	
	
	Thread::Get()->AddTask([&]()
	{

		for (uint i = 0; i < sharedData->drawCount; i++)
		{
			if (sharedData->IsPlayer&&i == 0)
			{
				continue;
			}
			bt->SetIsSeeEnemy(IsSeeEnemy(i));
			bt->Tick(i);
		}
	});
}

void ActorAi::OnStop()
{
	bStart = false;
}

void ActorAi::SetBehaviorTree(const uint & num)
{
	if (sharedData->behviorTreeNum == num) return;
	IsSetBt = true;
	sharedData->behviorTreeNum = num;

	//if (bt) SafeDelete(bt);

	bt = EventSystem::Get()->GetBT(num);
	//memcpy(bt, EventSystem::Get()->GetBT(num), sizeof(BehaviorTree));
	
	bt->SetActorData(sharedData->actorIndex);
	
}

bool ActorAi::IsSeeEnemy(const uint & index)
{
	 count = sharedData->TransformsCount();
	position = sharedData->GetPosition(index);

	

	for (uint a = 0; a < count; a++)
	{
		if (index == a) continue;
	    findPosition = sharedData->GetPosition(a);
		float temp = (findPosition.x - position.x)*(findPosition.x - position.x) +
			(findPosition.z - position.z)*(findPosition.z - position.z);

	
		if (temp < dist*dist)
		{
			/*dir = findPosition - position;
			double x = dir.x;
			double y = 0.0f;
			double z = dir.z;
			D3DXVec3Normalize(&dir, &Vector3(x, y, z));
			forward = sharedData->GetForward(index);
			
			x = forward.x;
			
			z = forward.z;
			D3DXVec3Normalize(&forward, &Vector3(x,y,z));
			angle = D3DXVec3Dot(&dir, &forward);
			*/

			//D3DXVECTOR3 delta = position - findPosition > 0 ? position - findPosition : (position - findPosition)*-1;
			dir = position - findPosition > 0 ? position - findPosition : (position - findPosition)*-1;
			double x = dir.x;
			double y = 0.0f;
			double z = dir.z;
			D3DXVec3Normalize(&dir, &Vector3(x, y, z));
			float line = D3DXVec3Length(&dir);

			float anglez = acosf(dir.z / line);
			float anglex = acosf(dir.x / line);
			if (findPosition.x<position.x&& findPosition.z>position.z)
			{
				anglez -= D3DXToRadian(360);
				angle = anglez;
			}
			else if (findPosition.x > position.x&& findPosition.z < position.z)
			{
				anglex -= D3DXToRadian(90);
				angle = -anglex;
			}
			else if (findPosition.x < position.x&& findPosition.z < position.z)
			{
				anglez -= D3DXToRadian(360);
				angle = anglez;
			}
			else if (findPosition.x > position.x&& findPosition.z > position.z)
			{
				anglex -= D3DXToRadian(270);
				angle = anglex;
			}
			
			D3DXMatrixDecompose(&s, &q, &p, &sharedData->transforms[index]);
			D3DXMatrixScaling(&S, s.x, s.y, s.z);
			D3DXMatrixTranslation(&T, p.x, p.y, p.z);
			D3DXMatrixRotationY(&R, angle);

			sharedData->transforms[index] = S * R*T;
			
			//sharedData->SetAngle(acosf(angle), index);
			return true;
		}
		else
		{
			return false;
    	}

	}
}
