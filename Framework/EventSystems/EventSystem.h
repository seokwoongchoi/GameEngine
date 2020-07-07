#pragma once
#include "Utilities/Xml.h"
#include "Utilities/BinaryFile.h"
enum class BehaviorTreeSave
{
	Selector,
	Sequence,
	SimpleParallel,
	Condition1,
	Condition2,
	Condition3,
	Action1,
	Action2,
	Action3,
	Back,
	End,
};
class EventSystem
{
public:
	static EventSystem* Get();

	static void Create();
	static void Delete();

	
	void ResidenceAnimator(class Animator* animator);
	bool IsEndAnimation(uint actor,uint instance, uint clip);

public://Save
	
	
	void CreateWriter(wstring fileName);
	void CloseWriter()
	{
		w->Close();
		SafeDelete(w);
	}
	BinaryWriter* Writer()
	{
		return w;
	}
	
public://Load
	
	void CreateReader(wstring fileName);
	void CloseReader()
	{
		r->Close();
		SafeDelete(r);
	}
	BinaryReader* Reader()
	{
		return r;
	}

public://Save
	void CreateDoc();
	Xml::XMLDocument* GetDoc() {
		return document
			;
	}
	void CreateNode(string name);
	void SetElemnet(Xml::XMLElement* element);
	void DeleteElement();
	void MakeFile(string fileName);
public://Load
	void CreateDoc(string file);
	Xml::XMLElement* GetNode()
	{
		return node;
	}

	Xml::XMLDocument* document = nullptr;
	Xml::XMLElement*node=nullptr;
	Xml::XMLElement* root = nullptr;
public:
	EventSystem();
	~EventSystem()=default;
	unordered_map<string, function<void(uint actor,uint instance)>> Events;
	function<void(uint actor, uint instance,Vector3 dir)> Collison;


public:
	void CreateBuilder();
	uint GetBTData();
	void Back();
	void ActiveSelector();
	void Sequence();
	void SimpleParallel();
	void Condition(uint conditionMode, bool isReverse);
	void Action(uint actionMode);

	void End();
	void Tick(const uint& btNum,const uint& actorNum);
	BehaviorTree* GetBT(const uint& index);


private:
	static EventSystem* instance;
	vector<class Animator*> animators;

	int clip = -1;

	
	vector<string> EventsName;
	vector<BehaviorTreeSave>behaviorTreeSaved;

	vector<BehaviorTree*> Bts;
	
	BinaryWriter* w = nullptr;
	BinaryReader* r = nullptr;
};
