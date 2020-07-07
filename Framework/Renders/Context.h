#pragma once

#define MAX_CAPSULE_LIGHT 32



//
//struct CapsuleLight
//{
//	Color Ambient;
//	Color Diffuse;
//	Color Specular;
//
//	Vector3 Position;
//	float Rnage;
//
//	Vector3 Direction;
//	float Length;
//
//	float Intensity;
//	Vector3 Padding;
//
//	
//};
class Context
{
public:
	inline static Context* Get()
	{
		return instance;
	}
	static void Create();
	static void Delete();

private:
	explicit Context();
	~Context();

	
		Context(const Context&) = delete;
	Context& operator=(const Context&) = delete;

public:
	void Update();
	void Render();
	void ResizeScreen();

	void PushViewMatrix(const Matrix& matrix);
	void PushViewMatrix();

	inline const Matrix& View() {
		
		return mView; }
	inline const Matrix& Projection() { return mProj; }
	inline void SetMainCamera() { subCamera = NULL; }
	inline void SetSubCamera(Camera* camera) { subCamera = camera; }
	inline const Matrix& OrbitView()
	{
		cameras[1]->GetMatrix(&view);
		return view;
	}


	inline class Perspective* GetPerspective() { return perspective; }
	inline class Viewport* GetViewport() { return viewport; }
	inline class Camera* GetCamera() { return cameras[index]; }

	inline Color& LightAmbient() { return lightAmbient; }
	inline Color& LightSpecular() { return lightSpecular; }
	inline Vector3& LightDirection() { return lightDirection; }
	inline Vector3& LightPosition() { return lightPosition; }
 
	
	
	void SetShader(class Shader* shader);

	//inline Vector4* PlaneNormals(){ return planeNormals.data(); }


	void SetCameraIndex(uint index) { this->index = index; }
	void PushSubCamera(class Camera* camera) { cameras.emplace_back(camera); }
	uint GetCameraCount() { return cameras.size(); }
	uint GetCameraIndex() {	return cameras.size()-1; }
	void PopCamera(uint index);
private:
	static Context* instance;

private:
	class Perspective* perspective;
	class Viewport* viewport;
	

	Matrix projection;
	Matrix view;
	uint index;

	static bool bOrbit;

	Color lightAmbient;
	Color lightSpecular;
	Vector3 lightDirection;
	Vector3 lightPosition;

	vector<Camera*>cameras;
	class Camera* subCamera;
private:
	//D3DXVECTOR4 planeNormals[4];
	vector< D3DXVECTOR4>planeNormals;
	D3DXVECTOR3 cameraFrustum[4];
	D3DXVECTOR3 centerFar;
	D3DXVECTOR3 offsetH;
	D3DXVECTOR3 offsetV;

	D3DXVECTOR3 cameraPos;

	D3DXVECTOR3 worldRight;
	D3DXVECTOR3 worldUp;
	D3DXVECTOR3 worldForward;

	Matrix mView;
	Matrix mProj;

	//D3DXVECTOR3 normal;

	bool bUpdate = true;

private:

	struct BufferDesc
	{
		//Matrix View;
		
		//Matrix Projection;
		Matrix VP;
		//D3DXVECTOR4 FrustumNormals[4];

	}bufferDesc;

	ConstantBuffer* buffer=nullptr;
	ID3DX11EffectConstantBuffer* sBuffer;

	bool IsRender = false;
};