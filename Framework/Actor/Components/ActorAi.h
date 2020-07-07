#pragma once
#include "Component.h"
class ActorAi : public Component
{
public:
	ActorAi(class Shader* shader = nullptr,
		class Model* model = nullptr, class SharedData* sharedData = nullptr);
	~ActorAi() = default;

	
	void OnDestroy() override;

	void OnStart() override;
	void OnUpdate() override;
	void OnStop() override;

	void SetBehaviorTree(const uint& num);
		 
	bool IsSeeEnemy(const uint& index);
private:
	bool bStart = false;
	
	float speed = 1.0f;

	bool IsSetBt = false;
	BehaviorTree* bt = nullptr;

	Vector3 position;
	uint count;
	Vector3 forward;
	Vector3 dir;
	float dist = 20.0f;
	float angle;
	Vector3 findPosition;
private:
	Vector3 s;
	Vector3 p;
	Quaternion q;
	Matrix S;
	Matrix T;
	Matrix R;
};

