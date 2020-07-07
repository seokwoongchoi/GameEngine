#pragma once


#include "Components/Component.h"
#include "Renderables/Renderable.h"

enum class GizmoType :uint
{
	Default,
	Bone,
	Box,
	Box1,
	Box2,
	Effect1,
	Effect2,
	ActorCamera
};
enum class ComponentType :uint
{
	ActorCollider,
	ActorCamera,
	ActorLight,
	Animator,
	ActorAi

};
enum class RenderableType :uint
{
	StaticMesh,
	SkeletalMesh

};

enum class EditMode :uint
{
	Animator,
	Render,

};

class Actor
{
public:
	explicit Actor(class Shader* shader);
	~Actor();

	Actor(const Actor&) = delete;
	Actor& operator=(const Actor&) = delete;

public:
	void Save();
	
	void Load();
	void LoadComplete();
public:
	void Start();
	void Stop();
	void Compile();
	inline void Pass(const uint& pass) { this->pass = pass; }

	void  ActorPos();

	void SetTerrain(class TerrainLOD* terrain)
	{
		this->terrain = terrain;
	}
	
	void Update();
	void Render();
	
	void PrevRender();
	void ForwardRender();

	void ImageButton();

	void SetIndex(const uint& index)
	{
		sharedData->actorIndex = index;
		cout << to_string(index) << endl;
	}

	Transform* PreviewTreansform();
	
private:

	void EditingMode();
	void Editor();
	void ShowFrame(const ImVec2 & size);
	GizmoType gizmoType;
	void ImGizmo(const ImVec2 & size);
	
	void ShowComponents(const ImVec2 & size);

	void ShowComponentPopUp();
	void ShowComponentListPopUp(string componentName);

	void CreateWeaponMesh();
	void LoadWeapon(wstring name);
	void LoadSkeletalMesh(wstring& file);
	void LoadMaterial(function<void(wstring, uint, Material*)> f, uint num, Material* material);
	void SetMaterial(wstring& file, uint textureType, Material* material);
	void LoadStaticMesh(wstring& file);
	void CreateColliderComponent();
	void CreateActorAI();
	void BehaviorTree(const uint& num);

	
	void ShowMaterial(const ImVec2 & size);
	void ShowHierarchy();
	
	void ShowBone(ModelBone* bone);
	void ShowChild(ModelBone * bone); 
	void ShowPopup();
	void BlendMesh();
	void CreateBox();
	void CreateBox(const uint& index);
	
	
	void SetDragDropPayload(const string& data);
private:
	void CreateActorCamera();
private:
	void CreateEffect(const uint& index);

	
private:
	void ShowAnimList(const ImVec2 & size);
	void ShowAnimFrame(const ImVec2 & size);
	void ClipFinder(wstring file);
	vector<string>clipList;
public:
	void AddTransform(const Matrix& transform);
	
	void UpdateTransforms();

public:
	inline const bool& IsDragged() const { return bDrag; }
	void SetIsDragged(const bool& bDrag)  {  this->bDrag=bDrag; }

	const std::string& GetName() const { return name; }
	void SetName(const std::string& name) { this->name = name; }
	
	vector<string>componentList;

private:
	class PreviewRender* previewRender;
	class Shader* shader;
	class Model* model;
	class TerrainLOD* terrain;
	class ModelBone* currentBone;
	

private:
	vector<ComponentType> componentTypeList;
	ComponentType componentType;
	unordered_map<ComponentType, class Component*>components;
	
	class Renderable*  renderable;
	RenderableType renderableType;
	//unordered_map<RenderableType, class Renderable*>renderables;
	//vector<class Renderabcle*> renderables;
	
	wstring modelName = L"";
	string currentClip = "";
	string componentName = "N/A";
	int currentClipnum;
	
	float frame;

	
	EditMode mode;
	uint pass;
private:
	VertexBuffer* instanceBuffer;
	//std::vector<class Transform*>transforms;
	Matrix worlds[MAX_MODEL_INSTANCE];
	
private:
	Texture* buttonTextures[3];
	
	std::string name;
	bool bActive;
	bool bEditing;
    bool bFirst;
	bool bDrag;
	bool bLoaded;
	bool bModelLoaded;
	bool bBone;
	bool bBlend;
	bool bCompiled;

	bool hasEffect;
	
private:
	
	class SharedData* sharedData;

private:
	Matrix orbitView;
	Matrix orbitProj;

	
	Matrix matrix;
	Matrix world;
	Matrix parent;

	Matrix parentInv;
	

	Matrix iden;
	Matrix local;

	Vector3 snap = Vector3(0.001f, 0.001f, 0.001f);
	D3D11_MAPPED_SUBRESOURCE subResource;
	Matrix cameraWorld;
	uint index=0;

	
	Matrix invLocal;
	int boxCount = -1;
	int particleCount = -1;

	
	uint colliderIndex[2];

	struct EffectData
	{
		int effectIndex=-2;
		int particleIndex = -1;
		Matrix effectWorld;
	};
	

	EffectData effectData[MAX_ACTOR_BONECOLLIDER];
	//int particleIndex[2];
	
	
	bool IsBindedTree=false;
	
	
};

