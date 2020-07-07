#pragma once
class Loader
{
public:
	Loader();
	~Loader();

public:
	
	void ReadFile(wstring file);
	void ExportMaterial(wstring savePath, bool bOverWrite = true);
	//우리가 원하는 xml형태로 저장한다
private:
	void ReadMaterial();
	void WriteMaterial(wstring savePath, bool bOverWrite);
	string WriteTexture(string savePath,string file);


public:
	void ExportMesh(wstring savePath, bool bOverWrite = true);
private:
	void ReadBoneData(aiNode* node, int index, int parent);
	void ReadMeshData(aiNode* node, int bone);
	void ReadSkinData();
	void WriteMeshData(wstring savePath, bool bOverWrite);

public:
	void GetClipList(vector<wstring>* list);
	void ExportAnimClip(uint index, wstring savePath, bool bOverWrite = true);
	
private:
	struct AsClip* ReadClipData(aiAnimation* animation);
	void ReadKeyFrameData(struct AsClip* clip, aiNode* node, vector<struct AsClipNode>& animNodeInfos);
	void WriteClipData(struct AsClip* clip, wstring savePath, bool bOverwrite);
private:
	 Matrix ConvertMatrix =
	{
		-1, 0,-8.74227766e-08f, 0,
		0, 1, 0, 0,
		8.74227766e-08f, 0, 1, 0,
		0, 0, 0, 1
	};
	wstring file;
	Assimp::Importer* importer;
	const aiScene* scene;


	vector<struct AsMaterial*> materials;
	vector<struct AsBone*> bones;
	vector<struct AsMesh*> meshes;

	int assimpFlags;

	Vector3 minPos;
	Vector3 maxPos;
};
