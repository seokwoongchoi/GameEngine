#pragma once
class PerFrame
{
public:
	PerFrame(Shader* shader);
	~PerFrame();

public:
	void Update();
	void Render(Vector3 Pos=Vector3(0,0,0));
private:
	struct BufferDesc
	{
		
		Matrix VP;
		
	}bufferDesc;

		


	
	//struct CapsuleLightDesc
	//{
	//	UINT Count = 0;
	//	float Padding[3];
	//
	//	CapsuleLight Lights[MAX_CAPSULE_LIGHT];
	//} capsuleLightDesc;
private:
	Shader* shader;
	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	
	

	//ConstantBuffer* spotLightBuffer;
	//ID3DX11EffectConstantBuffer* sSpotLightBuffer;

	//ConstantBuffer* capsuleLightBuffer;
	//ID3DX11EffectConstantBuffer* sCapsuleLightBuffer;
	//D3DXVECTOR4* planeNormals;
};