#pragma once
enum class LightType :uint
{
	unKnown,
	pointLight,
	spotLight,
	capsuleLight,
};
class LightObjects
{
public:
	explicit LightObjects(class DeferredPointLight* pointLight, class DeferredSpotLight* spotLight);
	~LightObjects();


	
		LightObjects(const LightObjects&) = delete;
	LightObjects& operator=(const LightObjects&) = delete;
	void Update(Vector3& mousePos);
	void Render();

	void SetStart(bool bStart)
	{
		this->bStart = bStart;
	}
	void PointLightControll(int index);
	void SpotLightControll(int index);
	void CapsuleLightControll(int index);

	void PointLightRender();
	void SpotLightRender();
	void CapsuleLightRender();

	void GridPointLight(Vector3& pos, float& Range, Color color);
	void GridSpotLight(uint index, Color color);
	void GridCapsuleLight(uint index, Color color);
	void SetDragDropPayload(const LightType& type, const string& data);


	bool CheckIsPicked(Vector3& proj, Vector3& mousePos, LightType type);

	
private:
	class DeferredPointLight* pointLight;
	class DeferredSpotLight* spotLight;
		
	LightType lightType;

	Texture* pointTexture;
	Texture* spotTexture;
	Texture* capsuleTexture;

	wstring dragLightName;



	Matrix world;
	Matrix V, P;
	Vector3 mousePos;
	Vector3 xy[360];
	Vector3 xz[360];
	Vector3 yz[360];
	
	int pointIndex;
	int spotIndex;
	int capsuleIndex;


	bool bDrag;
	
	bool bStart = false;
};

