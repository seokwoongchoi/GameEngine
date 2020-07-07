#pragma once
#define DEFAULT_POINT_SIZE    1.0f
#define MAX_SCALING_FACTOR   10.0f
#define SCALE_SLIDER_MAX        10

class EffectSystem
{
public:
	static EffectSystem* Get();

	static void Create();
	static void Delete();
public:
	EffectSystem();
	~EffectSystem();


	 ID3D11UnorderedAccessView * UAV(const uint& index);

	//inline ID3D11ShaderResourceView * SRV() {
	//	return m_pStructuredBufferSRV;	}

	 void ResidenceSharedData(const uint & actorIndex, const uint & instance,class SharedData * sharedData);
	
	void SetStart(bool bStart)
	{
		this->bStart = bStart;
	}

	void SetActivated(bool bActivated)
	{
		this->bActivated = bActivated;
	
	}

	void Update();
	void Render();
	void PreviewRender();

	uint SimulationsCount() { return simulations.size(); }

private:
	void ShowPopUp();
private:
	

	//class ParticleSimulation* simulation;
	//class ParticleSimulation* simulation2;
private:
	bool                          g_bShowHelp;
	bool                          g_bPaused;

	float                         g_pointSize;
	float                         g_fFrameTime;
	int                           g_iNumBodies;

	float                         g_clusterScale;
	float                         g_velocityScale;

	Vector3 position;
private:
	static EffectSystem* instance;
	unordered_map<string,class ParticleSimulation*> simulationsMap;
	vector<string> simulationList;


	vector< class ParticleSimulation*> simulations;

	
	bool bStart;

	bool bActivated = false;
	uint index;
};



