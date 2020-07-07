#include "Framework.h"
#include "Behavior.h"


//std::random_device rd;
//std::default_random_engine engine(rd());
//std::uniform_int_distribution<> dis(1, 100);
////auto dice= std::bind(dis, engine);
//std::_Binder<std::_Unforced, std::uniform_int_distribution<>&, std::default_random_engine&> dice = std::bind(dis, engine);

 uint actorIndex = 0;

 bool IsSeeEnemy = false;
 bool IsHealthLow = false;
 bool IsEnemyDead = false;
void Behavior::SetActorData(const uint& index)
{
	actorIndex = index;
}

void Behavior::SetIsSeeEnemy(bool condition)
{
	IsSeeEnemy = condition;
}

void Behavior::SetIsHealthLow(bool condition)
{
	IsHealthLow = condition;
}

void Behavior::SetIsEnemyDead(bool condition)
{
	IsEnemyDead = condition;
}



EStatus  Behavior::Tick(const uint& instanceIndex)
{
	 
	if (Status != EStatus::Running)
	{
		OnInitialize();
	}

	Status = Update(instanceIndex);
	

	if (Status != EStatus::Running)
	{
		OnTerminate(Status);
	}

	return Status;
}



EStatus  Repeat::Update(const uint& instanceIndex)
{
	while (true)
	{
		Child->Tick(instanceIndex);
		if (Child->IsRunning())return EStatus::Success;
		if (Child->IsFailuer())return EStatus::Failure;
		if (++Count == Limited)return EStatus::Success;
		Child->Reset();
	}
	return EStatus::Invalid;
}

void  Composite::RemoveChild(Behavior * InChild)
{
	auto it = std::find(Children.begin(), Children.end(), InChild);
	if (it != Children.end())
	{
		Children.erase(it);
	}
}

EStatus  Sequence::Update(const uint& instanceIndex)
{
	while (true)
	{
		EStatus s = (*CurrChild)->Tick(instanceIndex);
	
		if (s != EStatus::Success)
			return s;
		if (++CurrChild == Children.end())
			return EStatus::Success;
	}
	return EStatus::Invalid; 
}



EStatus  Selector::Update(const uint& instanceIndex)
{
	while (true)
	{
		EStatus s = (*CurrChild)->Tick(instanceIndex);
		if (s != EStatus::Failure)
			return s;
		if (++CurrChild == Children.end())
			return EStatus::Failure;
	}
	return EStatus::Invalid;  //Ñ­»·ÒâÍâÖÕÖ¹
}



EStatus  Parallel::Update(const uint& instanceIndex)
{
	int SuccessCount = 0, FailureCount = 0;
	int ChildrenSize = Children.size();
	for (auto it : Children)
	{
		
		if (!it->IsTerminate())
			it->Tick(instanceIndex);

		if (it->IsSuccess())
		{
			++SuccessCount;
			if (SucessPolicy == EPolicy::RequireOne)
			{
				it->Reset();
				return EStatus::Success;
			}

		}

		if (it->IsFailuer())
		{
			++FailureCount;
			if (FailurePolicy == EPolicy::RequireOne)
			{
				it->Reset();
				return EStatus::Failure;
			}
		}
	}

	if (FailurePolicy == EPolicy::RequireAll&&FailureCount == ChildrenSize)
	{
		for (auto it : Children)
		{
			it->Reset();
		}

		return EStatus::Failure;
	}
	if (SucessPolicy == EPolicy::RequireAll&&SuccessCount == ChildrenSize)
	{
		for (auto it : Children)
		{
			it->Reset();
		}
		return EStatus::Success;
	}

	return EStatus::Running;
}

void  Parallel::OnTerminate(EStatus InStatus)
{
	for (auto it : Children)
	{
		if (it->IsRunning())
			it->Abort();
	}
}


EStatus  ActiveSelector::Update(const uint& instanceIndex)
{

	Behaviors::iterator Previous = CurrChild;
	
	Selector::OnInitialize();
	EStatus result = Selector::Update(instanceIndex);
	
	if (Previous != Children.end()&CurrChild != Previous)
	{
		(*Previous)->Abort();
	}

	return result;
}



EStatus  Condition_IsSeeEnemy::Update(const uint& instanceIndex)
{
	if (IsSeeEnemy)
	{
		//std::cout << "See enemy!" << std::endl;
		return !IsNegation ? EStatus::Success : EStatus::Failure;
	}
	else
	{
	//	std::cout << "Not see enemy" << std::endl;
		return !IsNegation ? EStatus::Failure : EStatus::Success;

	}
	
}

EStatus  Condition_IsHealthLow::Update(const uint& instanceIndex)
{
	
  
    if(IsHealthLow)
	{
		//std::cout << "Health is low" << std::endl;
		return !IsNegation ? EStatus::Success : EStatus::Failure;
	}
	else
	{
		//std::cout << "Health is not low" << std::endl;
		return !IsNegation ? EStatus::Failure : EStatus::Success;
	}
}

EStatus  Condition_IsEnemyDead::Update(const uint& instanceIndex)
{
    
	
	if(	IsEnemyDead)
	{
		//std::cout << "Enemy is Dead" << std::endl;
		return !IsNegation ? EStatus::Success : EStatus::Failure;
	}

	else
	{
		//std::cout << "Enemy is not Dead" << std::endl;
		return !IsNegation ? EStatus::Failure : EStatus::Success;
	}
}

EStatus  Action_Attack::Update(const uint& instanceIndex)
{
	EventSystem::Get()->Events["Fire"](actorIndex, instanceIndex);
	//std::cout << "Action_Attack " << std::endl;
	return EStatus::Success;
}

EStatus  Action_Runaway::Update(const uint& instanceIndex)
{
	EventSystem::Get()->Events["Dying"](actorIndex, instanceIndex);
	if (EventSystem::Get()->IsEndAnimation(actorIndex, instanceIndex, 3))
	{
	//	std::cout << "Action_Runaway" << std::endl;
		return EStatus::Success;
	}
		
	
	return EStatus::Failure;
}

EStatus  Action_Patrol::Update(const uint& instanceIndex)
{   
	/*IsArrive = false;
	
	
	Vector3& randomDir = Math::RandomVec3(-100, 100);
	randomDir.y = 0;
	D3DXVec3Normalize(&nor, &randomDir);

	position = actorData->GetPosition(instanceIndex);
	finish = position + distance * nor;

	Vector3 currPosition = actorData->GetPosition(instanceIndex);
	Vector3 delta = finish - currPosition > 0 ?
		finish - currPosition : currPosition;
	float line = D3DXVec3Length(&delta);
	float anglez = acosf(delta.z / line);
	float anglex = acosf(delta.x / line);

	

	if (finish.x<currPosition.x&& finish.z>currPosition.z)
	{
		anglez -= D3DXToRadian(180);
		rotation.y = anglez;
	}
	else if (finish.x > currPosition.x&& finish.z < currPosition.z)
	{
		anglex += D3DXToRadian(90);
		rotation.y = -anglex;
	}
	else if (finish.x < currPosition.x&& finish.z < currPosition.z)
	{
		anglez -= D3DXToRadian(180);
		rotation.y = anglez;
	}
	else if (finish.x > currPosition.x&& finish.z > currPosition.z)
	{
		anglex += D3DXToRadian(270);
		rotation.y = anglex;
	}
	
	
	EventSystem::Get()->Events["Move"](actorData->ActorIndex(), instanceIndex);
	
	
	
	while (!IsArrive)
	{
		if (static_cast<uint>(position.x) != static_cast<uint>(finish.x))
			position += speed * nor;
		else
		{
			IsArrive = true;
			break;
		}
			
	}
	
	
	actorData->SetPosition(position, instanceIndex);
	actorData->SetRotation(rotation, instanceIndex); 
	*/
	EventSystem::Get()->Events["Move"](actorIndex, instanceIndex);
	//std::cout << "Action_Patrol" << std::endl;
	return EStatus::Success;

}
