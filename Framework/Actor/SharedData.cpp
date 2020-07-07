#include "Framework.h"
#include "SharedData.h"

SharedData::SharedData()
	: actorIndex(0),drawCount(0), InstBuffer(nullptr),
	InstBufferSRV(nullptr),  culledCount(0)
	
{
	//transforms.resize(10);
	computeShader = new Shader(L"Deferred/30_Collider.fx");

	drawCountBuffer = new ConstantBuffer(&drawCountDesc, sizeof(DrawCountDesc));
	sDrawCountBuffer = computeShader->AsConstantBuffer("CB_DrawCount");

	SafeRelease(InstBuffer);
	SafeRelease(InstBufferSRV);
	//CreateTexture
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = 4;
		desc.Height = MAX_MODEL_INSTANCE;
		
	    desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		//desc.Format = DXGI_FORMAT_R32G32_FLOAT;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;

		for (UINT i = 0; i < MAX_MODEL_INSTANCE; i ++)
		{
			D3DXMatrixIdentity(&instMatrix[i]);

		}//for(i)


		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = instMatrix;
		subResource.SysMemPitch = sizeof(Matrix);
		subResource.SysMemSlicePitch = sizeof(Matrix) * MAX_MODEL_INSTANCE;

		Check(D3D::GetDevice()->CreateTexture2D(&desc, &subResource, &InstBuffer));
	}


	//Create SRV
	{
		D3D11_TEXTURE2D_DESC desc;
		InstBuffer->GetDesc(&desc);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Format = desc.Format;

		Check(D3D::GetDevice()->CreateShaderResourceView(InstBuffer, &srvDesc, &InstBufferSRV));
	}
	sInstSrv = computeShader->AsSRV("InstInput");
	

}

SharedData::~SharedData()
{
}




void SharedData::Update(const uint& pass)
{


		for (uint i = 0; i < drawCount; i++)
			memcpy(instMatrix[i], transforms[i], sizeof(Matrix));

		D3D::GetDC()->UpdateSubresource
		(
			InstBuffer,
			0,
			NULL,
			//&destRegion,
			instMatrix,
			sizeof(Matrix),
			0
		);
		sInstSrv->SetResource(InstBufferSRV);


	//Thread::Get()->AddTask([&]()
	//{
	drawCountDesc.actorIndex = actorIndex;
		
	drawCountDesc.drawCount = drawCount;
	drawCountBuffer->Apply();
	sDrawCountBuffer->SetConstantBuffer(drawCountBuffer->Buffer());
		


		uint dispatch = drawCount == 0 ? 1 : drawCount;
		computeShader->Dispatch(0, pass, dispatch, 1, 1);
	//});
}

void SharedData::EffectUpdate(const uint& particleIndex)
{


	
	drawCountDesc.particleIndex = particleIndex;
	drawCountBuffer->Apply();
	sDrawCountBuffer->SetConstantBuffer(drawCountBuffer->Buffer());
	
	

	uint dispatch = drawCount == 0 ? 1 : drawCount;
	computeShader->Dispatch(0, 2, dispatch, 1, 1);
}

void SharedData::PopTransform(const uint & index)
{
	drawCount--;
	ColliderSystem::Get()->PopDrawCount();
	culledCount++;

	transforms.erase(transforms.begin() + index);
	//transforms.shrink_to_fit();
	Index.erase(Index.begin() + index);

	
	//transforms.shrink_to_fit();
}

void SharedData::PushTransform(const uint & culledIndex, const Matrix& culledMatrix)
{
	drawCount++;
	ColliderSystem::Get()->AddDrawCount();
	culledCount--;
	Index.emplace_back(culledIndex);
	sort(Index.begin(), Index.end());

	int index = 0;
	//index=count_if(Index.begin(), Index.end(), [&](int index) {
	//	if (culledIndex > index)
	//		return true;
	//});
	for (auto& sort : Index)
	{
		if (culledIndex > sort)
		{
			index++;
		}
	}
	
	transforms.insert(transforms.begin() + index, culledMatrix);
}

void SharedData::SortIndeies(const uint & DiedIndex)
{
	

	
	for (uint y = 0; y < Index.size(); y++)
	{
		
		if(DiedIndex< static_cast<uint>(Index[y]))
		{
			Index[y] = Index[y]-1;
			
		}
		
		/*if (Index[y] > DiedIndex)
		{
			uint temp = Index[y] - DiedIndex - fator;
			Index[y] = Index[y] - temp;
			fator++;
		}*/
	}
}

bool SharedData::DeleteDiedInstance(const uint & DiedIndex)
{
	for (uint i=0;i<Index.size();i++)
	{
		if (Index[i] == DiedIndex)
		{
			

			drawCount--;
			ColliderSystem::Get()->PopDrawCount();
			transforms.erase(transforms.begin() + i);
			transforms.shrink_to_fit();
			Index.erase(Index.begin() + i);
			Index.shrink_to_fit();
			CulledTransformIndex = DiedIndex;
			DiedTransformIndex = DiedIndex;
			bNeedSortCulledTransform = true;
			bNeedSortTransform = true;
			if(!Index.empty()&&DiedIndex!= Index.back())
			SortIndeies(DiedIndex);
			return true;
		
		}
		
	}
	return false;
}

const Matrix & SharedData::TransformWithoutScale(const uint & index)
{
	Vector3 pos, scale;
	Quaternion q;

	D3DXMatrixDecompose(&scale, &q, &pos, &transforms[index]);
	Matrix S,R, T;
	D3DXMatrixTranslation(&T, pos.x, pos.y, pos.z);
	D3DXMatrixRotationQuaternion(&R, &q);
	//D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
	const Matrix& temp = R * T;
	return temp;
}
