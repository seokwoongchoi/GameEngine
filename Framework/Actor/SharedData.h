#pragma once
//#include "Actor/Components/Animator.h"


class SharedData
{
	
public:
	
	friend class ActorCamera;
	friend class ParticleSimulation;
	friend class Actor;
	friend class Animator;
	friend class ActorCollider;
	friend class SkeletalMesh;
	friend class Command;
	friend class StaticMesh;
	friend class ActorAi;
	friend class Action_Patrol;
	friend class Condition_IsSeeEnemy;
public:
	SharedData();
	~SharedData();

	const uint& ActorIndex()
	{
		return actorIndex;
	}
	SharedData(const SharedData& rhs) = delete;
	SharedData& operator=(const SharedData& rhs) = delete;
	
	void Update(const uint& pass);
	void EffectUpdate(const uint& particleIndex);

	void PopTransform(const uint& index);
	void PushTransform(const uint& culledIndex,const Matrix& culledMatrix);
	void SortIndeies(const uint& DiedIndex);
	bool DeleteDiedInstance(const uint& DiedIndex);

	const Matrix& TransformWithoutScale(const uint& index);

	
	inline uint IndexNum(const uint& num)
	{
		return Index[num];
	}
	

public:
	inline uint TotalCount() { return drawCount + culledCount; }
	inline uint TransformsCount() { return drawCount; }
	inline Matrix* Transforms() { return &transforms[0]; }
public:
	
	inline const Vector3& GetForward(const uint& index)
	{
		const Vector3& temp = Vector3(transforms[index]._31, transforms[index]._32, transforms[index]._33);
		return temp;
	}
	inline const Vector3& GetRight(const uint& index)
	{
		const Vector3& temp= Vector3(transforms[index]._11, transforms[index]._12, transforms[index]._13);
		return temp;
	}
	inline const Vector3& GetPosition(const uint& index)
	{
		const Vector3& temp = Vector3(transforms[index]._41, transforms[index]._42, transforms[index]._43);
		return temp;
	}
	

	
	inline void SetPosition(const Vector3& position,const uint& index)
	{
		transforms[index]._41 = position.x;
		transforms[index]._42 = position.y;
		transforms[index]._43 = position.z;
	}
	
public:
	int* GetActiveIndex() { return &Index[0]; }
	uint GetActiveInstanceCount() { return Index.size(); }
protected:
	uint actorIndex;
	int behviorTreeNum = -1;
	bool IsPlayer = false;
	uint drawCount = 0;
	deque<Matrix>transforms;

	deque<int> Index;
	uint culledCount;

	Shader* computeShader;
	

	struct DrawCountDesc
	{
		uint drawCount=1;
		uint actorIndex = 0;
		uint particleIndex=1;
		uint pad;
	} drawCountDesc;
	ConstantBuffer* drawCountBuffer;
	ID3DX11EffectConstantBuffer* sDrawCountBuffer;

	ID3DX11EffectShaderResourceVariable* sInstSrv;
	

	ID3D11Texture2D			  *InstBuffer;
	ID3D11ShaderResourceView  *InstBufferSRV;

	Matrix instMatrix[MAX_MODEL_INSTANCE];
	bool bNeedSortCulledTransform = false;
	bool bNeedSortTransform = false;
	int CulledTransformIndex = 100;
	int DiedTransformIndex = 100;

	
private:
	
	
	
};


