#pragma once
#define MAX_PATH_STR 512
struct BodyData {
	unsigned int nBodies=0;
	float       *position=nullptr;
	float       *velocity = nullptr;

	
};


class ParticleSimulation
{
public:
	explicit ParticleSimulation(const uint& index);
	~ParticleSimulation();

	
	
	ParticleSimulation(const ParticleSimulation&) = delete;
	ParticleSimulation& operator=(const ParticleSimulation&) = delete;
	void Destroy();

	void Update();
	void PreviewRender();
	void Render();

	void ResetBodies(const int & instance);
	void SetPointSize(float size) { m_fPointSize = size; }
	
	void ImageButton();

	void ResidenceSharedData(class SharedData* sharedData);

	inline ID3D11UnorderedAccessView * UAV() {
		return Position_StructuredBufferUAV;
	}

	

private:
	void CreateComputeData();
	struct CS_TextureOutputDesc
	{
		Matrix matrix;

	};
	CS_TextureOutputDesc* csTexture = nullptr;
	
	ID3D11Buffer	  *m_pStructuredBuffer;
	ID3D11ShaderResourceView  *m_pStructuredBufferSRV;
	ID3D11UnorderedAccessView *m_pStructuredBufferUAV;

private:
	void Editor();
	void ShowFrame(const ImVec2& size);
	void SelectParticleType(const ImVec2 & size);
	void LoadTexture();
	void SetTexture(const wstring& file);
	
private:
	void Boom_InitBodies(uint numBodies);

	void GunFire_InitBodies(uint numBodies);

	void Blood_InitBodies(uint numBodies);


private:
	class Shader* shader;
	class Shader* csShader;
	BodyData bodyData;
private:
	uint    m_numBodies;
	float   m_fPointSize;
	uint    m_readBuffer;

	uint Count;


private:



	// structured buffer
	
	ID3D11Texture2D	  *Position_StructuredBuffer;
	ID3D11ShaderResourceView  *Position_StructuredBufferSRV;
	ID3D11UnorderedAccessView *Position_StructuredBufferUAV;

private:
	struct SimulationParametersCB
	{
		float g_timestep;
		float g_softeningSquared=0.01f;
		unsigned int g_numParticles;
		unsigned int g_readOffset;
		unsigned int g_writeOffset;
		float distance=20.0f;
		float dummy[2];
	};

	SimulationParametersCB simulateCB;

	ConstantBuffer* simulateBuffer;
	ID3DX11EffectConstantBuffer* sSimulateBuffer;

private:
	struct DrawCB
	{
		D3DXMATRIX   ViewProjection;
		float		 g_fPointSize;
		unsigned int g_readOffset;
		float        dummy[2];
	};

	DrawCB drawCB;

	ConstantBuffer* drawBuffer;
	ID3DX11EffectConstantBuffer* sDrawBuffer;
	

private:
	ID3DX11EffectUnorderedAccessViewVariable* uavEffect;
	ID3DX11EffectShaderResourceVariable* textureEffect;
	ID3DX11EffectShaderResourceVariable* srvEffect;

	ID3DX11EffectShaderResourceVariable* boneEffect;

private:
	Matrix orbitView;
	Matrix orbitProj;
	struct PreviewDesc
	{
		Matrix VP;
	}previewDesc;

	ConstantBuffer* previewbuffer;
	ID3DX11EffectConstantBuffer* sPreviewBuffer;

	class Orbit* orbit;
	class Perspective* pers;
	RenderTarget* previewTarget;
	class Transform* previewTransform;

	bool bEditing;
	bool bActive;

	Texture* particleTexture;
	float clusterScale = 1.0f;
	float velocityScale = 1.0f;
	uint pass=0;
	bool bPause;
	Texture* buttonTextures[3];

	float timer = 1.0f;

	float runningTime = 0.0f;
	bool bFirst = true;

private:
	VertexBuffer* instanceBuffer;

	class SharedData* sharedData;

	struct InstDesc
	{
		Matrix worlds;
		Vector4 factor;
	};

	InstDesc instDesc[MAX_MODEL_INSTANCE];

	vector<uint> indices;
	uint instanceIndex;

	uint index;
};
