#pragma once
#include "Systems/IExecute.h"

class Main : public IExecute
{
public:
	virtual void Initialize() override;
	virtual void Ready() override;
	virtual void Destroy() override;
	virtual void Update() override;
	
	virtual void Render() override;
	
	virtual void ResizeScreen() override;

private:
	void Push(IExecute* execute);

private:
	vector<IExecute *> executes;
	
};