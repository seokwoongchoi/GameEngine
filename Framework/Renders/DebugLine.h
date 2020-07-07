#pragma once
#define MAX_LINE_VERTEX 1000

class DebugLine
{
public:
	friend class Window;

public:
	static void Create();
	static void Delete();

	static DebugLine* Get();

public:
	
	void RenderLine(Vector3& start, Vector3& end);
	void RenderLine(Vector3& start, Vector3& end, float r, float g, float b);

	void RenderLine(float x, float y, float z, float x2, float y2, float z2);
	void RenderLine(float x, float y, float z, float x2, float y2, float z2, D3DXCOLOR& color);
	void RenderLine(float x, float y, float z, float x2, float y2, float z2, float r, float g, float b);

	void RenderLine(Vector3& start, Vector3& end, Color& color,uint ID=0);
	void RenderLine(const Vector3& start, const Vector3& end, uint ID = 0);
	
	
	inline void SRV(ID3D11ShaderResourceView  *srv)
	{
		this->srv = srv;
	}

	void Pass(const uint& pass)
	{
		this->pass = pass;
	}

private:
	void Render();

private:
	explicit DebugLine();
	~DebugLine();
	
		DebugLine(const DebugLine&) = delete;
	DebugLine& operator=(const DebugLine&) = delete;
private:
	static DebugLine* instance;

private:
	Shader* shader;
	PerFrame* perFrame;
	Transform* transform;

	VertexBuffer* vertexBuffer;
	
	VertexColor* vertices;

private:
	
	


	UINT drawCount = 0;

	ID3D11ShaderResourceView  *srv=nullptr;
	ID3DX11EffectShaderResourceVariable* sSrv;

	uint pass = 0;

	Matrix worlds[MAX_MODEL_INSTANCE];
};
