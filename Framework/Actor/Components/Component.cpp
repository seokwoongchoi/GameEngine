#include "Framework.h"
#include "Component.h"

Component::Component(Shader * shader, Model * model, SharedData* sharedData)
	:shader(nullptr), model(nullptr), sharedData(nullptr),  bStart(false)
{
	if (shader != nullptr)
		this->shader = shader;

	if (model != nullptr)
		this->model = model;

	if (sharedData != nullptr)
		this->sharedData = sharedData;

	
}
