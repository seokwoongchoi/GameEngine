#pragma once
class Orbit: public Camera

{
public:
	Orbit();
	~Orbit();

	void Update() override;

	void PreviewUpdate();


	
private:
	
	float distance;
	Vector3 R;

	Vector3 dir = Vector3(1,1, 1);
	Vector3 position = Vector3(0, 0, 0);
	Matrix X, Y, Z;
	Matrix matrixR;
	
	bool bUpdate = false;
};

