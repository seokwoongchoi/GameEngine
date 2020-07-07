#include "Framework.h"
#include "ActorCamera.h"
#include "Viewer/Orbit.h"

ActorCamera::ActorCamera(Shader* shader, Model* model, SharedData* sharedData)
    : Component(shader,model, sharedData)
	, bFire(false), velocity(0.0f)
{
	orbit = new Orbit();
	orbit->SetCameraType(CameraType::Orbit);
	Context::Get()->PushSubCamera(orbit);
	camIndex=Context::Get()->GetCameraIndex();
	aim = new Texture(L"cross_aim.png");
	desc = D3D::GetDesc();
    actorIndex = sharedData->actorIndex;
}

void ActorCamera::OnDestroy()
{
	SafeDelete(aim);
	Context::Get()->PopCamera(camIndex);
}

void ActorCamera::OnStart()
{
	
	Context::Get()->SetCameraIndex(1);
	m_pt.x = desc.Width / 2;
	m_pt.y = desc.Height / 2;
	ClientToScreen(desc.Handle, &m_pt);
	SetCursorPos(m_pt.x, m_pt.y);
	ShowCursor(false);
	bStart = true;

}

void ActorCamera::OnUpdate()
{
	if (!bStart) return;

	Gui::Get()->RenderTexture(D3DXVECTOR2(640, 400), D3DXVECTOR2(40.0f, 40.0f), Color(0, 1, 0, 1), aim);

	
	ActorCommand();
	
}


void ActorCamera::OnStop()
{
	ShowCursor(true);
	bStart = false;
	Context::Get()->SetCameraIndex(0);
}

void ActorCamera::ActorCommand()
{
	
	if (Keyboard::Get()->Down(27))
	{
		bPause ? bPause = false : bPause = true;
		ShowCursor(bPause);
	}
	
	if (bPause)
	{
		orbit->SetMoveValue(Vector2(0.0f,0.0f));
		return;
	}

	Forward = sharedData->GetForward(0);
	//Up = Vector3(sharedData->transforms[0]._21, sharedData->transforms[0]._22, sharedData->transforms[0]._23);
	Right = sharedData->GetRight(0);

	POINT point;
	GetCursorPos(&point);

	//ScreenToClient(desc.Handle, &point);
	moveValue.x = point.x - m_pt.x;
	moveValue.y = point.y - m_pt.y;
	orbit->SetMoveValue(moveValue);
	rotation.y += (moveValue.x*0.3f)*Time::Delta();

	SetCursorPos(m_pt.x, m_pt.y);


	D3DXMatrixDecompose(&s, &q, &p, &sharedData->transforms[0]);
	D3DXMatrixScaling(&S, s.x, s.y, s.z);

	D3DXMatrixRotationY(&R, rotation.y);

	prevPosition = p;
	position = p;
	if (Keyboard::Get()->Press('W'))
	{

		position -= 200.0f*Forward* Time::Delta();

	}
	else if (Keyboard::Get()->Press('S'))
	{

		position += 200.0f*Forward* Time::Delta();

	}

	if (Keyboard::Get()->Press('A'))
	{


		position += 200.0f*Right* Time::Delta();

	}
	else if (Keyboard::Get()->Press('D'))
	{

		position -= 200.0f*Right* Time::Delta();


	}
	velocity = D3DXVec3Length(&(position - prevPosition));
	if (Mouse::Get()->Press(0))
	{
		bFire = true;

	}
	if (velocity > 0.0f && !bFire)
	{
		EventSystem::Get()->Events["Move"](actorIndex, 0);
		ColliderSystem::Get()->SetMainActorState(MainActorState::Move);
	}

	else if (velocity > 0.0f&&bFire)
	{
		EventSystem::Get()->Events["WalkFire"](actorIndex, 0);
		ColliderSystem::Get()->SetMainActorState(MainActorState::WalkFire);
		bFire = false;
	}

	else if (bFire)
	{

		EventSystem::Get()->Events["Fire"](actorIndex, 0);
		ColliderSystem::Get()->SetMainActorState(MainActorState::Fire);
		bFire = false;


	}
	else if (velocity == 0.0f && !bFire)
	{
		EventSystem::Get()->Events["Idle"](actorIndex, 0);
		ColliderSystem::Get()->SetMainActorState(MainActorState::Idle);
	}
	orbit->SetOrbitTargetPosition(Vector3(position.x,position.y+10,position.z));
	D3DXMatrixTranslation(&T, position.x, position.y, position.z);
	sharedData->transforms[0] = S * R*T;

}

