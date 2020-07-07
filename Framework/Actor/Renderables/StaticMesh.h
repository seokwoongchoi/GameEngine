#pragma once
#include "Renderable.h"
class StaticMesh : public Renderable
{
public:
	StaticMesh(
		class Shader* shader = nullptr,

		class SharedData* sharedData = nullptr);
	~StaticMesh() = default;
	StaticMesh(const StaticMesh& rhs) = delete;
	StaticMesh& operator=(const StaticMesh& rhs) = delete;
	inline void Pass(const uint& pass)override
	{
		pass == 0 ? this->pass = 1 : this->pass = 6;
	}
	void SetModel(class Model* model) override;
	void OnDestroy() override;
	void Render() override;
	void ForwardRender() override;

public:
	void BlendModelBone(const Matrix& matrix);
	void CreateBoneTransforms();

private:
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* srv;

	ID3DX11EffectShaderResourceVariable* sTransformSRV;
private:
	struct forwardDesc
	{
		Matrix matrix;
	};
	forwardDesc forward;
	ConstantBuffer* forwardBuffer;
	ID3DX11EffectConstantBuffer* sForwardBuffer;



	uint pass;
	uint meshCount;

	ModelMesh* forwardMesh;
	uint forwardCount;
};
