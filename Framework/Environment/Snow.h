#pragma once

class Snow:public Renderer
{
public:
	Snow(Vector3& extent,uint count);
	~Snow();

	void Update();
	void Render();

private:
	struct Desc
	{
		D3DXCOLOR Color = D3DXCOLOR(1, 1, 1, 1);
		Vector3 Velocity = Vector3(-10, -100, 0);
		float DrawDistance = 1000.0f;
		
		Vector3 Origin = Vector3(0, 0, 0);
		float Turbulence = 5;
		
		Vector3 Extent = Vector3(0, 0, 0);
		float Padding1;
	}desc;
private:
	struct VertexSnow
	{
		Vector3 Position;
		Vector2 Uv;
		float Scale;
		Vector2 Random;
		
	};
private:
	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;
	VertexSnow* vertices;
	uint* indices;
	
	Texture* texture;
	
	uint drawCount;
	

};

