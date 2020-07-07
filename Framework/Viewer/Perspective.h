#pragma once

class Perspective : public Projection
{
public:
	Perspective(float width, float height, float zn = 0.1f, float zf = 1000.0f, float fov = (float)D3DX_PI * 0.25f);
	~Perspective();
	Perspective(const Perspective& rhs) = delete;
	Perspective& operator=(const Perspective& rhs) = delete;
	void Set(float width, float height, float zn = 0.1f, float zf = 1000.0f, float fov = (float)D3DX_PI * 0.25) override;

private:
	float aspect;

	
};