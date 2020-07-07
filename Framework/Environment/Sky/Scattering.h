#pragma once
class Scattering 
{
public:
	explicit	Scattering(Shader* shader);
	~Scattering();
	
	Scattering(const Scattering&) = delete;
	Scattering& operator=(const Scattering&) = delete;
	
	void Render();
	void Pass(uint i) {
		this->pass
			= i;
	}

private:
	void CreateQuad();
	void CreateDoom();

private:
	const UINT width, height;

	Shader* shader;
	Render2D* render2D;

	RenderTarget* mieTarget, *rayleighTarget;
	DepthStencil* depthStencil;
	Viewport* viewport;

	VertexBuffer* vertexBuffer;

private://¹Ý±¸¿ë
	UINT domeCount;
	VertexBuffer* domeVertexBuffer;
	IndexBuffer* domeIndexBuffer;
	UINT domeVertexCount;
	UINT domeIndexCount;

	
	
	ID3DX11EffectShaderResourceVariable* sStar;
	Texture* texture;

	uint pass;
};