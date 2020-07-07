#pragma once

class Transform
{
public:
	Transform();
	Transform(Shader* shader);
	~Transform();


	inline bool IsCulled() {return bCulled;	}
	inline void SetCulled(bool bCulled) {this->bCulled= bCulled; }
	void Set(Transform* transform);

	void SetShader(Shader* shader);

	void Position(float x, float y, float z);
	void Position(Vector3& vec);
	void Position(Vector3* vec);
	Vector3 GetPosition() { return position; }

	void Scale(float x, float y, float z);
	void Scale(Vector3& vec);
	void Scale(Vector3* vec);
	Vector3 GetScale() { return scale; }

	void Rotation(float x, float y, float z,float w);
	void Rotation(Quaternion& vec);
	void Rotation(Quaternion* vec);
	//Vector3 GetRotation() { return rotation; }
	Matrix GetRotationMatrix();

	void RotationDegree(float x, float y, float z,float w);
	void RotationDegree(Quaternion& vec);
	void RotationDegree(Quaternion* vec);

	Vector3 Direction();
	Vector3 Up();
	Vector3 Right();

	void WorldDecompose(const Matrix& matrix);
	void World(const Matrix& matrix) { bufferDesc.World = matrix; }
	inline const Matrix& World() { return bufferDesc.World; }
	inline  Matrix* WorldPointer() { return &bufferDesc.World; }

	Transform* GetParent() const { return parent; }
	void SetParent(Transform* newParent) { this->parent = newParent; }
	void SetChild(Transform* child) { childs.emplace_back(child); }

	const bool HasParent() const { return parent ? true : false; }
	const bool HasChilds() const { return !childs.empty(); }

	void SetLocalScale(const D3DXVECTOR3& vec);
	void SetLocalRotation(const Quaternion& vec);
	void SetLocalTranslation(const D3DXVECTOR3& vec);
	

	void UpdateWorld();

public:
	void Update();
	void Render();

	

private:
	struct BufferDesc
	{
		Matrix World;
	} bufferDesc;

private:
	Shader* shader;

	Transform* parent;
	std::vector<Transform*> childs;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;


	Vector3 position;
	Vector3 scale;
	Quaternion rotation;

	D3DXMATRIX local;
	D3DXMATRIX world;

	
	bool bParent;
	bool bCulled = false;
};
