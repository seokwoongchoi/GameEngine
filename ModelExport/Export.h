#pragma once
#include "Systems/IExecute.h"

class Export : public IExecute
{
public:
	virtual void Initialize() override;
	virtual void Ready() override {}
	virtual void Destroy() override {};
	virtual void Update() override {};
	
	virtual void Render() override {};
	
	virtual void ResizeScreen() override {}

private:
	void House();
	void Grass();
	void Tank();
	void Tower();
	void Kachujin();
	void Eclipse();
	void Samurai();
	void Nyra();
	void Katanami();
	void Shogun();
	void Tree();
	void Car();
	void Paladin();
	void Douglas();
	void Bridge();
	void Police();
	void Lamp();
	void Alex();
	
	void Katana();
	void M4();

	void SWAT();
	void Mask();
	void James();
};