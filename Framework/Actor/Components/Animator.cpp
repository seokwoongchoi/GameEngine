#include "Framework.h"
#include "Animator.h"
#include "Model/ModelClip.h"
#include "Model/ModelMesh.h"

Animator::Animator(Shader * shader, Model * model, SharedData* sharedData)
	:Component(shader,model, sharedData),bPause(false), meshCount(0), srv(nullptr),skinTransforms(nullptr)
	
	
	
	
{
	
	frameBuffer = new ConstantBuffer(tweens, sizeof(TweenDesc) * MAX_MODEL_INSTANCE);
	sFrameBuffer = shader->AsConstantBuffer("CB_AnimationFrame");

	sTransformSRV = shader->AsSRV("BoneTransforms");
	sCSTransformSRV= sharedData-> computeShader->AsSRV("BoneTransforms");

	sEffectUav= sharedData->computeShader->AsUAV("EffectOutput");
	sUav = sharedData->computeShader->AsUAV("Output");
	
	

	computeAttachBuffer = new ConstantBuffer(&attachDesc, sizeof(AttachDesc)*MAX_ACTOR_BONECOLLIDER);
	sComputeAttachBuffer = sharedData->computeShader->AsConstantBuffer("CB_Attach");

	

	computeEffectAttachBuffer = new ConstantBuffer(&effectAttachDesc, sizeof(EffectAttachDesc)*MAX_ACTOR_BONECOLLIDER);
	sComputeEffectAttachBuffer = sharedData->computeShader->AsConstantBuffer("CB_EffectAttach");

	sComputeFrameBuffer = sharedData->computeShader->AsConstantBuffer("CB_AnimationFrame");


	{
		//clipBuffer = new ConstantBuffer(&clipDesc, sizeof(ClipDesc) * 10);
	//sClipBuffer = computeShader->AsConstantBuffer("CB_Clip");



	//TweenDesc *tweenArray = new TweenDesc[MAX_MODEL_INSTANCE];
	//

	//D3D11_SUBRESOURCE_DATA initData = { tweenArray, 0, 0 };

	//SafeRelease(sharedData->tween_StructuredBuffer);
	//SafeRelease(sharedData->tween_StructuredBufferSRV);
	//SafeRelease(sharedData->tween_StructuredBufferUAV);

	//// Create Structured Buffer
	//D3D11_BUFFER_DESC sbDesc;
	//sbDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	//sbDesc.CPUAccessFlags = 0;
	//sbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	//sbDesc.StructureByteStride = sizeof(TweenDesc);
	//sbDesc.ByteWidth = sizeof(TweenDesc) *MAX_MODEL_INSTANCE;
	//sbDesc.Usage = D3D11_USAGE_DEFAULT;
	//Check(D3D::GetDevice()->CreateBuffer(&sbDesc, &initData, &sharedData->tween_StructuredBuffer));

	//
	//// create the Shader Resource View (SRV) for the structured buffer
	//D3D11_SHADER_RESOURCE_VIEW_DESC sbSRVDesc;
	//sbSRVDesc.Buffer.ElementOffset = 0;
	//sbSRVDesc.Buffer.ElementWidth = sizeof(TweenDesc);
	//sbSRVDesc.Buffer.FirstElement = 0;
	//sbSRVDesc.Buffer.NumElements = MAX_MODEL_INSTANCE;
	//sbSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	//sbSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	//Check(D3D::GetDevice()->CreateShaderResourceView(sharedData->tween_StructuredBuffer, &sbSRVDesc, &sharedData->tween_StructuredBufferSRV));
	//
	//// create the UAV for the structured buffer
	//D3D11_UNORDERED_ACCESS_VIEW_DESC sbUAVDesc;
	//sbUAVDesc.Buffer.FirstElement = 0;
	//sbUAVDesc.Buffer.Flags = 0;
	//sbUAVDesc.Buffer.NumElements = MAX_MODEL_INSTANCE;
	//sbUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	//sbUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	//Check(D3D::GetDevice()->CreateUnorderedAccessView(sharedData->tween_StructuredBuffer, &sbUAVDesc, &sharedData->tween_StructuredBufferUAV));
	//

	//SafeDeleteArray(tweenArray);


	//sFrameSRV=shader->AsSRV("Tweenframes");
	//sFrameUAV = computeShader->AsUAV("OutTween");

	//for (UINT i = 0; i < 10; i++)
	//{
	//	clipDesc[i].time = 0;
	//	clipDesc[i].frameRate = 0;
	//	clipDesc[i].frameCount =0;

	//}
	}
	
	tempTweens.assign(MAX_MODEL_INSTANCE, TweenDesc());
}

void Animator::OnDestroy()
{
	//SafeDelete(computeShader);
	//SafeDelete(computeBuffer);
	SafeDelete(computeAttachBuffer);
	SafeDelete(model);

	SafeDelete(frameBuffer);
	//SafeDelete(instanceBuffer);
	SafeDeleteArray(skinTransforms);

}

void Animator::OnStart()
{
	bStart = true;
}

void Animator::OnUpdate()
{

	//////////////////////////////////////////////////////////////////////////
	if (!bStart)return;
	
	
  	 ModelClip** clipData = model->ClipData();
	
	 
	
	for (UINT i = 0; i < sharedData->TotalCount(); i++)
	{

		TweenDesc& tween = tempTweens[i];
		tween.Curr.RunningTime += Time::Delta();
		uint currentClipNum = tween.Curr.Clip;
		
		
		
		switch (tween.state)
		{
		case ActorState::Idle:
		{ 
			
			PlayNextClip(i, 0);
		   
		}
		break;
		case ActorState::Fire:
		{
			PlayNextClip(i, 1);
		
		}
			break;
		case ActorState::Move:
		{
			
			PlayNextClip(i, 2);
		
		}
		break;
		case ActorState::Die:
		{
			PlayNextClip(i, 3);
			
			if (tween.Curr.NextFrame >= clipData[currentClipNum]->Duration() - 1)
			{
				tween.Curr.CurrFrame = static_cast<uint>(clipData[currentClipNum]->Duration()) - 2;
				tween.Curr.NextFrame = static_cast<uint>(clipData[currentClipNum]->Duration())-1;
			}

			tween.IsEventActivated++;
			if (tween.IsEventActivated > 1000)
			{

				if (sharedData->DeleteDiedInstance(i))
				{

					tempTweens.erase(tempTweens.begin() + i);
					tempTweens.shrink_to_fit();
					tempTweens.emplace_back(TweenDesc());

				}
				tween.IsEventActivated = 0;
				continue;
			}

			
		}
		break;
		case ActorState::WalkFire:
		{
			PlayNextClip(i, 4);
			
		}
		break;

		case ActorState::Reaction:
		{
			PlayNextClip(i, 5);
			
			if (tween.Curr.NextFrame >= clipData[currentClipNum]->Duration() - 1)
			{
				tween.state = ActorState::Idle;
				
			}
		}
		break;
		
		
		}
	
		    
			
			 float invFrameRate = 1.0f / clipData[currentClipNum]->FrameRate();
			 if (tween.Curr.RunningTime > invFrameRate)
			 {
				 tween.Curr.RunningTime = 0.0f;

				 tween.Curr.CurrFrame = (tween.Curr.CurrFrame + 1) % clipData[currentClipNum]->FrameCount();
				 tween.Curr.NextFrame = (tween.Curr.CurrFrame + 1) % clipData[currentClipNum]->FrameCount();
			 }

			 tween.Curr.Time = tween.Curr.RunningTime / invFrameRate;
			

			 if (tween.Next.Clip > -1)
			 {
				 tween.Next.RunningTime += Time::Delta();
				 tween.TweenTime = tween.Next.RunningTime / tween.TakeTime;

				 if (tween.TweenTime >= 1.0f)//완전히 변경됬다면
				 {
					 tween.Curr = tween.Next;


					 tween.Next.Clip = -1;
					 tween.Next.CurrFrame = 0;
					 tween.Next.NextFrame = 0;
					 tween.Next.Time = 0;
					 tween.Next.RunningTime = 0;

					 tween.TweenTime = 0.0f;
				 }
				 else
				 {
					 uint nextClipNum = tween.Next.Clip;
					 float invFrameRate = 1.0f / clipData[nextClipNum]->FrameRate();
					 if (tween.Next.Time > invFrameRate)
					 {
						 tween.Next.Time = 0.0f;

						 tween.Next.CurrFrame = (tween.Next.CurrFrame + 1) % clipData[nextClipNum]->FrameCount();
						 tween.Next.NextFrame = (tween.Next.CurrFrame + 1) % clipData[nextClipNum]->FrameCount();
					 }

					 tween.Next.Time = tween.Next.Time / invFrameRate;
				 }


			 }
			
			 
			 
	 }
	int index = 0;

	for (auto& i : sharedData->Index)
	{
		if (sharedData->TotalCount() == 1 && i > 0)
		{
			sharedData->Index[0] = 0;
		}
		
		memcpy(&tweens[index], &tempTweens[i], sizeof(TweenDesc));
		index++;
	}
		
	if (IsClacBoneBox)
	{
		CS_CalcBoneTransform();
		

	}

	/*if (IsClacEffect)
	{
		CS_CalcEffectTransform();


	}*/
	
	
}

void Animator::OnStop()
{
	
	bStart = false;
	for (UINT i = 0; i < MAX_MODEL_INSTANCE; i++)
	{
		tweens[i].state=ActorState::Idle;
		tweens[i].IsEventActivated = 0;
		tweens[i].Curr.Clip = 0;
		tweens[i].Next.Clip = -1;
		tweens[i].TakeTime = 1.0f;
		tweens[i].TweenTime = 0.0f;
	}
	
}

void Animator::Render()
{
	
	frameBuffer->Apply();
	sFrameBuffer->SetConstantBuffer(frameBuffer->Buffer());
	sTransformSRV->SetResource(srv);
}

void Animator::Collison(const uint & instance, const Vector3 & dir)
{
	if (sharedData->drawCount < instance|| sharedData->drawCount==0) return;
	D3DXVec3Normalize(&nor, &dir);

	
	
	
	sharedData->transforms[instance]._41 -= nor.x*0.4f;
	//sharedData->transforms[instance]._42 -= dir.y*0.5f;
	sharedData->transforms[instance]._43 -= nor.z*0.4f;

	

}

void Animator::SetBodyBox(const uint & BodyIndex, const Matrix & BodyMatrix, const uint & boxIndex)
{
	
	attachDesc[boxIndex].Index = BodyIndex;


	auto bone = model->BoneData();
	if (model->GetunArmedBoneCount() > BodyIndex)
	attachDesc[boxIndex].local = bone[attachDesc[boxIndex].Index]->Transform();
	else
	{
		D3DXMatrixIdentity(&attachDesc[boxIndex].local);
	}

	attachDesc[boxIndex].BoneScale = BodyMatrix;

	
	computeAttachBuffer->Apply();
	sComputeAttachBuffer->SetConstantBuffer(computeAttachBuffer->Buffer());
	IsClacBoneBox = true;
}





void Animator::SetEffectBone(const int & effectIndex,const uint& particleIndex)
{
	auto bone = model->BoneData();
	effectAttachDesc[particleIndex].EffectIndex = effectIndex;
	if (model->GetunArmedBoneCount() > effectIndex)
	effectAttachDesc[particleIndex].effectlocal = bone[effectAttachDesc[particleIndex].EffectIndex]->Transform();
	else
	{
		D3DXMatrixIdentity(&effectAttachDesc[particleIndex].effectlocal);
	}
	
	computeEffectAttachBuffer->Apply();
	sComputeEffectAttachBuffer->SetConstantBuffer(computeEffectAttachBuffer->Buffer());
	IsClacEffect = true;
}

void Animator::CreateBoneTransforms()
{
	
	if (model->ClipCount() > 0)
	{

		skinTransforms = new BoneTransform[model->ClipCount()];

		clip = model->ClipData();
		bone = model->BoneData();
		for (UINT i = 0; i < model->ClipCount(); i++)
			CreateAnimTransform(i);

		//Create Texture
		{
			D3D11_TEXTURE2D_DESC desc;
			ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
			desc.Width = MAX_MODEL_TRANSFORMS * 4;
			desc.Height = MAX_MODEL_KEYFRAMES;
			desc.MipLevels = 1;
			desc.ArraySize = model->ClipCount();
			desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_IMMUTABLE;

			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;


			UINT pageSize = MAX_MODEL_TRANSFORMS * 4 * 16 * MAX_MODEL_KEYFRAMES;
			void* p = malloc(pageSize * (model->ClipCount()));

			for (UINT c = 0; c < model->ClipCount(); c++)
			{
				for (UINT y = 0; y < MAX_MODEL_KEYFRAMES; y++)
				{
					UINT start = c * pageSize;
					void* temp = (BYTE *)p + MAX_MODEL_TRANSFORMS * y * sizeof(Matrix) + start;

					memcpy(temp, skinTransforms[c].Transform[y], sizeof(Matrix) * MAX_MODEL_TRANSFORMS);
				}
			}

			D3D11_SUBRESOURCE_DATA* subResource = new D3D11_SUBRESOURCE_DATA[model->ClipCount()];
			for (UINT c = 0; c < model->ClipCount(); c++)
			{
				void* temp = (BYTE *)p + c * pageSize;

				subResource[c].pSysMem = temp;
				subResource[c].SysMemPitch = MAX_MODEL_TRANSFORMS * sizeof(Matrix);
				subResource[c].SysMemSlicePitch = pageSize;
			}

			Check(D3D::GetDevice()->CreateTexture2D(&desc, subResource, &texture));

			SafeDeleteArray(subResource);
			free(p);
		}

		//Create SRV
		{
			D3D11_TEXTURE2D_DESC desc;
			texture->GetDesc(&desc);

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Format = desc.Format;
			srvDesc.Texture2DArray.MostDetailedMip = 0;
			srvDesc.Texture2DArray.MipLevels = 1;
			srvDesc.Texture2DArray.ArraySize = model->ClipCount();

			HRESULT hr = D3D::GetDevice()->CreateShaderResourceView(texture, &srvDesc, &srv);
			Check(hr);

			
			//// Create the UAVs
			//D3D11_UNORDERED_ACCESS_VIEW_DESC DescUAV;
			//ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
			//DescUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
			//DescUAV.Format = desc.Format;
			//
			//DescUAV.Texture2DArray.ArraySize = model->ClipCount() + 1;
			//
			//
			//Check(D3D::GetDevice()->CreateUnorderedAccessView(texture, &DescUAV, &uav));
		}
	}
	//sTransformSRV->SetResource(srv);
	
}

SharedData * Animator::GetSharedData()
{
	
	return sharedData;
}





void Animator::PlayNextClip(int instance, int clip, float time)
{
	
	/*if (sharedData->Index.size() <= instance) return;
	uint index = sharedData->Index[instance];
	
	tempTweens[index].IsEventActivated = 1;*/
	if (tempTweens[instance].Curr.Clip == clip) return;
	
	tempTweens[instance].Next.Clip = clip;
	tempTweens[instance].TakeTime = time;
	
	
}

void Animator::SetState(int instance, const ActorState & state)
{
	if (sharedData->Index.size() <= static_cast<uint>(instance)) return;
	uint index = sharedData->Index[instance];

	if (tempTweens[index].state == ActorState::Die) return;
	//tempTweens[index].IsEventActivated = instance;
	tempTweens[index].state = state;

	
	
}


bool Animator::IsEndAnimation(int instance, int clip)
{
	ModelClip** clipData= model->ClipData();
	if (tweens[instance].Curr.NextFrame >= clipData[clip]->Duration() - 1)
	return true;

	return false;
}




void Animator::CreateAnimTransform(UINT index)
{
	Matrix* bones = new Matrix[model->BoneCount()];
	
	for (UINT f = 0; f < clip[index]->FrameCount(); f++)
	{
		for (UINT b = 0; b < model->BoneCount(); b++)
		{
			
     		D3DXMatrixInverse(&invGlobal, NULL, &bone[b]->Transform());

			int parentIndex = bone[b]->ParentIndex();
			if (parentIndex < 0)
				D3DXMatrixIdentity(&parent);
			else
				parent = bones[parentIndex];


			
			ModelKeyframe* frame = clip[index]->Keyframe(bone[b]->Name());
			
			if (frame != NULL)
			{
				ModelKeyframeData data = frame->Transforms[f];
				
				D3DXMatrixScaling(&S, data.Scale.x, data.Scale.y, data.Scale.z);
				D3DXMatrixRotationQuaternion(&R, &data.Rotation);
				D3DXMatrixTranslation(&T, data.Translation.x, data.Translation.y, data.Translation.z);

				animation = S * R * T;

				bones[b] = animation * parent;

				skinTransforms[index].Transform[f][b] = invGlobal * bones[b];
			}
			else
			{
				bones[b] = parent;

				skinTransforms[index].Transform[f][b] = bone[b]->Transform()* bones[b];
			}


		}
	}
	SafeDelete(bones);
}


void Animator::CS_CalcBoneTransform()
{
	
	frameBuffer->Apply();
	sComputeFrameBuffer->SetConstantBuffer(frameBuffer->Buffer());
	//

	sCSTransformSRV->SetResource(srv);
		
	
	sUav->SetUnorderedAccessView(ColliderSystem::Get()->UAV());
	
	sharedData->Update(0);
	
}

void Animator::CS_CalcEffectTransform(const uint& particleIndex)
{
	
	frameBuffer->Apply();
	sComputeFrameBuffer->SetConstantBuffer(frameBuffer->Buffer());
	//

	sCSTransformSRV->SetResource(srv);



	sEffectUav->SetUnorderedAccessView(EffectSystem::Get()->UAV(particleIndex));
	sharedData->EffectUpdate(particleIndex);
}




