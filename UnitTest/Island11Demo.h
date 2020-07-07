#pragma once
#include "Systems/IExecute.h"

class Island11Demo : public IExecute
{

public:
	virtual void Initialize() override;
	virtual void Ready() override {}
	virtual void Destroy() override {}
	virtual void Update() override;

	virtual void Render() override;

	virtual void ResizeScreen() override {}

	class Island11* island11;
};

