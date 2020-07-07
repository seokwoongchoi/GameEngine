#pragma once
enum class PinType
{
	Flow,
	Bool,
	Int,
	Float,
	String,
	Object,
	Function,
	Delegate,
};

enum class PinKind
{
	Output,
	Input
};

enum class NodeType
{
	Blueprint,
	Simple,
	Tree,
	Comment,
	Houdini
};







#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/Source/imgui_internal.h"
//namespace ed = ax::NodeEditor;
//namespace util = ax::NodeEditor::Utilities;



namespace ax {


	class BehaviorTreeEditor
	{

	public:
		BehaviorTreeEditor() {};
		~BehaviorTreeEditor() {};

		const char* Application_GetName();
		void Application_Initialize();
		void Application_Finalize();
		void Application_Frame(bool* show = nullptr);

		uint NodeCount();
		void BehaviorTree(Matrix* temp);


	private:

		ImRect ImGui_GetItemRect();
		ImRect ImRect_Expanded(const ImRect& rect, float x, float y);

	private:


	
	private:
		int            s_PinIconSize = 24;

		Texture*          s_HeaderBackground = nullptr;
		//c ImTextureID          s_SampleImage = nullptr;
		Texture*           s_SaveIcon = nullptr;
		Texture*           s_RestoreIcon = nullptr;
		
		vector<Node>    s_Nodes;
		vector<Link>    s_Links;
	};

}