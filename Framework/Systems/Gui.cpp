#include "Framework.h"
#include "Gui.h"
#include"ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
//#include "Widget.h"

Gui* Gui::instance = NULL;

void Gui::Create()
{
	assert(instance == NULL);

	instance = new Gui();
}

void Gui::Delete()
{
	SafeDelete(instance);
}

Gui * Gui::Get()
{
	return instance;
}
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT Gui::MsgProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	return ImGui_ImplWin32_WndProcHandler(handle, message, wParam, lParam);
}

void Gui::Resize()
{
	ImGui_ImplDX11_InvalidateDeviceObjects();
	ImGui_ImplDX11_CreateDeviceObjects();
}

void Gui::Update()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void Gui::Render()
{
	
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(1280, 720));

	ImGui::SetNextWindowBgAlpha(0.0f);

	ImGui::Begin
	(
		"TextWindow", NULL,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoNavFocus
	);
	
	
	
	for (GuiText text : texts)
	{
		ImVec2 position = ImVec2(text.Position.x, text.Position.y);
		ImColor color = ImColor(text.Color.r, text.Color.g, text.Color.b, text.Color.a);

		ImGui::GetWindowDrawList()->AddText(position, color, text.Content.c_str());
		
		
	}
	texts.clear();

	for (GuiTexture gTexture : textures)
	{
		ImVec2 position = ImVec2(gTexture.Position.x, gTexture.Position.y);
		ImVec2 size = ImVec2(gTexture.scale.x, gTexture.scale.y);
		ImColor color = ImColor(gTexture.color.r, gTexture.color.g, gTexture.color.b, gTexture.color.a);
		

		ImGui::GetWindowDrawList()->AddImage(gTexture.texture?gTexture.texture->SRV():nullptr, ImVec2(position.x - size.x / 2, position.y - size.y / 2),
			ImVec2(position.x + size.x / 2, position.y + size.y / 2),ImVec2(0,0), ImVec2(1, 1), color);


	}
	textures.clear();
	
	
	if(controlTransform)
	ShowTransformGizmo(controlTransform);
	//transforms.clear();
	//DockingPannel();
	ImGui::End();
	/*for (Widget* widget : widgets)
	{
	if (widget->Visible() == true)
	{
	widget->Begin();
	widget->Render();
	widget->End();
	}
	}*/

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	const	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void Gui::AddWidget(Widget * widget)
{
	widgets.push_back(widget);
}

void Gui::RenderText(GuiText & text)
{
	texts.push_back(text);
}



void Gui::RenderText(float x, float y, string content)
{
	GuiText text;
	text.Position = D3DXVECTOR2(x, y);
	text.Color = D3DXCOLOR(1, 1, 1, 1);
	text.Content = content;

	RenderText(text);
}

void Gui::RenderText(float x, float y, float r, float g, float b, string content)
{
	GuiText text;
	text.Position = D3DXVECTOR2(x, y);
	text.Color = D3DXCOLOR(r, g, b, 1);
	text.Content = content;

	RenderText(text);
}

void Gui::RenderText(D3DXVECTOR2 position, D3DXCOLOR color, string content)
{
	GuiText text;
	text.Position = position;
	text.Color = color;
	text.Content = content;

	RenderText(text);


}


void Gui::RenderTexture(D3DXVECTOR2 position, D3DXVECTOR2 scale,Color color, Texture * texture)
{
	GuiTexture gTexture;
	gTexture.Position = position;
	gTexture.scale = scale;
	gTexture.color = color;
	gTexture.texture = texture;
	

	textures.emplace_back(gTexture);

}



void Gui::ShowTransformGizmo(Matrix * transform)
{
	static ImGuizmo::OPERATION operation(ImGuizmo::TRANSLATE);
	static ImGuizmo::MODE mode(ImGuizmo::WORLD);

	if (ImGui::IsKeyPressed('T'))//w
		operation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed('Y'))//e
		operation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed('U'))//r
		operation = ImGuizmo::SCALE;

	


	const	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	ImGuizmo::Manipulate(Context::Get()->View(), Context::Get()->Projection(), operation,
		mode,
		*transform);
	/*D3DXVECTOR3 position, scale;
	D3DXQUATERNION q;
	D3DXMatrixDecompose(&scale, &q, &position, &matrix);*/
	//transform->World(matrix);
	
	/*transform->Position(position);
	transform->Rotation(q);
	transform->Scale(scale);*/
}




void Gui::AddTransform(Matrix * transform)
{
	//transforms.emplace_back(transform);
	controlTransform = transform;
}





Gui::Gui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	
	D3DDesc desc = D3D::GetDesc();

	ImGui_ImplWin32_Init(desc.Handle);
	ImGui_ImplDX11_Init(D3D::GetDevice(), D3D::GetDC());

	ApplyStyle();

}

Gui::~Gui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Gui::ApplyStyle()
{
	ImGui::GetIO().ConfigWindowsResizeFromEdges = true;
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();


	float fontSize = 15.0f;
	float roundness = 2.0f;
	ImVec4 white = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	ImVec4 text = ImVec4(0.76f, 0.77f, 0.8f, 1.0f);
	ImVec4 black = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	ImVec4 backgroundVeryDark = ImVec4(0.08f, 0.086f, 0.094f, 1.00f);
	ImVec4 backgroundDark = ImVec4(0.117f, 0.121f, 0.145f, 1.00f);
	ImVec4 backgroundMedium = ImVec4(0.26f, 0.26f, 0.27f, 1.0f);
	ImVec4 backgroundLight = ImVec4(0.37f, 0.38f, 0.39f, 1.0f);
	ImVec4 highlightBlue = ImVec4(0.172f, 0.239f, 0.341f, 1.0f);
	ImVec4 highlightBlueHovered = ImVec4(0.202f, 0.269f, 0.391f, 1.0f);
	ImVec4 highlightBlueActive = ImVec4(0.382f, 0.449f, 0.561f, 1.0f);
	ImVec4 barBackground = ImVec4(0.078f, 0.082f, 0.09f, 1.0f);
	ImVec4 bar = ImVec4(0.164f, 0.180f, 0.231f, 1.0f);
	ImVec4 barHovered = ImVec4(0.411f, 0.411f, 0.411f, 1.0f);
	ImVec4 barActive = ImVec4(0.337f, 0.337f, 0.368f, 1.0f);

	// Spatial
	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 1.0f;
	style.FramePadding = ImVec2(5, 5);
	style.ItemSpacing = ImVec2(6, 5);
	style.Alpha = 1.0f;
	style.WindowRounding = roundness;
	style.FrameRounding = roundness;
	style.PopupRounding = roundness;
	style.GrabRounding = roundness;
	style.ScrollbarSize = 20.0f;
	style.ScrollbarRounding = roundness;

	// Colors
	style.Colors[ImGuiCol_Text] = text;
	style.Colors[ImGuiCol_WindowBg] = backgroundDark;
	style.Colors[ImGuiCol_Border] = black;
	style.Colors[ImGuiCol_FrameBg] = bar;
	style.Colors[ImGuiCol_FrameBgHovered] = highlightBlue;
	style.Colors[ImGuiCol_FrameBgActive] = highlightBlueHovered;
	style.Colors[ImGuiCol_TitleBg] = backgroundVeryDark;
	style.Colors[ImGuiCol_TitleBgActive] = bar;
	style.Colors[ImGuiCol_MenuBarBg] = backgroundVeryDark;
	style.Colors[ImGuiCol_ScrollbarBg] = barBackground;
	style.Colors[ImGuiCol_ScrollbarGrab] = bar;
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = barHovered;
	style.Colors[ImGuiCol_ScrollbarGrabActive] = barActive;
	style.Colors[ImGuiCol_CheckMark] = white;
	style.Colors[ImGuiCol_SliderGrab] = bar;
	style.Colors[ImGuiCol_SliderGrabActive] = barActive;
	style.Colors[ImGuiCol_Button] = barActive;
	style.Colors[ImGuiCol_ButtonHovered] = highlightBlue;
	style.Colors[ImGuiCol_ButtonActive] = highlightBlueActive;
	style.Colors[ImGuiCol_Header] = highlightBlue; // selected items (tree, menu bar etc.)
	style.Colors[ImGuiCol_HeaderHovered] = highlightBlueHovered; // hovered items (tree, menu bar etc.)
	style.Colors[ImGuiCol_HeaderActive] = highlightBlueActive;
	style.Colors[ImGuiCol_Separator] = backgroundLight;
	style.Colors[ImGuiCol_ResizeGrip] = backgroundMedium;
	style.Colors[ImGuiCol_ResizeGripHovered] = highlightBlue;
	style.Colors[ImGuiCol_ResizeGripActive] = highlightBlueHovered;
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.7f, 0.77f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = highlightBlue; // Also used for progress bar
	style.Colors[ImGuiCol_PlotHistogramHovered] = highlightBlueHovered;
	style.Colors[ImGuiCol_TextSelectedBg] = highlightBlue;
	style.Colors[ImGuiCol_PopupBg] = backgroundVeryDark;
	style.Colors[ImGuiCol_DragDropTarget] = backgroundLight;
}

void Gui::DockingPannel()
{
	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		//ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowBgAlpha(0.0f);


	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	bool bDocking = true;
	ImGui::Begin("DockingPannel", &bDocking, windowFlags);
	{
		ImGui::PopStyleVar(3);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID pannelID = ImGui::GetID("Pannel");
			ImGui::DockSpace(pannelID, ImVec2(0.0f, 0.0f), ImGuiWindowFlags_AlwaysAutoResize);
		}
	}
	ImGui::End();
}
