#pragma once
class Atmosphere :public Renderer
{
public:
	explicit Atmosphere(Shader* shader);
	~Atmosphere();

	Atmosphere(const Atmosphere&) = delete;
	Atmosphere& operator=(const Atmosphere&) = delete;

	void Update();
	void ImGui();
	
	void Render();
	
	void Pass(const uint& index);
	

private:
	struct ScatterDesc
	{
		Vector3 WaveLength = Vector3(0.65f, 0.57f, 0.475f);
		int SampleCount = 8;

		Vector3 InvWaveLength;
		float StarIntensity;

		Vector3 WaveLengthMie;
		float MoonAlpha;

		float Time;
		Vector3 pad;
	} scatterDesc;



private:
	bool realTime;
	float timeFactor;

	float theta, prevTheta;

	class Scattering* scattering;
	ConstantBuffer* scatterBuffer;
	ID3DX11EffectConstantBuffer* sScatterBuffer;
	

	class Moon* moon;
	uint pass;
	Vector3 position;
	Vector3 SunPos;

	 float scale = 850.0f;
};

