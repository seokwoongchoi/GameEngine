#include "Framework.h"
#include "ColliderSystem.h"


ColliderSystem* ColliderSystem::instance = NULL;

ColliderSystem * ColliderSystem::Get()
{
	return instance;
}

void ColliderSystem::Create()
{
	assert(instance == NULL);

	instance = new ColliderSystem();
}

void ColliderSystem::Delete()
{
	SafeDelete(instance);
}

ColliderSystem::ColliderSystem()
	:csShader(nullptr), bStart(false), drawCount(0), IsDameged(false), IsClac(false)
{
	csShader = new Shader(L"Deferred/CollisonCS.fx");

	rayBuffer = new ConstantBuffer(&rayDesc, sizeof(RayDesc));
	sRayBuffer = csShader->AsConstantBuffer("CB_Ray");

	
	sSrv = csShader->AsSRV("Input");
	sUav = csShader->AsUAV("Output");
	sCopyUav = csShader->AsUAV("CopyOutput");
	

	//float Range1 = 1.0f;
	//for (int i = 0; i < 30; i++)
	//{
	//	xy[i] = Vector3(Range1 * cosf(D3DXToRadian(i * 12)), Range1 * sinf(D3DXToRadian(i * 12)), 0);
	//	xz[i] = Vector3(Range1 * cosf(D3DXToRadian(i * 12)), 0, Range1 * sinf(D3DXToRadian(i * 12)));
	//	yz[i] = Vector3(0, Range1 * cosf(D3DXToRadian(i * 12)), Range1 * sinf(D3DXToRadian(i * 12)));
	//}
	D3DXMatrixIdentity(&world);

	dest[0] = Vector3(boneBoxMin.x, boneBoxMin.y, boneBoxMax.z);
	dest[1] = Vector3(boneBoxMax.x, boneBoxMin.y, boneBoxMax.z);
	dest[2] = Vector3(boneBoxMin.x, boneBoxMax.y, boneBoxMax.z);
	dest[3] = Vector3(boneBoxMax);
	dest[4] = Vector3(boneBoxMin);
	dest[5] = Vector3(boneBoxMax.x, boneBoxMin.y, boneBoxMin.z);
	dest[6] = Vector3(boneBoxMin.x, boneBoxMax.y, boneBoxMin.z);
	dest[7] = Vector3(boneBoxMax.x, boneBoxMax.y, boneBoxMin.z);

}

ColliderSystem::~ColliderSystem()
{
}


void ColliderSystem::Update()
{
	if (!bStart||!IsClac) return;

	
	//Context::Get()->GetViewport()->GetRay(&org, &dir, world, V, P);

	GetAimRay(&org, &dir);
		
	rayDesc.org = org;
	rayDesc.drawCount = drawCount;
	rayDesc.dir = dir;
	//Thread::Get()->AddTask([&]()
	//{
	rayBuffer->Apply();
	sRayBuffer->SetConstantBuffer(rayBuffer->Buffer());

	
	sSrv->SetResource(out_StructuredBufferSRV);
	//sUav->SetUnorderedAccessView(uav);
	sCopyUav->SetUnorderedAccessView(copy_StructuredBufferUAV);
	
		QueryPerformanceCounter((LARGE_INTEGER *)&currentTime);
		if (chrono::_Is_even<uint>(currentTime))
		{

			csShader->Dispatch(0, 0, drawCount, 1, 1);

			CopyResource();

		}
	//});
	
	
	DebugLine::Get()->Pass(1);
	DebugLine::Get()->SRV(out_StructuredBufferSRV);
	
	
	
	
	switch (state)
	{
	
	case MainActorState::Move:
	{
		
		for (uint i = 0; i < drawCount; i++)
		{
			CS_CopyDesc& data = csCopyOutput[i];
			if (data.Indices.w != 0) continue;
			if (data.Factor.z == 1)
			{

				EventSystem::Get()->Collison(data.Indices.x, data.Indices.y,
					Vector3(data.Direction.x, data.Direction.y, data.Direction.z));
				

			}
		}
		
			
		
	}
		break;
	case MainActorState::Fire:
	{
		
		for (uint i = 0; i < drawCount; i++)
		{
			CS_CopyDesc& data = csCopyOutput[i];
			if (data.Indices.w != 0) continue;

			if (data.Factor.y == 1)
			{

				EventSystem::Get()->Events["Dying"](data.Indices.x, data.Indices.y);
				


			}
			else if (data.Factor.x == 1)
			{


				EventSystem::Get()->Events["Reaction"](data.Indices.x, data.Indices.y);
				
			}
		}
	}
	case MainActorState::WalkFire:
	{
		
		for (uint i = 0; i < drawCount; i++)
		{
			CS_CopyDesc& data = csCopyOutput[i];
			if (data.Indices.w != 0) continue;
			if (data.Factor.y == 1)
			{

				EventSystem::Get()->Events["Dying"](data.Indices.x, data.Indices.y);
			}
			else if (data.Factor.x == 1)
			{


				EventSystem::Get()->Events["Reaction"](data.Indices.x, data.Indices.y);
				

			}
			if (data.Factor.z == 1)
			{

				EventSystem::Get()->Collison(data.Indices.x, data.Indices.y,
					Vector3(data.Direction.x, data.Direction.y, data.Direction.z));
			}
		}
		
	}
	break;

		
	}

	for (uint i = 0; i < drawCount; i++)
	{

		//if (csCopyOutput[i].factor.z == 1)
		//{
			//cout << csCopyOutput[i].factor.x; cout << " 번째 Actor, ";
			//cout << csCopyOutput[i].factor.y; cout << " 번째 Instance" << endl;
			//cout << "  " << endl;
			//cout << csCopyOutput[i].factor.x << endl;

		//}
		color = csCopyOutput[i].Factor.x == 1 || csCopyOutput[i].Factor.z == 1 ? Color(1, 0, 0, 1) : Color(0, 1, 0, 1);


		//Front
		DebugLine::Get()->RenderLine(dest[0], dest[1], color, i);
		DebugLine::Get()->RenderLine(dest[1], dest[3], color, i);
		DebugLine::Get()->RenderLine(dest[3], dest[2], color, i);
		DebugLine::Get()->RenderLine(dest[2], dest[0], color, i);

		//Backward
		DebugLine::Get()->RenderLine(dest[4], dest[5], color, i);
		DebugLine::Get()->RenderLine(dest[5], dest[7], color, i);
		DebugLine::Get()->RenderLine(dest[7], dest[6], color, i);
		DebugLine::Get()->RenderLine(dest[6], dest[4], color, i);

		//Side
		DebugLine::Get()->RenderLine(dest[0], dest[4], color, i);
		DebugLine::Get()->RenderLine(dest[1], dest[5], color, i);
		DebugLine::Get()->RenderLine(dest[2], dest[6], color, i);
		DebugLine::Get()->RenderLine(dest[3], dest[7], color, i);

		color = csCopyOutput[i].Factor.y == 1 ? Color(1, 0, 0, 1) : Color(0, 1, 0, 1);
		//Front
		DebugLine::Get()->RenderLine(dest[0], dest[1], color, i);
		DebugLine::Get()->RenderLine(dest[1], dest[3], color, i);
		DebugLine::Get()->RenderLine(dest[3], dest[2], color, i);
		DebugLine::Get()->RenderLine(dest[2], dest[0], color, i);

		//Backward
		DebugLine::Get()->RenderLine(dest[4], dest[5], color, i);
		DebugLine::Get()->RenderLine(dest[5], dest[7], color, i);
		DebugLine::Get()->RenderLine(dest[7], dest[6], color, i);
		DebugLine::Get()->RenderLine(dest[6], dest[4], color, i);

		//Side
		DebugLine::Get()->RenderLine(dest[0], dest[4], color, i);
		DebugLine::Get()->RenderLine(dest[1], dest[5], color, i);
		DebugLine::Get()->RenderLine(dest[2], dest[6], color, i);
		DebugLine::Get()->RenderLine(dest[3], dest[7], color, i);

	}
}




void ColliderSystem::CreateComputeDesc()
{

	
	IsClac = true;
	SafeRelease(out_StructuredBuffer);
	SafeRelease(out_StructuredBufferSRV);
	SafeRelease(out_StructuredBufferUAV);


	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = 10;
	desc.Height = MAX_MODEL_INSTANCE;
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;

	UINT outSize = MAX_MODEL_INSTANCE;
	if (csTexture == NULL)
	{
		csTexture = new CS_TextureOutputDesc[outSize];

		for (UINT i = 0; i < outSize; i++)
		{
			D3DXMatrixIdentity(&csTexture[i].body);
			D3DXMatrixIdentity(&csTexture[i].head);
			csTexture[i].factor = Vector4(0, 0, 0, 0);
		}
	}


	D3D11_SUBRESOURCE_DATA subResource;
	subResource.pSysMem = csTexture;
	subResource.SysMemPitch = sizeof(CS_TextureOutputDesc);
	subResource.SysMemSlicePitch = sizeof(CS_TextureOutputDesc) * MAX_MODEL_INSTANCE;

	Check(D3D::GetDevice()->CreateTexture2D(&desc, &subResource, &out_StructuredBuffer));
	
	//Create SRV
	{
		

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Format = desc.Format;

		Check(D3D::GetDevice()->CreateShaderResourceView(out_StructuredBuffer, &srvDesc, &out_StructuredBufferSRV));
	}

	
	// create the UAV for the structured buffer
	D3D11_UNORDERED_ACCESS_VIEW_DESC sbUAVDesc;
	sbUAVDesc.Buffer.FirstElement = 0;
	sbUAVDesc.Buffer.Flags = 0;
	sbUAVDesc.Buffer.NumElements = MAX_MODEL_INSTANCE*10;
	sbUAVDesc.Format = desc.Format;
	sbUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	Check(D3D::GetDevice()->CreateUnorderedAccessView(out_StructuredBuffer, &sbUAVDesc, &out_StructuredBufferUAV));
	SafeDelete(csTexture);
	CreateCopyBuffer();
	//
	//UINT outSize = MAX_MODEL_INSTANCE;
	//if (csOutput == NULL)
	//{
	//	csOutput = new CS_AnimOutputDesc[outSize];

	//	for (UINT i = 0; i < outSize; i++)
	//	{
	//		D3DXMatrixIdentity(&csOutput[i].Result);
	//		csOutput[i].boneColor = Color(0, 0, 0, 0);
	//	}
	//}
	////ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
	//D3D11_SUBRESOURCE_DATA Data = { csOutput ,0,0 };

	//SafeRelease(out_StructuredBuffer);
	//SafeRelease(out_StructuredBufferSRV);
	//SafeRelease(out_StructuredBufferUAV);


	//// Create Structured Buffer
	//D3D11_BUFFER_DESC sbDesc;

	//sbDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	//sbDesc.CPUAccessFlags = 0;
	//sbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	//sbDesc.StructureByteStride = sizeof(CS_AnimOutputDesc);
	//sbDesc.ByteWidth = sizeof(CS_AnimOutputDesc) *outSize;
	//sbDesc.Usage = D3D11_USAGE_DEFAULT;
	//Check(D3D::GetDevice()->CreateBuffer(&sbDesc, &Data, &out_StructuredBuffer));

	//// create the Shader Resource View (SRV) for the structured buffer
	//D3D11_SHADER_RESOURCE_VIEW_DESC sbSRVDesc;

	//sbSRVDesc.Buffer.ElementOffset = 0;
	//sbSRVDesc.Buffer.ElementWidth = sizeof(CS_AnimOutputDesc);
	//sbSRVDesc.Buffer.FirstElement = 0;
	//sbSRVDesc.Buffer.NumElements = outSize;
	//sbSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	//sbSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	//Check(D3D::GetDevice()->CreateShaderResourceView(out_StructuredBuffer, &sbSRVDesc, &out_StructuredBufferSRV));
	//
	//// create the UAV for the structured buffer
	//D3D11_UNORDERED_ACCESS_VIEW_DESC sbUAVDesc;
	//sbUAVDesc.Buffer.FirstElement = 0;
	//sbUAVDesc.Buffer.Flags = 0;
	//sbUAVDesc.Buffer.NumElements = outSize;
	//sbUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	//sbUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	//Check(D3D::GetDevice()->CreateUnorderedAccessView(out_StructuredBuffer, &sbUAVDesc, &out_StructuredBufferUAV));
}

//void ColliderSystem::CreateStructuredBuffer()
//{
	//UINT outSize = MAX_MODEL_INSTANCE;
	//if (csOutput2== NULL)
	//{
	//	csOutput2 = new CS_OutputDesc[outSize];

	//	for (UINT i = 0; i < outSize; i++)
	//	{
	//		D3DXMatrixIdentity(&csOutput2[i].Result);
	//		//csOutput2[i].Position = Vector4(0, 0, 0, 0);
	//		csOutput2[i].boneColor = Color(0, 0, 0, 0);

	//	}

	//	

	//}
	////ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
	//D3D11_SUBRESOURCE_DATA Data = { csOutput2 ,0,0 };

	//SafeRelease(buffer);
	//SafeRelease(srv);
	//SafeRelease(uav);


	//// Create Structured Buffer
	//D3D11_BUFFER_DESC sbDesc;

	//sbDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	//sbDesc.CPUAccessFlags = 0;
	//sbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	//sbDesc.StructureByteStride = sizeof(CS_OutputDesc);
	//sbDesc.ByteWidth = sizeof(CS_OutputDesc) *outSize;
	//sbDesc.Usage = D3D11_USAGE_DEFAULT;
	//Check(D3D::GetDevice()->CreateBuffer(&sbDesc, &Data, &buffer));

	//// create the Shader Resource View (SRV) for the structured buffer
	//D3D11_SHADER_RESOURCE_VIEW_DESC sbSRVDesc;

	//sbSRVDesc.Buffer.ElementOffset = 0;
	//sbSRVDesc.Buffer.ElementWidth = sizeof(CS_OutputDesc);
	//sbSRVDesc.Buffer.FirstElement = 0;
	//sbSRVDesc.Buffer.NumElements = outSize;
	//sbSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	//sbSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	//Check(D3D::GetDevice()->CreateShaderResourceView(buffer, &sbSRVDesc, &srv));

	//// create the UAV for the structured buffer
	//D3D11_UNORDERED_ACCESS_VIEW_DESC sbUAVDesc;
	//sbUAVDesc.Buffer.FirstElement = 0;
	//sbUAVDesc.Buffer.Flags = 0;
	//sbUAVDesc.Buffer.NumElements = outSize;
	//sbUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	//sbUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	//Check(D3D::GetDevice()->CreateUnorderedAccessView(buffer, &sbUAVDesc, &uav));
//}

void ColliderSystem::CreateCopyBuffer()
{
	UINT outSize = MAX_MODEL_INSTANCE;
	if (csCopyOutput == NULL)
	{
		csCopyOutput = new CS_CopyDesc[outSize];

		for (UINT i = 0; i < outSize; i++)
		{

			csCopyOutput[i].Indices = Vector4(-1, -1, -1, -1);
			csCopyOutput[i].Factor = Vector4(0, 0, 0, 0);
			csCopyOutput[i].Direction = Vector4(0, 0, 0, 0);
		}
	}

	
	if (csCopyInput == NULL)
	{
		csCopyInput = new CS_CopyDesc[outSize];

		for (UINT i = 0; i < outSize; i++)
		{

			csCopyOutput[i].Indices = Vector4(-1, -1, -1, -1);
			csCopyOutput[i].Factor = Vector4(0, 0, 0, 0);
			csCopyOutput[i].Direction = Vector4(0, 0, 0, 0);
		}
	}
	//ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
	D3D11_SUBRESOURCE_DATA Data = { csCopyInput ,0,0 };

	SafeRelease(copy_StructuredBuffer);

	SafeRelease(copy_StructuredBufferUAV);



	// Create Structured Buffer
	D3D11_BUFFER_DESC sbDesc;


	sbDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	sbDesc.CPUAccessFlags = 0;
	sbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	sbDesc.StructureByteStride = sizeof(CS_CopyDesc);
	sbDesc.ByteWidth = sizeof(CS_CopyDesc) *outSize;
	sbDesc.Usage = D3D11_USAGE_DEFAULT;
	Check(D3D::GetDevice()->CreateBuffer(&sbDesc, &Data, &copy_StructuredBuffer));

	//D3D11_SHADER_RESOURCE_VIEW_DESC sbSRVDesc;

 //   sbSRVDesc.Buffer.ElementOffset = 0;
 //   sbSRVDesc.Buffer.ElementWidth = sizeof(CS_CopyDesc);
 //   sbSRVDesc.Buffer.FirstElement = 0;
 //   sbSRVDesc.Buffer.NumElements = outSize;
 //   sbSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
 //   sbSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
 //   Check(D3D::GetDevice()->CreateShaderResourceView(copy_StructuredBuffer, &sbSRVDesc, &copy_StructuredBufferSRV));
	
	// create the UAV for the structured buffer
	D3D11_UNORDERED_ACCESS_VIEW_DESC sbUAVDesc;
	sbUAVDesc.Buffer.FirstElement = 0;
	sbUAVDesc.Buffer.Flags = 0;
	sbUAVDesc.Buffer.NumElements = outSize;
	sbUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	sbUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	Check(D3D::GetDevice()->CreateUnorderedAccessView(copy_StructuredBuffer, &sbUAVDesc, &copy_StructuredBufferUAV));
	


	D3D11_BUFFER_DESC desc;
	copy_StructuredBuffer->GetDesc(&desc);
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;
	Check(D3D::GetDevice()->CreateBuffer(&desc, nullptr, &copyBuffer));
	
}

void ColliderSystem::CopyResource()
{
	D3D::GetDC()->CopyResource(copyBuffer, copy_StructuredBuffer);


	D3D::GetDC()->Map(copyBuffer, 0, D3D11_MAP_READ, 0, &subResource);
	{
		memcpy(csCopyOutput, subResource.pData, sizeof(CS_CopyDesc)* drawCount);
	}
	D3D::GetDC()->Unmap(copyBuffer, 0);
}

void ColliderSystem::GetAimRay(OUT Vector3 * position, OUT Vector3 * direction)
{


	V = Context::Get()->View();
	P = Context::Get()->Projection();



	//Inv Viewport
	{
		point.x = (((2.0f * 640) / 1280) - 1.0f);
		point.y = (((2.0f * 400) / 720) - 1.0f)*-1.0f;

	}

	//Inv Projection
	{
		point.x = point.x / P._11;
		point.y = point.y / P._22;
	}

	
	//inv View
	{
		
		D3DXMatrixInverse(&invView, nullptr, &V);

		cameraPosition = Vector3(invView._41, invView._42, invView._43);

		D3DXVec3TransformNormal(direction, &Vector3(point.x, point.y, 1), &invView);
		D3DXVec3Normalize(direction, direction);
	}
	//inv world
	{
		
		D3DXMatrixInverse(&invWorld, nullptr, &world);

		D3DXVec3TransformCoord(position, &cameraPosition, &invWorld); //직선이 맞을 물체와 카메라의 공간을 일치시켜주기위해서 
		D3DXVec3TransformNormal(direction, direction, &invWorld);//직선이 맞을 물체와 카메라의 공간을 일치시켜주기위해서
		D3DXVec3Normalize(direction, direction);
		D3DXVec3TransformCoord(position, position, &invWorld);
	}
}
