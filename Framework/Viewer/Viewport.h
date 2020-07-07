#pragma once

class Viewport
{
public:
	Viewport(float width, float height, float x = 0, float y = 0, float minDepth = 0, float maxDepth = 1);
	~Viewport();

	void RSSetViewport();
	void Set(float width, float height, float x = 0, float y = 0, float minDepth = 0, float maxDepth = 1);

	float GetWidth() { return width; }
	float GetHeight() { return height; }

	void GetRay(OUT Vector3* position,OUT Vector3* direction,IN Matrix& w, IN Matrix& v, IN Matrix& p);
	void Unprojection(OUT Vector3* position, Vector3& source, Matrix& W, Matrix& V, Matrix& P);
	void Projection(OUT Vector3* position, Vector3& source, Matrix& W, Matrix& V, Matrix& P);
private:
	float x, y;
	float width, height;
	float minDepth, maxDepth;

	D3D11_VIEWPORT viewport;

	
};