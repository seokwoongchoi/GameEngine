#pragma once
enum class CameraType
{
	Free,
	Orbit,
	
};
class Camera
{
public:
	Camera();
	virtual ~Camera();

	virtual void Update() = 0;
public://Orbit
	inline void SetOrbitTargetPosition(const Vector3& pos)
	{
		targetPosition = pos;
	}
	inline void SetMoveValue(const Vector2& value)
	{
		moveValue = value;
	}
	inline void SetDeltaPos(const Vector3& pos)
	{
		deltaPos = pos;
	}
	inline const Vector3& GetLookAtPt()
	{
		switch (cameraType)
		{
		case CameraType::Free:
		{
			const Vector3& temp = (position + forward);
			return temp;
		}
			
			break;
		case CameraType::Orbit:
			return targetPosition;
			break;

		}
	}
public:
	void Position(float x, float y, float z);
	void Position(Vector3 vec);
	void Position(Vector3* vec);

	void Rotation(float x, float y, float z);
	void Rotation(Vector3& vec);
	void Rotation(Vector3* vec);


	void RotationDegree(float x, float y, float z);
	void RotationDegree(Vector3& vec);
	void RotationDegree(Vector3* vec);

	void GetMatrix(Matrix* matrix);

	void Forward(const Vector3& forward) { this-> forward= forward; }
	const Vector3& Forward() { return forward; }
	void Forward(Vector3* vec);
	Vector3& Right() { return right; }
	void Right(Vector3* vec);
	Vector3& Up() { return up; }
	void Up(Vector3* vec);

	void SetCameraType(CameraType cameraType);
	CameraType GetCameraType() { return cameraType; }


protected:
	virtual void Move();
	virtual void Rotation();

protected:
	void View();
	void OrbitView();


	Vector3 forward;
private:
	Vector3 position;
	Vector3 rotation;
	Vector3 dir;


	Vector3 up;
	Vector3 right;

	Matrix matRotation;
	CameraType cameraType;
protected:
	Matrix matView;
protected://Orbit
	Vector2 moveValue = Vector2(0, 0);
	Vector3 deltaPos = Vector3(0, 0, 0);
	Vector3 targetPosition = Vector3(0, 0, 0);
};