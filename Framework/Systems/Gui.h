#pragma once

struct GuiText
{
	D3DXVECTOR2 Position;
	D3DXCOLOR Color;
	string Content;

	GuiText()
	{

	}
};

struct GuiTexture
{
	D3DXVECTOR2 Position;
	D3DXVECTOR2 scale;
	class Texture* texture;
	Color color;

	GuiTexture()
	{

	}
};

class Gui
{
public:
	static void Create();
	static void Delete();

	static Gui* Get();

	LRESULT MsgProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
	void Resize();

	void Update();
	void Render();
		
	

	void AddWidget(class Widget* widget);

	void RenderText(GuiText& text);
	void RenderText(float x, float y, string content);
	void RenderText(float x, float y, float r, float g, float b, string content);
	void RenderText(D3DXVECTOR2 position, D3DXCOLOR color, string content);
	void RenderTexture(D3DXVECTOR2 position, D3DXVECTOR2 scale, Color color, Texture* texture);



	void ShowTransformGizmo(Matrix* transform);


	void AddTransform(Matrix* transform);
private:
	Gui();
	~Gui();

private:
	void ApplyStyle();
	void DockingPannel();

private:
	static Gui* instance;
	vector<class Widget *> widgets;

	vector<GuiText> texts;
	vector<GuiTexture> textures;
	vector<Matrix*> transforms;
	Matrix* controlTransform=nullptr;

};