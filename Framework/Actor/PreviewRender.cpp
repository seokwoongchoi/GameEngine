#include "Framework.h"
#include "PreviewRender.h"
#include "Viewer/Orbit.h"
#include "Model/ModelMesh.h"
#include "Model/ModelClip.h"


PreviewRender::PreviewRender(Shader * shader)
	:shader(shader),model(nullptr), currentMat(nullptr), bBlend(false),
	bPause(false), lineCount(0)
{

	orbit = new Orbit();
	orbit->SetCameraType(CameraType::Orbit);
	orbit->SetOrbitTargetPosition(Vector3(0, 0, 0));
	pers = new Perspective(1280, 720);

	previewbuffer = new ConstantBuffer(&previewDesc, sizeof(PreviewDesc));

	sPreviewBuffer = shader->AsConstantBuffer("CB_Preview");

	previewTarget = new RenderTarget(static_cast<uint>(1280), static_cast<uint>(720));
	Vector3 pos = { 0.0f, 0.0f, 0.0f };
	Vector3 scale = { 0.04f, 0.04f, 0.04f };
	previewTransform = new Transform(shader);
	previewTransform->Position(pos);
	previewTransform->Scale(scale);

	forwardBuffer = new ConstantBuffer(&forward, sizeof(forwardDesc));
	sForwardBuffer = shader->AsConstantBuffer("CB_PreviewForward");

	previewFrameBuffer = new ConstantBuffer(&previewframe, sizeof(previewFrameDesc));
	sPreviewFrameBuffer = shader->AsConstantBuffer("CB_PreviewFrame");

	for (uint i = 0; i < MAX_MODEL_TRANSFORMS; i++)
	{
		D3DXMatrixIdentity(&previewframe.matrix[i]);
	}
	vertices = new VertexColor[MAX_LINE_VERTEX];
	ZeroMemory(vertices, sizeof(vertices));

	vertexBuffer = new VertexBuffer(vertices, MAX_LINE_VERTEX, sizeof(VertexColor), 0, true);

	buffer = new ConstantBuffer(&bufferDesc, sizeof(BufferDesc));
	sBuffer = shader->AsConstantBuffer("CB_World");
	

	D3DXMatrixIdentity(&boxWorld);
	for(uint i=0;i<2;i++)
	D3DXMatrixIdentity(&colliderBoxData[i].ColliderBoxWorld);

	D3DXMatrixIdentity(&effectBone);
	cameraTexture = new Texture(L"CameraGizmo.png");

}

PreviewRender::~PreviewRender()
{
	SafeDelete(cameraTexture);
	SafeDeleteArray(vertices);
	SafeDelete(vertexBuffer);
	SafeDelete(buffer);
	SafeRelease(sBuffer);
	SafeDelete(previewFrameBuffer);
	SafeRelease(sPreviewFrameBuffer);
	SafeDelete(forwardBuffer);
	SafeRelease(sForwardBuffer);
	SafeDelete(orbit);
	SafeDelete(pers);
	SafeDelete(previewbuffer);
	SafeRelease(sPreviewBuffer);
	SafeDelete(previewTarget);
	SafeDelete(previewTransform);
	SafeDelete(model);
	SafeDelete(shader);
}

void PreviewRender::SetModel(Model * model)
{
	BoundsMin = Vector3(
		std::numeric_limits<float>::infinity(),
		std::numeric_limits<float>::infinity(),
		std::numeric_limits<float>::infinity()
	);
	BoundsMax = Vector3(
		-std::numeric_limits<float>::infinity(),
		-std::numeric_limits<float>::infinity(),
		-std::numeric_limits<float>::infinity()
	);

	this->model = model;
	uint count = this->model->previewMeshesCount();
	
	
	if (count > 0)
	{
		auto mesh = this->model->previewMeshsData();
		
			for (uint i = 0; i < count; i++)
			{

			mesh[i]->SetShader(shader);
		
			Vector3 min, max;
			D3DXVec3TransformCoord(&min, &mesh[i]->MinPos(), &previewTransform->World());
			D3DXVec3TransformCoord(&max, &mesh[i]->MaxPos(), &previewTransform->World());
			BoundsMin = Math::Min(BoundsMin, min);
			BoundsMax = Math::Min(BoundsMax, max);
		


			}
		
		
		
	}


	
	
		
	
}

void PreviewRender::Update()
{
	
	bone = model->BoneData();
	
	if (model->ClipCount() > 0)
	{
		
		ModelClip* clip = model->ClipByIndex(previewframe.Clip);
		
		uint f = static_cast<uint>(UpdatePreviewFrame());
		//for (UINT f = 0; f < clip->FrameCount(); f++)
		
		for (UINT b = 0; b < model->BoneCount(); b++)
		{
			//ModelBone* bone = model->BoneByIndex(b);

			Matrix parent;
			Matrix invGlobal = bone[b]->Transform();
			D3DXMatrixInverse(&invGlobal, NULL, &invGlobal);

			int parentIndex = bone[b]->ParentIndex();
			if (parentIndex < 0)
				D3DXMatrixIdentity(&parent);
			else
				parent = bones[parentIndex];


			Matrix animation;

			ModelKeyframe* frame = clip->Keyframe(bone[b]->Name());

			
			if (frame != NULL)
			{

				if (f <= 0.0f)
				{
					ModelKeyframeData data = frame->Transforms[0];
					Vector3 s = data.Scale;
					Vector3 p = data.Translation;
					Quaternion q = data.Rotation;

					D3DXMATRIX S, R, T;
					D3DXMatrixScaling(&S, s.x, s.y, s.z);
					D3DXMatrixTranslation(&T, p.x, p.y, p.z);
					D3DXMatrixRotationQuaternion(&R, &q);

					animation = S * R * T;
				}
				//애니메이션 끝일때
				else if (f >= clip->FrameCount() - 1)
				{
					ModelKeyframeData data = frame->Transforms[clip->FrameCount() - 1];
					Vector3 s = data.Scale;
					Vector3 p = data.Translation;
					Quaternion q = data.Rotation;

					D3DXMATRIX S, R, T;
					D3DXMatrixScaling(&S, s.x, s.y, s.z);
					D3DXMatrixTranslation(&T, p.x, p.y, p.z);
					D3DXMatrixRotationQuaternion(&R, &q);

					animation = S * R * T;
				}
				//애니메이션 중일때
				else
				{
					
					
						ModelKeyframeData data = frame->Transforms[uint(f)];
						
						ModelKeyframeData nextData = frame->Transforms[uint(f) + 1];
						
						Vector3 s = D3DXVECTOR3(1, 1, 1);
						Vector3 s0 = data.Scale;
						Vector3 s1 = nextData.Scale;

						Vector3 p = D3DXVECTOR3(0, 0, 0);
						Vector3 p0 = data.Translation;
						Vector3 p1 = nextData.Translation;

						Quaternion q = D3DXQUATERNION(0, 0, 0, 1);
						Quaternion q0 = data.Rotation;
						Quaternion q1 = nextData.Rotation;

						//선형보간 - linear interpolate
						D3DXVec3Lerp(&s, &s0, &s1, previewframe.Time);
						D3DXVec3Lerp(&p, &p0, &p1, previewframe.Time);


						//구면보간
						D3DXQuaternionSlerp(&q, &q0, &q1, previewframe.Time);

						D3DXMATRIX S, R, T;
						D3DXMatrixScaling(&S, s.x, s.y, s.z);
						D3DXMatrixTranslation(&T, p.x, p.y, p.z);
						D3DXMatrixRotationQuaternion(&R, &q);

						animation = S * R*T;
						
					
					
				}

				bones[b] = animation * parent;

				previewframe.matrix[b] = invGlobal * bones[b];
			
			}
			else
			{
				//bones[b] = parent;
				//if(bone[b]->Name()== L"4_Cube.007")
				//Debug::DebugVector(Vector3(bone[b]->Transform()._41,bone[b]->Transform()._42, bone[b]->Transform()._43));
				previewframe.matrix[b] = bone[b]->Transform()* bones[bone[b]->ParentIndex()];

			}
		}



	}
	else
	{
	    
		for (UINT b = 0; b < model->BoneCount(); b++)
		{
			//ModelBone* bone = model->BoneByIndex(b);

			Matrix parent;


			int parentIndex = bone[b]->ParentIndex();
			if (parentIndex < 0)
				D3DXMatrixIdentity(&parent);
			else
				parent = bones[parentIndex];

			
			Matrix invGlobal = bone[b]->Transform();
			D3DXMatrixInverse(&invGlobal, NULL, &invGlobal);
			//bones[b] = parent;
			bones[b] = parent;

			previewframe.matrix[b] = bone[b]->Transform()*bones[b];


		}


	}


	if (previewFrameBuffer != NULL)
	{//에니메이션있는 모델만
		previewFrameBuffer->Apply();
		sPreviewFrameBuffer->SetConstantBuffer(previewFrameBuffer->Buffer());
	}
	

}

void PreviewRender::Render()
{
	/*Vector3 Pos;
		previewTransform->Position(&Pos);*/

	if (ImGui::IsKeyPressed('E') && ImGui::IsAnyWindowHovered())
	{
		UpDownFactor += 1.1f;
	}
	else if (ImGui::IsKeyPressed('Q') && ImGui::IsAnyWindowHovered())
	{
		UpDownFactor -= 1.1f;
	}

	if (ImGui::IsKeyPressed('D') && ImGui::IsAnyWindowHovered())
	{
		LeftRightFactor += 1.1f;
	}
	else if (ImGui::IsKeyPressed('A') && ImGui::IsAnyWindowHovered())
	{
		LeftRightFactor -= 1.1f;
	}


	orbit->SetOrbitTargetPosition(Vector3(0 + LeftRightFactor, 0 + UpDownFactor, 0));
	orbit->PreviewUpdate();
	orbit->GetMatrix(&orbitView);

	pers->GetMatrix(&orbitProj);
	D3DXMatrixMultiply(&previewDesc.VP, &orbitView, &orbitProj);
	previewbuffer->Apply();
	sPreviewBuffer->SetConstantBuffer(previewbuffer->Buffer());


	previewTransform->Render();
	previewTarget->Set(nullptr);
	
	uint count = model->previewMeshesCount();
	if (count > 0)
	{
	   auto mesh = model->previewMeshsData();
		
		for (uint i = 0; i < count; i++)
		{
			mesh[i]->Pass(13);
			mesh[i]->PreviewRender();
		}
	}

	if (bBlend)
	{
		ForwardRender();
	}
	

}

void PreviewRender::ForwardRender()
{
	if (forwardBuffer != NULL)
	{//에니메이션있는 모델만
		forwardBuffer->Apply();
		sForwardBuffer->SetConstantBuffer(forwardBuffer->Buffer());
	}

	uint count = model->forwardMeshesCount();
	if (count > 0)
	{
		auto mesh = model->forwardMeshsData();
		for (uint i = 0; i < count; i++)
		{
			mesh[i]->Pass(14);
			mesh[i]->PreviewRender();
		}
	}
}

void PreviewRender::DebugRender()
{
	BoxRender(boxWorld, BoundsMin, BoundsMax, Color(1, 0, 0, 1));

	if(boxCount>0)
		for (uint i = 0; i < static_cast<uint>(boxCount); i++)
		{
			if (model->GetunArmedBoneCount() > colliderBoxData[i].Index)
				colliderBoxData[i].matrix= bone[colliderBoxData[i].Index]->Transform()*
				previewframe.matrix[colliderBoxData[i].Index] * previewTransform->World();
			else
				colliderBoxData[i].matrix = previewframe.matrix[colliderBoxData[i].Index] * previewTransform->World();

			D3DXMatrixDecompose(&colliderBoxData[i].scale, &colliderBoxData[i].q, 
				&colliderBoxData[i].position, &colliderBoxData[i].matrix);

			D3DXMatrixRotationQuaternion(&colliderBoxData[i].R, &colliderBoxData[i].q);
						
			D3DXMatrixTranslation(&colliderBoxData[i].T, colliderBoxData[i].position.x,
				colliderBoxData[i].position.y, colliderBoxData[i].position.z);

			

			colliderBoxData[i].result = colliderBoxData[i].ColliderBoxWorld* colliderBoxData[i].R * colliderBoxData[i].T;
			BoxRender(colliderBoxData[i].result, boneBoxMin, boneBoxMax, Color(0, 1, 0, 1));
		}


	
}


const Matrix & PreviewRender::GetEffectBoneMatrix( const uint& index)
{
	if (model->GetunArmedBoneCount() > effectData[index].Index)
		effectData[index].matrix = model->BoneByIndex(effectData[index].Index)->Transform()*
		previewframe.matrix[effectData[index].Index]*previewTransform->World();
	else
		effectData[index].matrix =	previewframe.matrix[effectData[index].Index] * previewTransform->World();
	

	return effectData[index].matrix;
	//return effectData[index].R * effectData[index].T;
}


void PreviewRender::GetBoxWorld(Matrix * matrix, uint boxType)
{
		
		switch (boxType)
		{
		case 2:
			*matrix = boxWorld;
			break;
		case 3:
			*matrix = colliderBoxData[0].ColliderBoxWorld;
			break;
		case 4:
			*matrix = colliderBoxData[1].ColliderBoxWorld;
			break;
		
		default:
			break;
		}
}

void PreviewRender::SetBoxWorld(const Matrix & matrix, uint boxType)
{
	switch (boxType)
	{
	case 2:
		this->boxWorld = matrix;
		break;
	case 3:
		this->colliderBoxData[0].ColliderBoxWorld = matrix;
		break; 
	case 4:
		this->colliderBoxData[1].ColliderBoxWorld = matrix;
		break;
	
	default:
		break;
	}
}

void PreviewRender::GetBox(Vector3 *min, Vector3 * max)
{
	Matrix inv;
	D3DXMatrixInverse(&inv, nullptr, &previewTransform->World());
	Matrix result = boxWorld * inv;
	D3DXVec3TransformCoord(min, &BoundsMin, &result);
	D3DXVec3TransformCoord(max, &BoundsMax, &result);
	
}

const Matrix & PreviewRender::GetSkinnedBoneTransform(const uint & index)
{
	
	
	if (model->GetunArmedBoneCount() > index)
	{
		const Matrix& temp= bone[index]->Transform()*previewframe.matrix[index];
		return temp;
	}


	return previewframe.matrix[index];
	

}





const Matrix & PreviewRender::BlendModelBone(const uint & index)
{
	forward.matrix= previewframe.matrix[index];
	return previewframe.matrix[index];
}



float PreviewRender::UpdatePreviewFrame()
{

	ModelClip** clipData = model->ClipData();

	if (bPause)
		previewframe.RunningTime += Time::Delta();


	float invFrameRate = 1.0f / clipData[previewframe.Clip]->FrameRate();
	if (previewframe.RunningTime > invFrameRate && bPause)
	{
		previewframe.RunningTime = 0.0f;

		previewframe.CurrFrame = (previewframe.CurrFrame + 1) % clipData[previewframe.Clip]->FrameCount();
		previewframe.NextFrame = (previewframe.CurrFrame + 1) % clipData[previewframe.Clip]->FrameCount();
	}
	else if (!bPause)
	{
		previewframe.CurrFrame = static_cast<uint>(currentFrame);
	}

	previewframe.Time = previewframe.RunningTime / invFrameRate;


	return previewframe.CurrFrame;
}

void PreviewRender::RenderLine(const Vector3 & start, const Vector3 & end, const Color & color)
{
	vertices[lineCount].Color = color;
	vertices[lineCount++].Position = start;

	vertices[lineCount].Color = color;
	vertices[lineCount++].Position = end;
}

void PreviewRender::BoxRender(const Matrix & matrix, const Vector3 & min, const Vector3 & max, const Color& color)
{
	bufferDesc.World = matrix;

	
	D3DXVECTOR3 dest[8], temp[8];
	dest[0] = Vector3(min.x, min.y, max.z);
	dest[1] = Vector3(max.x, min.y, max.z);
	dest[2] = Vector3(min.x, max.y, max.z);
	dest[3] = Vector3(max);
	dest[4] = Vector3(min);
	dest[5] = Vector3(max.x, min.y, min.z);
	dest[6] = Vector3(min.x, max.y, min.z);
	dest[7] = Vector3(max.x, max.y, min.z);


	//Front
	RenderLine(dest[0], dest[1], color);
	RenderLine(dest[1], dest[3], color);
	RenderLine(dest[3], dest[2], color);
	RenderLine(dest[2], dest[0], color);

	//Backward
	RenderLine(dest[4], dest[5], color);
	RenderLine(dest[5], dest[7], color);
	RenderLine(dest[7], dest[6], color);
	RenderLine(dest[6], dest[4], color);

	//Side
	RenderLine(dest[0], dest[4], color);
	RenderLine(dest[1], dest[5], color);
	RenderLine(dest[2], dest[6], color);
	RenderLine(dest[3], dest[7], color);

	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());


	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, vertices, sizeof(VertexColor) * MAX_LINE_VERTEX);
	}
	D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);


	vertexBuffer->Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	shader->Draw(0, 16, lineCount * 2);

	lineCount = 0;

	ZeroMemory(vertices, sizeof(vertices));
}

