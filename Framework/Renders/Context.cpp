#include "Framework.h"
#include "Context.h"
#include "Viewer/Viewport.h"
#include "Viewer/Perspective.h"
#include "Viewer/Freedom.h"
#include "Viewer/Orbit.h"


Context* Context::instance = NULL;

bool Context::bOrbit = true;
//Context * Context::Get()
//{
//	//assert(instance != NULL);
//
//	return instance;
//}

void Context::Create()
{
	assert(instance == NULL);

	instance = new Context();
}

void Context::Delete()
{
	SafeDelete(instance);
}

Context::Context()
{
	D3DDesc desc = D3D::GetDesc();

	perspective = new Perspective(desc.Width, desc.Height);
	viewport = new Viewport(desc.Width, desc.Height);
	
	Camera* main = new Freedom();
	main->SetCameraType(CameraType::Free);
	cameras.emplace_back(main);


	//orbit = new Orbit();
	//orbit->SetCameraType(CameraType::Orbit);
	//cameras.emplace_back(orbit);
	index = 0;
	
	
	lightAmbient = D3DXCOLOR(0, 0, 0, 1);
	lightSpecular = D3DXCOLOR(1, 1, 1, 1);
	lightDirection = D3DXVECTOR3(-1, -1, 1);
	lightPosition = D3DXVECTOR3(0, 0, 0);
	
	

	
	planeNormals.assign(4, Vector4(1,1,1,0));
	
}

Context::~Context()
{
	SafeDelete(perspective);
	SafeDelete(viewport);

	for(auto camera:cameras)
	SafeDelete(camera);
}

void Context::Update()
{
	
	cameras[index]->Update();

	cameras[index]->GetMatrix(&mView);
	perspective->GetMatrix(&mProj);

    

	//bufferDesc.Projection = mProj;
	D3DXMatrixMultiply(&bufferDesc.VP, &mView, &mProj);
	if (IsRender)
	{
		buffer->Apply();
		sBuffer->SetConstantBuffer(buffer->Buffer());


	}

	//if (Keyboard::Get()->Down('G'))
	//{
	//	bUpdate ? bUpdate = false : bUpdate = true;
	//}
	//if (bUpdate)
	//{
	//	

	//	worldRight = D3DXVECTOR3(mView._11, mView._21, mView._31);
	//	worldUp = D3DXVECTOR3(mView._12, mView._22, mView._32);
	//	worldForward = D3DXVECTOR3(mView._13, mView._23, mView._33);

	//	float clipNear = -mProj._43 / mProj._33;
	//	float clipFar = clipNear * mProj._33 / (mProj._33 - 1.0f);

	//	cameras[index]->Position(&cameraPos);

	//	centerFar = cameraPos + worldForward * clipFar;
	//	offsetH = (clipFar / mProj._11) * worldRight;
	//	//static float t = 0.0f;
	//  // ImGui::SliderFloat("fov", (float*)&t, -100.0f, 100.0f);
	//
	//	offsetV = (clipFar / mProj._22) * worldUp;

	//	cameraFrustum[0] = centerFar - offsetV - offsetH;
	//	cameraFrustum[1] = centerFar + offsetV - offsetH;
	//	cameraFrustum[2] = centerFar + offsetV + offsetH;
	//	cameraFrustum[3] = centerFar - offsetV + offsetH;

	//	// left/top planes normals
	//	D3DXVECTOR3 normal;
	//	D3DXVECTOR3 temp = cameraFrustum[1] - cameraPos;
	//	D3DXVec3Cross(&normal, &temp, &worldUp);
	//	D3DXVec3Normalize((D3DXVECTOR3*)&planeNormals[0], &normal);
	//	D3DXVec3Cross(&normal, &temp, &worldRight);
	//	D3DXVec3Normalize((D3DXVECTOR3*)&planeNormals[1], &normal);

	//	// right/bottom planes normals
	//	temp = cameraFrustum[3] - cameraPos;
	//	D3DXVec3Cross(&normal, &worldUp, &temp);
	//	D3DXVec3Normalize((D3DXVECTOR3*)&planeNormals[2], &normal);
	//	D3DXVec3Cross(&normal, &worldRight, &temp);
	//	D3DXVec3Normalize((D3DXVECTOR3*)&planeNormals[3], &normal);

	//	bufferDesc.FrustumNormals[0] = planeNormals[0];
	//	bufferDesc.FrustumNormals[1] = planeNormals[1];
	//	bufferDesc.FrustumNormals[2] = planeNormals[2];
	//	bufferDesc.FrustumNormals[3] = planeNormals[3];
	//}
	
		
}

void Context::Render()
{
	
	/*ImGui::Checkbox("Orbit", &bOrbit);
	bOrbit ? index = 1 : index = 0;*/
	viewport->RSSetViewport();
	SetMainCamera();
	string str = string("Frame Rate : ") + to_string(ImGui::GetIO().Framerate);
	Gui::Get()->RenderText(5, 20, 1, 1, 1, str);

	D3DXVECTOR3 camPos;
	Context::Get()->GetCamera()->Position(&camPos);

	D3DXVECTOR3 camDir;
	Context::Get()->GetCamera()->RotationDegree(&camDir);

	str = "Cam Position : ";
	str += to_string((int)camPos.x) + ", " + to_string((int)camPos.y) + ", " + to_string((int)camPos.z);
	Gui::Get()->RenderText(5, 35, 1, 1, 1, str);

	str = "Cam Rotation : ";
	str += to_string((int)camDir.x) + ", " + to_string((int)camDir.y);
	Gui::Get()->RenderText(5, 50, 1, 1, 1, str);


}

void Context::ResizeScreen()
{
	static_cast<Perspective*>(perspective)->Set(D3D::Width(), D3D::Height());
	viewport->Set(D3D::Width(), D3D::Height());
}

void Context::PushViewMatrix(const Matrix & matrix)
{
	perspective->GetMatrix(&mProj);
	
	D3DXMatrixMultiply(&bufferDesc.VP, &matrix, &mProj);

	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());
}

void Context::PushViewMatrix()
{
	cameras[index]->Update();

	cameras[index]->GetMatrix(&mView);
	perspective->GetMatrix(&mProj);
	
	D3DXMatrixMultiply(&bufferDesc.VP, &mView, &mProj);

	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());
}






void Context::SetShader(Shader * shader)
{
	
	buffer = new ConstantBuffer(&bufferDesc, sizeof(BufferDesc));
	sBuffer = shader->AsConstantBuffer("CB_PerFrame");

	
	IsRender = true;
}

void Context::PopCamera(uint index)
{
	cameras.erase(cameras.begin()+index);
	cameras.shrink_to_fit();
	/*uint camIndex = 0;
	for (auto& iter = cameras.begin(); iter != cameras.end();)
	{
		
		auto type = *iter;
		if (camIndex ==index)
		{

			iter = 

		}
		else
			iter++;

		index++;
	}*/
}





