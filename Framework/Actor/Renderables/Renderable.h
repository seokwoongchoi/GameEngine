#pragma once
#include "Model/ModelMesh.h"
#include "Model/ModelClip.h"
#include "Actor/SharedData.h"
//#include "Actor/IComponent.h"
class Renderable //:public IComponent
{
public:
	Renderable(
		class Shader* shader = nullptr,
		
		class SharedData* sharedData = nullptr);
	 virtual ~Renderable() = default;
	
	 virtual void SetModel(class Model* model) = 0;
	 virtual void Pass(const uint& pass) = 0;
	
	 virtual void OnDestroy() = 0;
	 virtual void Render() = 0;
	
	 virtual void ForwardRender() = 0;
	
	
protected:
	class Shader* shader;
	class Model model;
	class ModelMesh* mesh;
	class SharedData* sharedData;
	
	uint pass;
};

