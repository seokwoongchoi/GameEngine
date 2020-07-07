#pragma once
#include "Systems/IExecute.h"

#define MAX_OBJECT_COUNT 14



class EditorMain : public IExecute
{
	
public:
	virtual void Initialize() override;
	virtual void Ready() override {}
	virtual void Destroy() override;
	virtual void Update() override;

	virtual void Render() override;
	
	virtual void ResizeScreen() override {}

	void MenuBar();
	void ToolBar();
	void DeleteActor();

	void Save(wstring fileName);
	void Load(wstring fileName);
private:
	uint frameCount=0;
    void Assets();
	void ShowPopUp(bool bAsset);

	void AddTrasform(const Vector3& pos);
private:
	void CreateHDRRTV(uint width, uint height);
	void CreateRender2DTarget(uint width, uint height);
	void SetMaterial();
	void CreateObjects();
	
	void Depth_PreRender();
	void Reflection_PreRender();
	void GbufferPacking();
	void GbufferUnPacking();
	
private:

	Shader* deferredShader;
	DeferredDirLight* dirLight;
	DeferredPointLight* pointLight;
	DeferredSpotLight* spotLight;
private:
	class LightObjects* lights;
	
	
private:
	ID3D11ShaderResourceView* skyIRSRV;
	ID3DX11EffectShaderResourceVariable* sSkyMap;
	ID3DX11EffectShaderResourceVariable* sPreviewSkyMap;
	ID3DX11EffectShaderResourceVariable* sBRDF;
	
	
	class Island11* island11;

	//class  Fixity* reflectionCam;
	//uint camIndex;
	

private://Env
	class TerrainLOD* terrain;
	
	class OceanRenderer* oceanRenderer;

	class Fog* fog;
	ID3D11SamplerState* samplerState;
	Texture* preintegratedFG;
	
	class PreFilter* prefilter;
	class Atmosphere* sky;
	class Cloud* cloud;
	
private://post& ss
	class SSAO* ssao;
	class SSLR* sslr;
	
	class HDR* hdr;
	//class HDRBloom* hdr;
	Matrix T;
	vector<Renderer*>actor;

	vector <Matrix*>transforms;
	//포팅 마이그레이션

	
private:
	
	Render2D* diffuse;
	Render2D* normal;
	Render2D* specular;
	Render2D* bump;

	
	ID3D11DepthStencilView* mainDSV;
	ID3D11ShaderResourceView* depthSRV;
	
	class Texture* buttonTextures[3];
	class Texture* behaviorTreeTexture;
		
private:
	uint skyPass;
	uint meshPass;
	uint modelPass;
	uint animPass;
	uint finalPass;
	uint terrainPass;
	
	
private:
	float pointRange;
	Vector3 pointPosition;
	Vector3 position, rotation;
private:
	//Material* floor;
	Material* stone;
	Material* brick;
	Material* wall;
	

	MeshSphere* sphere[MAX_OBJECT_COUNT];

	Material* mat[8];
	MeshSphere* materialSphere[8];


	MeshCylinder* cylinder[MAX_OBJECT_COUNT];
	MeshCube* cube;
//	vector< MeshCube*>gizmos;
	//MeshGrid* grid;
		

	 
private:
	// HDR light accumulation buffer
	ID3D11Texture2D* g_pHDRTexture = NULL;
	ID3D11RenderTargetView* g_HDRRTV = NULL;
	ID3D11ShaderResourceView* g_HDRSRV = NULL;

	ID3D11RenderTargetView* old_target;
	ID3D11DepthStencilView* old_depth;
	D3D11_VIEWPORT oldvp;
private:

	class ProgressBar* progress;
private:

	//unordered_map<ActorType, std::vector<class IActor*>> actors;
	
   // class Transform* selectT;
	
	vector<class Actor*>actors;
	private:

	/*Texture* buttonTextures[3];*/

	bool bStart;
	bool bPause;

	
	Vector3 pos;
	uint actorIndex;
	//PerFrame* perFrame;
	 bool bFirst = true;

   class TextureCube* cubeMap;
   //RenderTarget* rtvs[4];
   float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
   uint count;
   bool bShowBehaviorTree = false;

   bool bCreatedBehaviorTree = false;

   bool bShowMetricsWindow=false;
   bool bShowStyleEditor = false;
   bool bShowDemoWindow = false;
   bool bShowScriptTool = false;
   wstring fileName;
   ID3D11Predicate* predicate;
};