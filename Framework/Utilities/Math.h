#pragma once
class Math
{
public:
	static const float PI;
	static const float EPSILON;

	static float Modulo(float val1, float val2);

	static float ToRadian(float degree);
	static float ToDegree(float radian);

	static int Random(int r1, int r2);
	static float Random(float r1, float r2);
	
	static D3DXVECTOR2 RandomVec2(float r1, float r2);
	static D3DXVECTOR3 RandomVec3(float r1, float r2);
	static D3DXCOLOR RandomColor3();
	static D3DXCOLOR RandomColor4();


	static float Clamp(float value, float min, float max);

	static void LerpMatrix(OUT D3DXMATRIX& out, const D3DXMATRIX& m1, const D3DXMATRIX& m2, float amount);

	static D3DXQUATERNION LookAt(const D3DXVECTOR3& origin, const D3DXVECTOR3& target, const D3DXVECTOR3& up);

	static void toEulerAngle(const D3DXQUATERNION& q, float& pitch, float& yaw, float& roll);
	static void toEulerAngle(const D3DXQUATERNION& q, D3DXVECTOR3& out);

	static float Gaussian(float val, UINT blurCount);
	static void MatrixDecompose(const D3DXMATRIX& m, OUT Vector3& S, OUT Vector3& R, OUT Vector3& P);

	template <typename T>
	static const T Clamp(const T& x, const T& min, const T& max)
	{
		return x < min ? min : (x > max ? max : x);
	}

	template <typename T>
	static const T Abs(const T& value)
	{
		return value >= 0 ? value : -value;
	}

	template <typename T>
	static const T Max(const T& value1, const T& value2)
	{
		return value1 > value2 ? value1 : value2;
	}

	template <typename T>
	static const T Min(const T& value1, const T& value2)
	{
		return value1 < value2 ? value1 : value2;
	}
};