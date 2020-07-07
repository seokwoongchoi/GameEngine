#pragma once

class IExecute
{
public:
	virtual void Initialize() = 0;
	virtual void Ready() = 0;
	virtual void Destroy() = 0;

	virtual void Update() = 0;

	
	virtual void Render() = 0;
	

	virtual void ResizeScreen() = 0;
};