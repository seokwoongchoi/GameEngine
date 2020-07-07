#include "Framework.h"
#include "Renderable.h"

Renderable::Renderable(Shader * shader, SharedData* sharedData)
	:shader(nullptr), sharedData(nullptr), pass(0)
{
	if (shader != nullptr)
		this->shader = shader;

	if (sharedData != nullptr)
		this->sharedData = sharedData;
	
}
