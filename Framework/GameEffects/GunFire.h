#pragma once
#include "Effect.h"

struct EffectPosition
{
	Vector3 position;
	
};
class GunFire:public Effect
{
public:
	GunFire();
	~GunFire();

	void Update()override;
	void Render()override;
	void PreviewRender()override;

	void SetWorld(const Matrix& matrix)
	{
		world= matrix;
		
	}
	void SetView(const Matrix& matrix)
	{
		//D3DXMatrixInverse(&previewDesc.View, nullptr, &matrix);
		bufferDesc.VP = matrix;
	}
private:
	Texture* effectTexture;
private:
	ID3DX11EffectShaderResourceVariable* textureEffect;
	EffectPosition vertex;
	vector< EffectPosition>vertices;

private:
	struct BufferDesc
	{
		Matrix World;
		Matrix VP;
		
	} bufferDesc;


	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;
private:

	Matrix world;
	float rotation;
	INT64 currentTime;
	class PerFrame* preframe;
};

