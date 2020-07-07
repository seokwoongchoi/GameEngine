#pragma once
#include "stdafx.h"

struct AsMaterial
{
	string Name;

	Color Ambient;
	Color Diffuse;
	Color Specular;
	Color Roughness;
	Color Metallic;
	
	string DiffuseFile;
	string SpecularFile;
	string NormalFile;
	string RoughnessFile;
	string MetallicFile;
};

struct AsBone
{
	int Index;
	string Name;
	
	int Parent;
	Matrix Transform;
};


struct AsMesh
{
	string Name;
	int BoneIndex; 

	Vector3 min;
	Vector3 max;
	string MaterialName;
	//aiMesh* Mesh;

	vector<Model::ModelVertex> Vertices;
	vector<uint> Indices;

};
/////////////////////////////////////////////////////////////////////////
struct AsBlendWeight
{
	D3DXVECTOR4 Indices = D3DXVECTOR4(0, 0, 0, 0);
	D3DXVECTOR4 Weights = D3DXVECTOR4(0, 0, 0, 0);

	void Set(UINT index, UINT boneIndex, float weight)
	{
		float i = (float)boneIndex;
		float w = weight;

		switch (index)
		{
		case 0: Indices.x = i; Weights.x = w; break;
		case 1: Indices.y = i; Weights.y = w; break;
		case 2: Indices.z = i; Weights.z = w; break;
		case 3: Indices.w = i; Weights.w = w; break;
		}
	}
};

struct AsBoneWeights
{
private:
	typedef pair<int, float> Pair;
	vector<Pair> BoneWeights;

public:
	void AddWeights(UINT boneIndex, float boneWeights)
	{
		if (boneWeights <= 0.0f) return;

		bool bAdd = false;
		vector<Pair>::iterator it = BoneWeights.begin();
		while (it != BoneWeights.end())
		{
			if (boneWeights > it->second)
			{
				BoneWeights.insert(it, Pair(boneIndex, boneWeights));
				bAdd = true;

				break;
			}

			it++;
		} // while(it)

		if (bAdd == false)
			BoneWeights.push_back(Pair(boneIndex, boneWeights));
	}

	void GetBlendWeights(AsBlendWeight& blendWeights)
	{
		for (UINT i = 0; i < BoneWeights.size(); i++)
		{
			if (i >= 4) return; //4개까지만 들어가게한다.

			blendWeights.Set(i, BoneWeights[i].first, BoneWeights[i].second);
		}
	}

	void Normalize()
	{
		float totalWeight = 0.0f;

		int i = 0;
		vector<Pair>::iterator it = BoneWeights.begin();
		while (it != BoneWeights.end())
		{
			if (i < 4)
			{
				totalWeight += it->second;
				i++; it++;
			}
			else
				it = BoneWeights.erase(it);
		}

		float scale = 1.0f / totalWeight;

		it = BoneWeights.begin();
		while (it != BoneWeights.end())
		{
			it->second *= scale;
			it++;
		}
	}
};

/////////////////////////////////////////////////////////////////////////
struct AsKeyframeData
{
	float Time;

	D3DXVECTOR3 Scale;
	D3DXQUATERNION Rotation;
	D3DXVECTOR3 Translation;
};

struct AsKeyframe
{
	string BoneName;
	vector<AsKeyframeData> Transforms;
};

struct AsClipNode
{
	vector<AsKeyframeData> Keyframe;
	aiString Name;
};

struct AsClip
{
	string Name;

	UINT FrameCount;
	float FrameRate;
	float Duration;

	vector<AsKeyframe *> Keyframes;
};