#pragma once
#include<stack>
#include "Behavior.h"
//namespace BT

	enum class EActionMode
	{
		Attack,
		Patrol,
		Runaway,
	};

	enum class EConditionMode
	{
		IsSeeEnemy,
		IsHealthLow,
		IsEnemyDead,
	};

	class BehaviorTree
	{
	public:
		explicit BehaviorTree(Behavior* InRoot) :Root(InRoot) {}

		
		BehaviorTree(const BehaviorTree&) = delete;
		BehaviorTree& operator=(const BehaviorTree&) = delete;

		void SetActorData(const uint& index);
		inline void Tick(const uint& actorIndex)
		{
			Root->Tick(actorIndex);
		}
		bool HaveRoot() { return Root ? true : false; }
		void SetRoot(Behavior* InNode) { Root = InNode; }
		
		void Release() { Root->Release(); }
	public:
		inline void SetIsSeeEnemy(bool condition)
		{
			Root->SetIsSeeEnemy(condition);
			
		}
		inline void SetIsHealthLow(bool condition)
		{
			Root->SetIsHealthLow(condition);
		}
		inline void SetIsEnemyDead(bool condition)
		{
			Root->SetIsEnemyDead(condition);
		}
		
	private:
		Behavior* Root;
		
	};

	//行为树构建器，用来构建一棵行为树,通过前序遍历方式配合Back()和End()方法进行构建
	class BehaviorTreeBuilder
	{
	public:
		BehaviorTreeBuilder() {}
		~BehaviorTreeBuilder() { }
		
		BehaviorTreeBuilder* Sequence();
		BehaviorTreeBuilder* Action(EActionMode ActionModes);
		BehaviorTreeBuilder* Condition(EConditionMode ConditionMode, bool IsNegation);
		BehaviorTreeBuilder* Selector();
		BehaviorTreeBuilder* Repeat(int RepeatNum);
		BehaviorTreeBuilder* ActiveSelector();
		BehaviorTreeBuilder* Filter();
		BehaviorTreeBuilder* Parallel(EPolicy InSucess, EPolicy InFailure);
		BehaviorTreeBuilder* Monitor(EPolicy InSucess, EPolicy InFailure);
		BehaviorTreeBuilder* Back();
		BehaviorTree* End();

	private:
		void AddBehavior(Behavior* NewBehavior);

	private:
		Behavior* TreeRoot = nullptr;
		//用于存储节点的堆栈
		std::stack<Behavior*> NodeStack;
	};


