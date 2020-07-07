#pragma once
#include "Component.h"
enum class ColliderShapeType : uint
{
    Box,
    Sphere,
    StaticPlane,
    Cylinder,
    Capsule,
    Cone,
    Mesh
};


class ActorCollider final : public Component
{
public:
	explicit ActorCollider
    (
		class Shader* shader = nullptr,
		class Model* model = nullptr, class SharedData* sharedData = nullptr
    );
    ~ActorCollider();

	
	ActorCollider(const ActorCollider&) = delete;
	ActorCollider& operator=(const ActorCollider&) = delete;
	void OnDestroy() override;

	void OnStart() override;
	void OnUpdate() override;
	void OnStop() override;
	
	void CalcInstBuffer()
	{
		IsCalc = true;
	}

	

	int GetSelectBoxNum() {return selectBoxNum;	}
	void SetBoxTransform(Matrix transform) {
		this->world = transform;

	}
	void TranslationBoxWorld( const Vector3& min, const Vector3& max);

	Matrix GetCulledTransforms(const uint& index) {
		return culledTransforms[index].first;
	}

	uint  GetCulledTransformIndex(const uint& index) {
		return culledTransforms[index].second;
	}
	
private:
	
	void CS_CalcBoxTransform();

	
	ID3D11ShaderResourceView  *srv = nullptr;
	ID3DX11EffectShaderResourceVariable* sSrv;

	uint pass = 0;;
private:
	bool IntersectionAABB(Vector3 org, Vector3 dir, Vector3 & Pos,Vector3 boundsMin,Vector3 boundsMax, float& d);
	

	int selectBoxNum;
	bool bClicked;

	
	Vector3 boundsMin, boundsMax;
	Vector3 wboundsMin, wboundsMax;


	Matrix boxWorld;
	
	Matrix world;


	class Frustum* frustum;

	D3DXVECTOR3 cdest[8],dest[8], temp[8];

	Color color = Color(1, 0, 0, 1);
	Vector3 org, dir;
	

	Matrix V;
	Matrix P;

	//deque<pair<Transform*,uint>>culledTransforms;
	 deque<pair<Matrix,uint>>culledTransforms;

private:

	 struct BoxDesc
	 {
		
		 Matrix BoxBound;
		
	 } boxDesc;

	


	 ConstantBuffer* computeBoxBuffer;
	 ID3DX11EffectConstantBuffer* sComputeBoxBuffer;
	
	
	 ID3DX11EffectUnorderedAccessViewVariable* sUav;
	 bool IsCalc = false;

	// D3DXVECTOR4* planeNormals;
	
	 //bool Frustum(const uint& index);
	 Vector3 pos;
	 float d = 0;
	 
};