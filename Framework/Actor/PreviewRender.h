#pragma once


class PreviewRender
{
public:
	explicit PreviewRender(class Shader* shader);
	~PreviewRender();
	
	
		PreviewRender(const PreviewRender&) = delete;
	PreviewRender& operator=(const PreviewRender&) = delete;
	void SetModel(class Model* model);
	void Update();
	
	void Render();
	void ForwardRender();
   	void DebugRender();

public:

	
	void SetColliderBoxIndex(const uint& colliderIndex, const uint& index)
	{
		boxCount= index+1;
		this->colliderBoxData[index].Index = colliderIndex;
	}

	
	void SetEffectIndex(const uint& effectIndex, const uint& index)
	{
		
		this->effectData[index].Index = effectIndex;
	}
	
	const Matrix& GetEffectBoneMatrix(const uint& index);
	

	//void SetEffectIndex(const uint& effectIndex, const uint& index)
	//{
	//	boxCount = index + 1;
	//	this->Index[index] = colliderIndex;
	//}
	void GetBoxWorld(Matrix* matrix, uint boxType); 
	void SetBoxWorld(const Matrix& matrix, uint boxType); 

	void GetBox(Vector3* min, Vector3* max);
	

	const Matrix& GetBox(const uint& index) { return colliderBoxData[index].ColliderBoxWorld; }
	

	const Matrix& GetSkinnedBoneTransform(const uint& index);
	
	
	
	void SetBoneTransform(const Matrix& matrix,const uint& index)
	{
		
		previewframe.matrix[index] = matrix;
	}


public:
	inline const float& GetPreviewFrame() {return previewframe.CurrFrame;}
	void StartAnimation(const bool& bPause) { this->bPause = bPause; }
	void SetFrame(float currentFrame) { this->currentFrame = currentFrame; }
	void SetClip(uint index) {
		previewframe.Clip = index;
	}
public:
	void SetBlend(bool bBlend) { this->bBlend = bBlend; }
	
	const Matrix& BlendModelBone(const uint& index);
public:
	Vector3  GetBoundsMax() { return BoundsMax; }
	Vector3  GetBoundsMin() {	return BoundsMin;}
	void OrbitProj(Matrix* matrix) { *matrix= orbitProj; }
	void OrbitView(Matrix* matrix) { *matrix = orbitView; }
	inline Perspective* PersPective() { return pers; }
	inline ID3D11ShaderResourceView* SRV() { return previewTarget->SRV()? previewTarget->SRV():nullptr; }
	Transform* PreviewTransform() {
		return previewTransform
			;
	}
private:
	float UpdatePreviewFrame();
	class Shader* shader;
	class Model* model;

private:
	Matrix orbitView;
	Matrix orbitProj;
	struct PreviewDesc
	{
		Matrix VP;
	}previewDesc;

	ConstantBuffer* previewbuffer;
	ID3DX11EffectConstantBuffer* sPreviewBuffer;

	float UpDownFactor = 0;
	float LeftRightFactor = 0;

	struct forwardDesc
	{
		Matrix matrix;
	};
	forwardDesc forward;
	ConstantBuffer* forwardBuffer;
	ID3DX11EffectConstantBuffer* sForwardBuffer;
private:
	class Orbit* orbit;
	class Perspective* pers;
	RenderTarget* previewTarget;
	class Transform* previewTransform;

	class Material* currentMat;

	bool bBlend;

private:
	struct previewFrameDesc
	{
		int Clip = 0;
		UINT CurrFrame = 0;
		UINT NextFrame = 0;

		float Time = 0.0f;
		float RunningTime = 0.0f;

		Vector3 Padding;

		Matrix matrix[MAX_MODEL_TRANSFORMS];
	};
	previewFrameDesc previewframe;
	Matrix bones[MAX_MODEL_TRANSFORMS];
	ConstantBuffer* previewFrameBuffer;
	ID3DX11EffectConstantBuffer* sPreviewFrameBuffer;

	bool bPause;
	float currentFrame = 0;
private:
	VertexBuffer* vertexBuffer;

	VertexColor* vertices;

	uint lineCount;
	void RenderLine(const Vector3& start,const Vector3& end,const Color& color);
	

private:

	void BoxRender(const Matrix& matrix, const Vector3& min, const Vector3& max,const Color& color);
	struct BufferDesc
	{
		Matrix World;
	} bufferDesc;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	
	

	

	Vector3 boneBoxMin = Vector3(-0.5f, -0.5f, -0.5f);
	Vector3 boneBoxMax = Vector3(0.5f, 0.5f, 0.5f);
	
	
	Vector3 BoundsMin=  Vector3(-0.5f, -0.5f, -0.5f);
	Vector3 BoundsMax = Vector3(0.5f, 0.5f, 0.5f);

	Matrix boxWorld;
	
	
private:

	
	uint effectIndex;
	Matrix effectBone;
private:

	struct ColliderBoxData
	{
		Matrix matrix;
		Matrix  R, T;
		Vector3 scale, position;
		Quaternion q;
		Matrix ColliderBoxWorld;
		Matrix result;
		uint Index;
	
	};
	int boxCount = -1;
	
	ColliderBoxData colliderBoxData[MAX_ACTOR_BONECOLLIDER];

private:
	struct EffectData
	{
		Matrix matrix;
		uint Index;

	};
	
	EffectData effectData[MAX_ACTOR_BONECOLLIDER];
		
private:
	ModelBone** bone;
	Texture* cameraTexture;
};

