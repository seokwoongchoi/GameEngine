#pragma once
//#include "Actor/IComponent.h"
#include "Actor/SharedData.h"
class Component//:public IComponent
{
public:
	Component(class Shader* shader = nullptr,
		class Model* model = nullptr, class SharedData* sharedData=nullptr);
	 virtual ~Component() = default;

	 
	 virtual void OnDestroy() = 0;
	 virtual void OnStart() = 0;
	 virtual void OnUpdate() = 0;
	 virtual void OnStop() = 0;
	
	

protected:
	class Shader* shader;
	class Model* model;
	class SharedData* sharedData;

	bool bStart;
	


	
};
