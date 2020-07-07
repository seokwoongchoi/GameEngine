#include "Framework.h"
#include "EventSystem.h"
#include "Actor/Components/Animator.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Actor/SharedData.h"

vector<BehaviorTree*> Bts;

EventSystem* EventSystem::instance = NULL;
EventSystem * EventSystem::Get()
{
	return instance;
}

void EventSystem::Create()
{
	assert(instance == NULL);

	instance = new EventSystem();
}

void EventSystem::Delete()
{
	SafeDelete(instance);
}

void EventSystem::ResidenceAnimator(Animator * animator)
{
	animators.emplace_back(animator);
}



bool EventSystem::IsEndAnimation(uint actor, uint instance,uint clip)
{
	return animators[actor]->IsEndAnimation(instance, clip);
}


void EventSystem::CreateWriter(wstring fileName)
{
	w = new BinaryWriter();
	w->Open(fileName);
}


void EventSystem::CreateReader(wstring fileName)
{

	r = new BinaryReader();
	r->Open(fileName);
}

void EventSystem::CreateDoc()
{
	
	document = new Xml::XMLDocument();

	Xml::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);


	root = document->NewElement("Level");
	document->LinkEndChild(root);
}



void EventSystem::CreateNode(string name)
{
	
	node = document->NewElement(name.c_str());
	root->LinkEndChild(node);
}

void EventSystem::SetElemnet(Xml::XMLElement * element)
{
	node->LinkEndChild(element);
}

void EventSystem::DeleteElement()
{
	document = nullptr;
    node = nullptr;
	root = nullptr;
}

void EventSystem::MakeFile(string fileName)
{
	auto ext = ".Level";
	string sname = fileName;
	if (String::Contain(fileName, ext))
	{
		auto lastIndex = fileName.find_last_of('.');
		sname = fileName.substr(0, lastIndex);
	}
	
	
	document->SaveFile((sname + ext).c_str());
}

void EventSystem::CreateDoc(string file)
{

	Xml::XMLDocument* document = new Xml::XMLDocument();
	Xml::XMLError error = document->LoadFile(file.c_str());
	assert(error == Xml::XML_SUCCESS);

	root = document->FirstChildElement();
	node = root->FirstChildElement();
}

EventSystem::EventSystem()
	
{
	Events["Idle"] = [ this ](const uint& actor, const uint& instance)->void
	{
		// animators[actor]->PlayNextClip(instance, 0);
		
		animators[actor]->SetState(instance,ActorState::Idle);
	};
	Events["Fire"] = [this](const uint& actor, const uint& instance)->void
	{
		
		//animators[actor]->PlayNextClip(instance, 1);
		animators[actor]->SetState(instance,  ActorState::Fire);
		
		if (EffectSystem::Get()->SimulationsCount() > 1)
		{
			animators[actor]->CS_CalcEffectTransform(1);
			EffectSystem::Get()->SetActivated(true);

			SharedData* temp = animators[actor]->GetSharedData();
			if (temp)
			{
				EffectSystem::Get()->ResidenceSharedData(1, instance, temp);
			}
		}
		
		

	};
	Events["Move"]= [this](const uint& actor, const uint& instance)->void
	{
		
		//animators[actor]->PlayNextClip(instance,2);
		animators[actor]->SetState(instance, ActorState::Move);
		
	};
	
	Events["Dying"] = [this](const uint& actor, const uint& instance)->void
	{
		
		//animators[actor]->PlayNextClip(instance, 3);
		animators[actor]->SetState(instance, ActorState::Die);
		animators[actor]->CS_CalcEffectTransform(0);
		EffectSystem::Get()->SetActivated(true);
		
		SharedData* temp = animators[actor]->GetSharedData();
		if (temp)
		{
			EffectSystem::Get()->ResidenceSharedData(0, instance, temp);
		}
		
	};

	Events["WalkFire"] = [this](const uint& actor, const uint& instance)->void
	{
		
		//animators[actor]->PlayNextClip(instance, 4);
		animators[actor]->SetState(instance, ActorState::WalkFire);
		
		if (EffectSystem::Get()->SimulationsCount() > 1)
		{
			animators[actor]->CS_CalcEffectTransform(1);
			EffectSystem::Get()->SetActivated(true);

			SharedData* temp = animators[actor]->GetSharedData();
			if (temp)
			{
				EffectSystem::Get()->ResidenceSharedData(1, instance, temp);
			}
		}
	};

	Events["Reaction"] = [this](const uint& actor, const uint& instance)->void
	{
		//animators[actor]->PlayNextClip(instance, 5);
		
		animators[actor]->SetState(instance, ActorState::Reaction);
	};

	Events["Patrol"] = [this](const uint& actor, const uint& instance)->void
	{
		

		animators[actor]->SetState(instance, ActorState::Move);
	};

	Collison= [this](const uint& actor, const uint& instance,const Vector3& dir)->void
	{
		
		animators[actor]->Collison(instance,dir);
	};
	

	
}

void EventSystem::CreateBuilder()
{
	 BehaviorTreeBuilder* Builder = new  BehaviorTreeBuilder(); 
	 BehaviorTree* Bt = nullptr;
	for (auto& saved : behaviorTreeSaved)
	{
		switch (saved)
		{
		case BehaviorTreeSave::Selector:
			Builder->ActiveSelector();
			//cout << "Builder->ActiveSelector();" << endl;
			break;
		case BehaviorTreeSave::Sequence:
			Builder->Sequence();
			//cout << "Builder->Sequence();" << endl;
			break;
		case BehaviorTreeSave::SimpleParallel:
			Builder->Parallel( EPolicy::RequireAll,  EPolicy::RequireOne);
			//cout << "Builder->Parallel( EPolicy::RequireAll,  EPolicy::RequireOne);" << endl;
			break;
		case BehaviorTreeSave::Condition1:
			Builder->Condition( EConditionMode::IsSeeEnemy, false);
			//cout << "Builder->Condition( EConditionMode::IsSeeEnemy, false);" << endl;
			
			break;
		case BehaviorTreeSave::Condition2:
			Builder->Condition( EConditionMode::IsHealthLow, false);
			//cout << "Builder->Condition( EConditionMode::IsHealthLow, false);" << endl;
			break;
		case BehaviorTreeSave::Condition3:
			Builder->Condition( EConditionMode::IsEnemyDead, true);
			//cout << "Builder->Condition( EConditionMode::IsEnemyDead, false);" << endl;
			break;
		case BehaviorTreeSave::Action1:
			Builder->Action( EActionMode::Attack);
			//cout << "Builder->Action( EActionMode::Attack);" << endl;
			
			break;
		case BehaviorTreeSave::Action2:
			Builder->Action( EActionMode::Patrol);
			//cout << "Builder->Action( EActionMode::Patrol);" << endl;
			
			break;
		case BehaviorTreeSave::Action3:
			Builder->Action( EActionMode::Runaway);
			//cout << "Builder->Action( EActionMode::Runaway);" << endl;
			break;
		case BehaviorTreeSave::Back:
			Builder->Back();
			//cout << "Builder->Back();" << endl;
			break;
		case BehaviorTreeSave::End:
			Bt = Builder->End();
			//cout << "Bt = Builder->End();" << endl;
			break;
		
		}
	}
	behaviorTreeSaved.clear();
	behaviorTreeSaved.shrink_to_fit();
	Bts.emplace_back(Bt);
	SafeDelete(Builder);
	
}

uint EventSystem::GetBTData()
{
	return Bts.size();
}



void EventSystem::Back()
{
	behaviorTreeSaved.emplace_back(BehaviorTreeSave::Back);
}

void EventSystem::ActiveSelector()
{
	//Builder->ActiveSelector();
	behaviorTreeSaved.emplace_back(BehaviorTreeSave::Selector);
}

void EventSystem::Sequence()
{
	behaviorTreeSaved.emplace_back(BehaviorTreeSave::Sequence);
	//Builder->Sequence();
}

void EventSystem::SimpleParallel()
{
	behaviorTreeSaved.emplace_back(BehaviorTreeSave::SimpleParallel);
	//Builder->Parallel(BT::EPolicy::RequireAll, BT::EPolicy::RequireOne);
}

void EventSystem::Condition(uint conditionMode,bool isReverse)
{
	switch (conditionMode)
	{
	case 0:
		behaviorTreeSaved.emplace_back(BehaviorTreeSave::Condition1);
		//Builder->Condition(BT::EConditionMode::IsSeeEnemy, isReverse);
		break;
	case 1:
		behaviorTreeSaved.emplace_back(BehaviorTreeSave::Condition2);
		//Builder->Condition(BT::EConditionMode::IsHealthLow, isReverse);
		break;
	case 2:
		behaviorTreeSaved.emplace_back(BehaviorTreeSave::Condition3);
		//Builder->Condition(BT::EConditionMode::IsEnemyDead, isReverse);
		break;

		
	}
	
	
}

void EventSystem::Action(uint actionMode)
{

	
	switch (actionMode)
	{
	case 0:
		behaviorTreeSaved.emplace_back(BehaviorTreeSave::Action1);
		//Builder->Action(BT::EActionMode::Attack);
		break;
	case 1:
		behaviorTreeSaved.emplace_back(BehaviorTreeSave::Action2);
		//Builder->Action(BT::EActionMode::Patrol);
		//Bt = Builder->End();
		//delete Builder; 
		break;
	case 2:
		behaviorTreeSaved.emplace_back(BehaviorTreeSave::Action3);
		//Builder->Action(BT::EActionMode::Runaway);
		break;

		
	}

	
}



void EventSystem::End()
{
	behaviorTreeSaved.emplace_back(BehaviorTreeSave::End);
	//Bt = Builder->End();
	
}

void EventSystem::Tick(const uint& btNum, const uint& actorNum)
{
	Bts[btNum]->Tick(actorNum);
}

BehaviorTree * EventSystem::GetBT(const uint & index)
{
	return Bts[index];
}

