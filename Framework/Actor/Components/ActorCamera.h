#pragma once
#include "Component.h"

class ActorCamera final : public Component
{
public:
	explicit ActorCamera
    (
		class Shader* shader = nullptr,
		class Model* model = nullptr,class SharedData* sharedData = nullptr
    );
    ~ActorCamera() = default;
	
	ActorCamera(const ActorCamera&) = delete;
	ActorCamera& operator=(const ActorCamera&) = delete;

	void OnDestroy() override;

	void OnStart() override;
	void OnUpdate() override;
	void OnStop() override;

	void SetDeltaPos(const Vector3& pos)
	{
		orbit->SetDeltaPos(pos);
	}
private:
	void ActorCommand();

	
public:
   

	class Camera* orbit;
	uint camIndex;
private:
	Vector3 position = Vector3(0, 0, 0);
	Vector3 Forward = Vector3(0, 0, 1);
	Vector3 Up = Vector3(0, 1, 0);
	Vector3 Right = Vector3(1, 0, 0);
	Matrix R;

	Vector3 rotation = Vector3(0, 0, 0);

	Matrix S, T;
	Vector3 p, s, r;
	Quaternion q;
	Vector3 prevPosition = Vector3(0, 0, 0);

	Vector2 moveValue;
	bool bFire;
	float velocity;
	class Texture* aim;

	bool bPause = false;

	float xValue = 0;

	D3DDesc desc;
	POINT m_pt;
	
	uint actorIndex;

};