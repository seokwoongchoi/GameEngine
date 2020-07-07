#include "stdafx.h"
#include "Main.h"
#include "Systems/Window.h"
#include "Export.h"



void Main::Initialize()
{
	
  Push(new Export());
 // Push(new RenderModel());
	
}

void Main::Ready()
{

}

void Main::Destroy()
{
	for (IExecute* exe : executes)
	{
		exe->Destroy();
		SafeDelete(exe);
	}
}

void Main::Update()
{
	for (IExecute* exe : executes)
		exe->Update();
}


void Main::Render()
{
	for (IExecute* exe : executes)
		exe->Render();
}


void Main::ResizeScreen()
{
	for (IExecute* exe : executes)
		exe->ResizeScreen();
}

void Main::Push(IExecute * execute)
{
	executes.push_back(execute);

	execute->Initialize();
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR param, int command)
{
	D3DDesc desc;
	desc.AppName = L"D3D Game";
	desc.Instance = instance;
	desc.bFullScreen = false;
	desc.bVsync = true;
	desc.Handle = NULL;
	desc.Width = 1280;
	desc.Height = 720;
	desc.Background = D3DXCOLOR(0.3f, 0.3f, 0.3f, 1);
	D3D::SetDesc(desc);


	Main* main = new Main();
	WPARAM wParam = Window::Run(main);

	SafeDelete(main);

	return wParam;
}