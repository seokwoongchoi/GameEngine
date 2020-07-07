#pragma once

#define MAX_MODEL_TRANSFORMS 450
#define MAX_MODEL_KEYFRAMES 350
#define MAX_MODEL_INSTANCE 50
#define MAX_ACTOR_BONECOLLIDER 2
#ifdef _DEBUG
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

#include <Windows.h>
#include <assert.h>

//STL
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <functional>
#include <iterator>
#include <thread>
#include <mutex>
#include <queue>
#include <algorithm>
#include <chrono>
#include <utility>
using namespace std;


//Direct3D
#include <dxgi1_2.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <d3dx11effect.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Effects11d.lib")


//ImGui
# include "ImGui/Source/imgui.h"




#include "Systems/ImGuizmo.h"



//DirectXTex
#include <DirectXTex.h>
#pragma comment(lib, "directxtex.lib")

#define Check(hr) { assert(SUCCEEDED(hr)); }
#define Super __super

#define SafeRelease(p){ if(p){ (p)->Release(); (p) = NULL; } }
#define SafeDelete(p){ if(p){ delete (p); (p) = NULL; } }
#define SafeDeleteArray(p){ if(p){ delete [] (p); (p) = NULL; } }

typedef D3DXVECTOR2 Vector2;
typedef D3DXVECTOR3 Vector3;
typedef D3DXVECTOR4 Vector4;

typedef D3DXCOLOR Color;
typedef D3DXMATRIX Matrix;
typedef D3DXQUATERNION Quaternion;
typedef unsigned int uint;
typedef D3DXPLANE Plane;

#include "Systems/D3D.h"
#include "Systems/Keyboard.h"
#include "Systems/Mouse.h"
#include "Systems/Time.h"
#include "Systems/Gui.h"
#include "Systems/Thread.h"
#include "BehaviorTree/BehaviorTree.h"
#include "EventSystems/EventSystem.h"
#include "EventSystems/EffectSystem.h"
#include "EventSystems/ColliderSystem.h"

#include "Viewer/Viewport.h"
#include "Viewer/Projection.h"
#include "Viewer/Perspective.h"
#include "Viewer/Orthographic.h"
#include "Viewer/Camera.h"
#include "Viewer/RenderTarget.h"
#include "Viewer/DepthStencil.h"
#include "Viewer/Frustum.h"


#include "Renders/Buffers.h"
#include "Renders/Shader.h"
#include "Renders/Texture.h"
#include "Renders/VertexLayouts.h"
#include "Renders/Context.h"
#include "Renders/Material.h"
#include "Renders/PerFrame.h"
#include "Renders/Transform.h"
#include "Renders/Renderer.h"
#include "Renders/DebugLine.h"
#include "Renders/Render2D.h"


#include "Renders/GBuffer/DeferredDirLight.h"
#include "Renders/GBuffer/DeferredPointLight.h"
#include "Renders/GBuffer/DeferredSpotLight.h"


#include "Objects/Collider.h"
#include "Objects/Shadow.h"

#include "Utilities/Math.h"
#include "Utilities/String.h"
#include "Utilities/Path.h"
#include "Utilities/Debug.h"

#include "Mesh/Mesh.h"
#include "Mesh/MeshQuad.h"
#include "Mesh/MeshSphere.h"
#include "Mesh/MeshCylinder.h"
#include "Mesh/MeshGrid.h"
#include "Mesh/MeshCube.h"
#include "Mesh/MeshNormal.h"
#include "Mesh/MeshCollider.h"
#include "Mesh/MeshInstances/MeshCubeInstance.h"
#include "Mesh/MeshInstances/MeshSphereInstance.h"
#include "Mesh/MeshInstances/MeshCylinderInstance.h"

#include "Model/Model.h"
