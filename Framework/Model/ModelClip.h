#pragma once

struct ModelKeyframeData
{
	float Time;

	D3DXVECTOR3 Scale;
	D3DXQUATERNION Rotation;
	D3DXVECTOR3 Translation;
};

struct ModelKeyframe
{
	wstring BoneName;
	vector<ModelKeyframeData> Transforms;
};

class ModelClip
{
public:
	friend class Model;

private:
	ModelClip();
	~ModelClip();

public:
	float Duration() { return duration; }
	inline float FrameRate() { return frameRate; }
	inline UINT FrameCount() { return frameCount; }

	ModelKeyframe* Keyframe(wstring name);
	

private:
	wstring name;

	float duration;
	float frameRate;
	UINT frameCount;

	unordered_map<wstring, ModelKeyframe *> keyframeMap;


	//redblack tree Ư¡ �����غ���
	//grid
	//������������
	//������ bakc tracking
};