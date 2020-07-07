#pragma once
#include "Renders/Buffers.h"

enum class MainActorState
{   
	Idle,
	Move,
	Fire,
	WalkFire
};
class ColliderSystem
{
public:
	static ColliderSystem* Get();

	static void Create();
	static void Delete();



public:
	explicit ColliderSystem();
	~ColliderSystem();
	
	ColliderSystem(const ColliderSystem&) = delete;
	ColliderSystem& operator=(const ColliderSystem&) = delete;
	void Update();
	inline ID3D11UnorderedAccessView * UAV() {
		return out_StructuredBufferUAV
			;
	}
	void SetMainActorState(const MainActorState& state)
	{
		this->state = state;
	}
	/*inline ID3D11ShaderResourceView * SRV() {
		return copy_StructuredBufferSRV
			;
	}*/
	
	/*void SetColliders(class SharedData* collider) {
		colliders.emplace_back(collider);
	}*/
	void SetStart(bool bStart)
	{
		this->bStart = bStart;
	}
	void AddDrawCount() { this->drawCount += 1; }
	void PopDrawCount() { this->drawCount -= 1; }

	void CreateComputeDesc();

private:
	void CreateCopyBuffer();
	void CopyResource();
private:
	void GetAimRay(OUT Vector3* position, OUT Vector3* direction);
private:
	static ColliderSystem* instance;
	class Shader* csShader;
	MainActorState state;

	//ID3D11Buffer			  *out_StructuredBuffer=nullptr;
	ID3D11Texture2D			  *out_StructuredBuffer = nullptr;
	ID3D11ShaderResourceView  *out_StructuredBufferSRV = nullptr;
	ID3D11UnorderedAccessView *out_StructuredBufferUAV = nullptr;

	uint drawCount;

	Vector3 boneBoxMin = Vector3(-0.5f, -0.5f, -0.5f);
	Vector3 boneBoxMax = Vector3(0.5f, 0.5f, 0.5f);
private:

	struct RayDesc
	{
		Vector3 org;
		float drawCount;
		Vector3 dir;
		float pad2;

	};
	RayDesc rayDesc;
	ConstantBuffer* rayBuffer;
	ID3DX11EffectConstantBuffer* sRayBuffer;

private:

	

	ID3DX11EffectShaderResourceVariable* sSrv;
	ID3DX11EffectUnorderedAccessViewVariable* sUav;

	bool bStart;
	//unordered_map<uint, uint>actorMap;

	uint actorCount = 0;

private:
	struct CS_AnimOutputDesc
	{
		Matrix Result;
		Color boneColor;


	};

	struct CS_TextureOutputDesc
	{
		Matrix body;
		Matrix head;
		Vector4 factor;


	};
	CS_TextureOutputDesc* csTexture = nullptr;
	struct CS_OutputDesc
	{
		Matrix Result;
		Color boneColor;


	};
	CS_AnimOutputDesc* csOutput = NULL;
	CS_OutputDesc* csOutput2 = NULL;
	
	ID3D11Buffer			  *buffer = nullptr;
	ID3D11ShaderResourceView  *srv = nullptr;
	ID3D11UnorderedAccessView*  uav = nullptr;

	

	
	/*Vector3 xy[30];
	Vector3 xz[30];
	Vector3 yz[30];*/

	Vector3 org, dir;

	Matrix world;
	Matrix invWorld;
	Vector2 point;
	Matrix invView;
	Vector3 cameraPosition;
	Matrix V;
	Matrix P;

	D3DXVECTOR3 dest[8];
private:

		struct CS_CopyDesc
		{
			Vector4 Indices;
			Vector4 Factor;
			Vector4 Direction;
		};

		CS_CopyDesc* csCopyInput = NULL;
		CS_CopyDesc* csCopyOutput = NULL;
		ID3D11Buffer			  *copy_StructuredBuffer = nullptr;
		ID3D11UnorderedAccessView  *copy_StructuredBufferUAV = nullptr;
	//	ID3D11ShaderResourceView*  copy_StructuredBufferSRV = nullptr;
		ID3D11Buffer		  *copyBuffer = nullptr;

		ID3DX11EffectUnorderedAccessViewVariable* sCopyUav;

		

		D3D11_MAPPED_SUBRESOURCE subResource;

		Color color;

		//vector<SharedData*> colliders;

		bool IsDameged;
		bool IsClac;
		INT64 currentTime;
};


