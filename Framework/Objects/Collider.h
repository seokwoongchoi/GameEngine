#pragma once

struct Obb
{
	Vector3 Position;
	Vector3 AxisX;
	Vector3 AxisY;
	Vector3 AxisZ;

	Vector3 HalfSize;
};
class Collider
{
public:
	Collider(Transform* transform,Transform* init=NULL);
	~Collider();

	bool Isintersect(Collider* other);
	void Render(Color color, vector<Vector3> lines);

	Transform* GetTransform() { return transform; }

	void Update();
	void SetObb();
private:
	bool SperatingPlane(Vector3& position, Vector3& direction, Obb& box1, Obb& box2);
	bool Collision(Obb& box1, Obb& box2);
	Vector3 Cross(Vector3& vec1, Vector3& vec2);

	Obb obb;
	Transform* transform;
	//Vector3 lines[8];
	vector<Vector3> lines;

	Transform* init;
	
};

