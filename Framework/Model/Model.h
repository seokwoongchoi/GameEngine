#pragma once


class ModelBone;
class ModelMesh;
class ModelClip;

class Model //데이터만 읽고 분배해주는 역활
{
public:
	typedef VertexTextureNormalTangentBlend ModelVertex;
public:
	Model();
	~Model();

	uint GetunArmedBoneCount() {
		return unArmedBoneCount;
	}
	UINT MaterialCount() { return materials.size(); }
	vector<Material *>& Materials() { return materials; }
	Material* MaterialByIndex(UINT index) { return materials[index]; }
	Material* MaterialByName(wstring name);

	void AddBone(int index, string name, int parentIndex, Matrix transform);
	void AddBindingBone(int parentIndex);
	
	UINT BoneCount() { return bones.size(); }
	inline ModelBone** BoneData() { return bones.data(); }
	vector<ModelBone *>& Bones() { return bones; }
	ModelBone* BoneByIndex(UINT index);
	ModelBone* BoneByName(wstring name);
	void ChangeBone(const Matrix& matrix, uint index);

	inline UINT MeshCount() { return meshes.size(); }
//	vector<ModelMesh *>& Meshes() { return meshes; }
	inline ModelMesh** MeshsData() { return meshes.data(); }
	

	

	ModelMesh* MeshByIndex(UINT index) { return meshes[index]; }
    ModelMesh* MeshByName(wstring name);

	UINT ClipCount() { return clips.size(); }
	vector<ModelClip *>& Clips() { return clips; }
	ModelClip* ClipByIndex(UINT index) { return clips[index]; }
	ModelClip* ClipByName(wstring name);

	inline ModelClip** ClipData() { return clips.data(); }

	void DeleteBone(ModelBone* bone);

public:
	inline uint previewMeshesCount() { return previewMeshes.size(); }
	inline ModelMesh** previewMeshsData() {return previewMeshes.data();}
public:
	inline uint forwardMeshesCount() { return forwardMeshes.size(); }
	inline ModelMesh** forwardMeshsData() { return forwardMeshes.data(); }
	void BlendModelMeshes(ModelBone* currentBone);

	string ModelName() { return modelName; }
	void Attach(Shader* shader, Model* model, int parentBoneIndex, Transform* transform = NULL);
public:
	void ReadMaterial(wstring file);
	void ReadMesh(wstring file);
	void ReadPreviewMesh(wstring file, const uint& MeshType);
	void ReadClip(wstring file);
public:
	void CompileMesh();
	

private:
	void BindingBone();
	void BindingMesh();
	void BindingPreviewMesh();
private:
	vector<Material*> materials;

	ModelBone* root;
	vector<ModelBone *> bones;

	vector<ModelMesh *> meshes;

	vector<ModelMesh *> previewMeshes;

	vector<ModelMesh *> forwardMeshes;

	vector<ModelClip *> clips;
	uint unArmedBoneCount = 0;
	
	string modelName;

	Vector3 min;
	Vector3 max;
};

