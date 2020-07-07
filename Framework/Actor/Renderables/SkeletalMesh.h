#pragma once
#include "Renderable.h"

class SkeletalMesh final : public Renderable
{
public:
	SkeletalMesh(
		class Shader* shader = nullptr,
		class SharedData* sharedData = nullptr);
	~SkeletalMesh() = default;
	SkeletalMesh(const SkeletalMesh& rhs) = delete;
	SkeletalMesh& operator=(const SkeletalMesh& rhs) = delete;
public:

	inline void Pass(const uint& pass)override
	{
		pass == 0 ? this->pass = 2 : this->pass = 7;
	}
	void SetModel(class Model* model) override;
	void OnDestroy() override;
	void Render() override;
	void ForwardRender() override;

public:
	void BlendModelBone(const Matrix& matrix) { forward.matrix = matrix; }
	void CreateBoneTransforms();

	void SetAnimator(class Animator* animator) { this->animator = animator; }
private:
	class Animator* animator;
private:
	struct forwardDesc
	{
		Matrix matrix;
	};
	forwardDesc forward;
	ConstantBuffer* forwardBuffer;
	ID3DX11EffectConstantBuffer* sForwardBuffer;

	ID3D11ShaderResourceView** srv;

	//ID3DX11EffectShaderResourceVariable* sCollisonSrv;
	uint pass;
	uint meshCount;

};