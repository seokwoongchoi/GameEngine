#pragma once

class Freedom : public Camera
{
public:
	Freedom();
	~Freedom();

	void Update() override;

	void Speed(float move, float rotation);

private:
	float move;
	float rotation;
};