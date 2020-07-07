#pragma once
class Fog
{
public:
	Fog(class Shader* shader);
	~Fog();

	void Update();
	void Render();
	void ImGui();
private:
	struct CB_FOG_PS
	{
		D3DXVECTOR3 vColor;
		float fStartDepth;
		D3DXVECTOR3 vHighlightColor;
		float fGlobalDensity;
		
		float fHeightFalloff;
		Vector3 Padding;
	};

	CB_FOG_PS fogCB;

	ConstantBuffer* fogBuffer;
	ID3DX11EffectConstantBuffer* sFogBuffer;

private:
	class Shader* shader;
};
