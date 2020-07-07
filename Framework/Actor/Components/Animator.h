#pragma once
#include "Component.h"
enum class ActorState
{
	Idle,
	Fire,
	Move,
	Die,
	WalkFire,
	Reaction,

};

class Animator : public Component
{

public:
	explicit Animator(class Shader* shader = nullptr,
		class Model* model = nullptr, class SharedData* sharedData = nullptr);
	~Animator() = default;

	
	Animator(const Animator&) = delete;
	Animator& operator=(const Animator&) = delete;
	void OnDestroy() override;

	void OnStart() override;
	void OnUpdate() override;
	void OnStop() override;

	
	void Render();
	
	void Collison(const uint& instance, const Vector3& dir);

	void SetBodyBox(const uint& BodyIndex,const Matrix& BodyMatrix,const uint& boxIndex);
	

	void SetEffectBone(const int& effectIndex, const uint& particleIndex);

	void CreateBoneTransforms();
	class SharedData* GetSharedData( );
	void CS_CalcEffectTransform(const uint& paticleIndex);
public:
	void PlayNextClip(int instance, int clip, float time = 0.15f);
	void SetState(int instance,  const ActorState& state);

	bool IsEndAnimation(int instance, int clip);
	
	
private:
	void CreateAnimTransform(UINT index);
private:
	void CS_CalcBoneTransform();
	
private:
	
	struct BoneTransform
	{
		Matrix** Transform;
		BoneTransform()
		{
			Transform = new Matrix*[MAX_MODEL_KEYFRAMES];

			for (UINT i = 0; i < MAX_MODEL_KEYFRAMES; i++)
				Transform[i] = new Matrix[MAX_MODEL_TRANSFORMS];
		}
		~BoneTransform()
		{

			for (UINT i = 0; i < MAX_MODEL_KEYFRAMES; i++)
				SafeDeleteArray(Transform[i]);

			SafeDeleteArray(Transform);
		}
	};

	BoneTransform* skinTransforms;
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* srv;
	
	ID3DX11EffectShaderResourceVariable* sTransformSRV;
	ID3DX11EffectShaderResourceVariable* sCSTransformSRV;

private:

	
	bool bPause;
	float currentFrame = 0;
	uint meshCount;

private:
	
	
		struct AttachDesc
		{
			Matrix local;
			Matrix BoneScale;
			int Index = -1;
			Vector3 Pad;

			AttachDesc()
			{
				
				D3DXMatrixIdentity(&local);
				D3DXMatrixIdentity(&BoneScale);
				Index = -1;

				
			}
			
		};
		AttachDesc attachDesc[MAX_ACTOR_BONECOLLIDER];
		ConstantBuffer* computeAttachBuffer;
		ID3DX11EffectConstantBuffer* sComputeAttachBuffer;

		

		struct EffectAttachDesc
		{
			Matrix effectlocal;

			

			int EffectIndex = -1;
			
			float Padding[3];

		} ;
		EffectAttachDesc effectAttachDesc[MAX_ACTOR_BONECOLLIDER];
		ConstantBuffer* computeEffectAttachBuffer;
		ID3DX11EffectConstantBuffer* sComputeEffectAttachBuffer;
private:

		ID3DX11EffectConstantBuffer* sComputeFrameBuffer;


		ID3DX11EffectUnorderedAccessViewVariable* sUav;
		ID3DX11EffectUnorderedAccessViewVariable* sEffectUav;
		
private:
	
	Vector3 boneBoxMin = Vector3(-0.5f, -0.5f, -0.5f);
	Vector3 boneBoxMax = Vector3(0.5f, 0.5f, 0.5f);
		int skipCount=-1;

		INT64 currentTime;
		
		
		


		struct KeyframeDesc
		{
			int Clip = 0;
			UINT CurrFrame = 0;
			UINT NextFrame = 0;

			float Time = 0.0f;
			float RunningTime = 0.0f;


			Vector3 Padding;
		};

		struct TweenDesc
		{
			float TakeTime = 1.0f;
			float TweenTime = 0.0f; //0부터 1까지 a와b의 보간
			ActorState state = ActorState::Idle;
			uint IsEventActivated = 0;
			//bool IsDying = false;

			KeyframeDesc Curr;
			KeyframeDesc Next;

			TweenDesc()
			{
				Curr.Clip = 0;

				Next.Clip = -1;
			}
		};
		//TweenDesc tempTweens[MAX_MODEL_INSTANCE];
		vector< TweenDesc>tempTweens;
		TweenDesc tweens[MAX_MODEL_INSTANCE];
		//TweenDesc copyTweens[MAX_MODEL_INSTANCE];
		
	
		ConstantBuffer* frameBuffer;
		ID3DX11EffectConstantBuffer* sFrameBuffer;
		bool IsClacBoneBox = false;

		bool IsClacEffect = false;

private:
	Matrix S, R, T;
	Matrix animation;
	Matrix parent;
	Matrix invGlobal;

	ModelClip** clip;
	ModelBone** bone;
	Matrix* bones;

	int particleIndex;
	Vector3 nor;

	float timer = 0.0f;

	
};

