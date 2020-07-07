#include "Framework.h"
#include "ParticleSimulation.h"
#include "Viewer/Orbit.h"
#include "Actor/SharedData.h"
ParticleSimulation::ParticleSimulation(const uint& index)
	:index(index),m_pStructuredBuffer(nullptr), m_pStructuredBufferSRV(nullptr), m_pStructuredBufferUAV(nullptr),
	Position_StructuredBuffer(nullptr), Position_StructuredBufferSRV(nullptr), Position_StructuredBufferUAV(nullptr),
	 bEditing(false), bActive(false), bPause(false), sharedData(nullptr)
{ 
	this->shader = new Shader(L"Deferred/RenderParticles.fxo");
	this->csShader = new Shader(L"Deferred/nBodyCS.fxo");
	Count = 1024;
	m_fPointSize = 5;
	//if(!this->shader)
	
	
	m_readBuffer = 0;

	

	drawBuffer = new ConstantBuffer(&drawCB, sizeof(DrawCB));
	sDrawBuffer = this->shader->AsConstantBuffer("cbDraw");

	simulateBuffer = new ConstantBuffer(&simulateCB, sizeof(SimulationParametersCB));
	sSimulateBuffer = csShader->AsConstantBuffer("SimulationParameters");

	//wstring temp = L"../../_Textures/Environment/pointsprite_grey.dds";
	//wstring temp = L"../../_Textures/;
	//wstring temp = L"../../_Textures/Environment/CherrBlossom.png";
    //wstring temp = L"../../_Textures/Environment/Smoke.dds";
	
	particleTexture = new Texture(L"Environment/fire.jpg");
	
	previewTransform = new Transform(shader);
	//////////////////////////////////////////////////////////////////////
	orbit = new Orbit();
	orbit->SetCameraType(CameraType::Orbit);
	orbit->SetOrbitTargetPosition(Vector3(0, 0, 0));
	pers = new Perspective(1280, 720);
	previewTarget = new RenderTarget(static_cast<uint>(1280), static_cast<uint>(720));
	
	buttonTextures[0] = new Texture(L"playButton.png");
	buttonTextures[1] = new Texture(L"pauseButton.png");
	buttonTextures[2] = new Texture(L"stopButton.png");

	instanceBuffer = new VertexBuffer(&instDesc, MAX_MODEL_INSTANCE, sizeof(InstDesc), 1, true);
	CreateComputeData();
	
	uavEffect = this->csShader->AsUAV("particles");
	textureEffect = this->shader->AsSRV("particleTexture");
	srvEffect = this->shader->AsSRV("g_particles");
	boneEffect = this->shader->AsSRV("EffectTransforms");
}

ParticleSimulation::~ParticleSimulation()
{
	SafeDelete(shader);
	SafeDelete(csShader);
	

	//SafeRelease(m_pParticleTex);    // Texture for displaying particles
	SafeDelete(particleTexture);


	// structured buffer
	//SafeRelease(m_pBodiesTex1D[2]);
	//SafeRelease(m_pBodiesTexSRV[2]);
		
	//SafeRelease(m_pBodiesTexUAV[2]);
	SafeRelease(m_pStructuredBuffer);
	SafeRelease(m_pStructuredBufferSRV);
	SafeRelease(m_pStructuredBufferUAV);

	SafeDelete(simulateBuffer);

	SafeDelete(drawBuffer);

	
}

void ParticleSimulation::Destroy()
{
	SafeDelete(shader);
	SafeDelete(csShader);


	//SafeRelease(m_pParticleTex);    // Texture for displaying particles
	SafeDelete(particleTexture);


	// structured buffer
	//SafeRelease(m_pBodiesTex1D[2]);
	//SafeRelease(m_pBodiesTexSRV[2]);

	//SafeRelease(m_pBodiesTexUAV[2]);
	SafeRelease(m_pStructuredBuffer);
	SafeRelease(m_pStructuredBufferSRV);
	SafeRelease(m_pStructuredBufferUAV);

	SafeDelete(simulateBuffer);

	SafeDelete(drawBuffer);

}

void ParticleSimulation::Update()
{
	if ( bPause) return;
	simulateCB.g_timestep = Time::Delta();
	//simulateCB.g_softeningSquared = 0.01f;
	simulateCB.g_numParticles = m_numBodies;
	simulateCB.g_readOffset = m_readBuffer * m_numBodies;
	//simulateCB.g_readOffset = 0;
	simulateCB.g_writeOffset = (1 - m_readBuffer) * m_numBodies;
	
	//simulateCB.g_writeOffset =  m_numBodies;
	uavEffect->SetUnorderedAccessView(m_pStructuredBufferUAV);
	
	if (simulateBuffer)
	{	
		simulateBuffer->Apply();
		sSimulateBuffer->SetConstantBuffer(simulateBuffer->Buffer());
	}
	csShader->Dispatch(0, pass,m_numBodies / 256, 1, 1);
	//csShader->Dispatch(0, pass, 1, 1, 1);
	m_readBuffer = 1 - m_readBuffer;

	runningTime += simulateCB.g_timestep;
}

void ParticleSimulation::PreviewRender()
{
	if (!bEditing||!bActive) return;
	Update();
	orbit->SetOrbitTargetPosition(Vector3(0 , 0 , 0));
	orbit->PreviewUpdate();
	orbit->GetMatrix(&orbitView);

	pers->GetMatrix(&orbitProj);
	
	previewTransform->Render();

	drawCB.ViewProjection = orbitView*orbitProj;


	drawCB.g_fPointSize = m_fPointSize;
    drawCB.g_readOffset = m_readBuffer * m_numBodies;
	//drawCB.g_readOffset =  m_numBodies;

	
	if (drawBuffer)
	{
		drawBuffer->Apply();
		sDrawBuffer->SetConstantBuffer(drawBuffer->Buffer());
	}
	textureEffect->SetResource(particleTexture->SRV());
	srvEffect->SetResource(m_pStructuredBufferSRV);
	previewTarget->Set(nullptr);

	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	if(timer >runningTime)
	shader->Draw(0, 1, m_numBodies);
	
}

void ParticleSimulation::Render()
{

	
	
	Update();
	
	if (!indices.empty())
	{
		for (uint i = 0; i < indices.size(); i++)
		{
			ImGui::InputInt("indices", (int*)&indices[i]);
			ImGui::InputInt("culledIndex", (int*)&sharedData->CulledTransformIndex);
			
			if (sharedData->bNeedSortTransform)
			{
				if( static_cast<uint>(sharedData->DiedTransformIndex)< indices[i])
				indices[i] = indices[i] - 1;

				sharedData->bNeedSortTransform = false;
				sharedData->DiedTransformIndex = 100;
				
			}
			if(sharedData->transforms.size()> indices[i])
			memcpy(instDesc[i].worlds, sharedData->transforms[indices[i]], sizeof(Matrix));
			instDesc[i].factor = Vector4(
				 0,
				 0,
				 0,
				indices[i]);
			//instanceIndex = indices[i];
			
		}

		D3D11_MAPPED_SUBRESOURCE subResource;
		D3D::GetDC()->Map(instanceBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
		{
			memcpy(subResource.pData, &instDesc, sizeof(InstDesc)*indices.size());
		}
		D3D::GetDC()->Unmap(instanceBuffer->Buffer(), 0);
	}
	
	
	//D3D::GetDC()->UpdateSubresource
	//(
	//	instanceBuffer->Buffer(),
	//	0,
	//	NULL,
	//	//&destRegion,
	//	instDesc,
	//	sizeof(InstDesc)* indices.size(),
	//	0
	//);
	

	drawCB.ViewProjection = Context::Get()->View()*Context::Get()->Projection();
	drawCB.g_fPointSize = m_fPointSize;
	drawCB.g_readOffset = m_readBuffer * m_numBodies;
	

	
	if (drawBuffer)
	{
		drawBuffer->Apply();
		sDrawBuffer->SetConstantBuffer(drawBuffer->Buffer());
	}
	textureEffect->SetResource(particleTexture->SRV());
	srvEffect->SetResource(m_pStructuredBufferSRV);
	boneEffect->SetResource(Position_StructuredBufferSRV);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	instanceBuffer->Render();

	if (timer > runningTime)
	shader->DrawInstanced(0, 0, m_numBodies, indices.size(), 0, 0);
	else
	{
		if (!indices.empty())
		{
			indices.erase(indices.begin());
			indices.shrink_to_fit();
			
		}
		EffectSystem::Get()->SetActivated(false);
	}
	//shader->Draw(0, 0, m_numBodies);
	
	
	
}

void ParticleSimulation::ResetBodies(const int & instance)
{
	if (!bFirst&&timer > runningTime) return;

	if (instance > -1)
	{
		
		indices.emplace_back(instance);
	}
		
	bFirst = false;
	runningTime = 0.0f;
	m_numBodies = bodyData.nBodies;

	D3DXVECTOR4 *particleArray = new D3DXVECTOR4[m_numBodies * 3];
	for (unsigned int i = 0; i < m_numBodies; i++) {
		particleArray[i] = D3DXVECTOR4(bodyData.position[i * 3 + 0],
			bodyData.position[i * 3 + 1],
			bodyData.position[i * 3 + 2],
			1.0f);
		
		particleArray[i + m_numBodies] = particleArray[i];
		particleArray[i + 2 * m_numBodies] = D3DXVECTOR4(bodyData.velocity[i * 3 + 0],
			bodyData.velocity[i * 3 + 1],
			bodyData.velocity[i * 3 + 2],
			1.0f);
	}

	D3D11_SUBRESOURCE_DATA initData = { particleArray, 0, 0 };

	SafeRelease(m_pStructuredBuffer);
	SafeRelease(m_pStructuredBufferSRV);
	SafeRelease(m_pStructuredBufferUAV);

	// Create Structured Buffer
	D3D11_BUFFER_DESC sbDesc;
	sbDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	sbDesc.CPUAccessFlags = 0;
	sbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	sbDesc.StructureByteStride = sizeof(D3DXVECTOR4);
	sbDesc.ByteWidth = sizeof(D3DXVECTOR4) * m_numBodies * 3;
	sbDesc.Usage = D3D11_USAGE_DEFAULT;
	Check(D3D::GetDevice()->CreateBuffer(&sbDesc, &initData, &m_pStructuredBuffer));

	// create the Shader Resource View (SRV) for the structured buffer
	D3D11_SHADER_RESOURCE_VIEW_DESC sbSRVDesc;
	sbSRVDesc.Buffer.ElementOffset = 0;
	sbSRVDesc.Buffer.ElementWidth = sizeof(D3DXVECTOR4);
	sbSRVDesc.Buffer.FirstElement = 0;
	sbSRVDesc.Buffer.NumElements = m_numBodies * 3;
	sbSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	sbSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	Check(D3D::GetDevice()->CreateShaderResourceView(m_pStructuredBuffer, &sbSRVDesc, &m_pStructuredBufferSRV));

	// create the UAV for the structured buffer
	D3D11_UNORDERED_ACCESS_VIEW_DESC sbUAVDesc;
	sbUAVDesc.Buffer.FirstElement = 0;
	sbUAVDesc.Buffer.Flags = 0;
	sbUAVDesc.Buffer.NumElements = m_numBodies * 3;
	sbUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	sbUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	Check(D3D::GetDevice()->CreateUnorderedAccessView(m_pStructuredBuffer, &sbUAVDesc, &m_pStructuredBufferUAV));

	SafeDeleteArray( particleArray);
}

void ParticleSimulation::ImageButton()
{
	if (ImGui::ImageButton(previewTarget->SRV() ? previewTarget->SRV() : nullptr, ImVec2(80, 80)))
	{

		bEditing = true;
	}
	
	Editor();
	
}

void ParticleSimulation::ResidenceSharedData(SharedData * sharedData)
{
	if (this->sharedData != sharedData)
	{
		this->sharedData = sharedData;
	}
}

void ParticleSimulation::CreateComputeData()
{
	SafeRelease(Position_StructuredBuffer);
	SafeRelease(Position_StructuredBufferSRV);
	SafeRelease(Position_StructuredBufferUAV);


	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = 4;
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
			D3DXMatrixIdentity(&csTexture[i].matrix);

		}
	}


	D3D11_SUBRESOURCE_DATA subResource;
	subResource.pSysMem = csTexture;
	subResource.SysMemPitch = sizeof(CS_TextureOutputDesc);
	subResource.SysMemSlicePitch = sizeof(Matrix) * MAX_MODEL_INSTANCE;

	Check(D3D::GetDevice()->CreateTexture2D(&desc, &subResource, &Position_StructuredBuffer));

	//Create SRV
	{


		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Format = desc.Format;

		Check(D3D::GetDevice()->CreateShaderResourceView(Position_StructuredBuffer, &srvDesc, &Position_StructuredBufferSRV));
	}


	// create the UAV for the structured buffer
	D3D11_UNORDERED_ACCESS_VIEW_DESC sbUAVDesc;
	sbUAVDesc.Buffer.FirstElement = 0;
	sbUAVDesc.Buffer.Flags = 0;
	sbUAVDesc.Buffer.NumElements = 4 * MAX_MODEL_INSTANCE;
	sbUAVDesc.Format = desc.Format;
	sbUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	Check(D3D::GetDevice()->CreateUnorderedAccessView(Position_StructuredBuffer, &sbUAVDesc, &Position_StructuredBufferUAV));
	SafeDelete(csTexture);
}

void ParticleSimulation::Editor()
{
	if (!bEditing) return;

	ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints
	(
		ImVec2(800, 600),
		ImVec2(1280, 720)
	);
	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove;

	//ImGuiWindowFlags_MenuBar|

	ImGui::Begin("Editor", &bEditing, windowFlags);
	{

		ImVec2 size = ImGui::GetWindowSize();


		ShowFrame(size);

		SelectParticleType(size);


	}
	ImGui::End();
}

void ParticleSimulation::ShowFrame(const ImVec2 & size)
{
	ImGui::BeginChild("##Frame", ImVec2(size.x*0.5f, 0), true, ImGuiWindowFlags_NoScrollbar);
	{

		pers->Set(size.x * 0.5f, size.y);
		ImGui::Image
		(
			previewTarget->SRV() ? previewTarget->SRV() : nullptr, ImVec2(size.x*0.5f, size.y)
		);
		

	}
	ImGui::EndChild();

}

void ParticleSimulation::SelectParticleType(const ImVec2 & size)
{
	ImGui::SameLine();
	ImGui::BeginChild("##ParticleType", ImVec2(size.x*0.5f - 70.0f, 0), true);
	{


		if (ImGui::CollapsingHeader("ParticleType", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::Button("Boom", ImVec2(80, 30)) )
			{
				
				Boom_InitBodies(Count);
				ResetBodies(-1);
				pass = 0;
				bActive = true;
				
				runningTime = 0.0f;

			}
			ImGui::SameLine(100);
			if (ImGui::Button("GumFire", ImVec2(80, 30)))
			{
				
				GunFire_InitBodies(Count);
				ResetBodies(-1);
				pass = 1;
				bActive = true;
				runningTime = 0.0f;
				
			}
			ImGui::SameLine(200);

			if (ImGui::Button("Blood", ImVec2(80, 30)))
			{

				Blood_InitBodies(Count);
				ResetBodies(-1);
				pass = 2;
				bActive = true;
				runningTime = 0.0f;

			}
			ImGui::SameLine(300);

			if (ImGui::ImageButton
			(
				particleTexture ? particleTexture->SRV() : nullptr,
				ImVec2(80, 80)

			))
			{
				LoadTexture();
			}
			
			ImGui::InputInt("NumBodies", (int*)&Count);
			ImGui::InputFloat("Squared", &simulateCB.g_softeningSquared, 0.1f);
		    ImGui::InputFloat("clusterScale", &clusterScale, 0.1f);
		    ImGui::InputFloat("velocityScale", &velocityScale, 0.1f);
			ImGui::InputFloat("pontSize", &m_fPointSize, 1.0f);
			ImGui::InputFloat("Distance", &simulateCB.distance, 1.0f);
			ImGui::InputFloat("Time", &timer, 1.0f);

			if (ImGui::ImageButton(buttonTextures[0]->SRV(), ImVec2(20.0f, 20.0f)))//play
			{

				bPause=false;
			}
			ImGui::SameLine();
			if (ImGui::ImageButton(buttonTextures[1]->SRV(), ImVec2(20.0f, 20.0f)))//pause
			{

				bPause = true;
			}
			ImGui::SameLine();
			if (ImGui::ImageButton(buttonTextures[2]->SRV(), ImVec2(20.0f, 20.0f)))//stop
			{

			}
		}
		

	}

	ImGui::EndChild();
}

void ParticleSimulation::LoadTexture()
{
	function<void(wstring)> f;
	HWND hWnd = NULL;
	f = bind(&ParticleSimulation::SetTexture, this, placeholders::_1);
	Path::OpenFileDialog(L"", Path::EveryFilter, L"../../_Textures/", f, hWnd);
}

void ParticleSimulation::SetTexture(const wstring & file)
{
	SafeDelete(particleTexture);
	particleTexture = new Texture(file);
}

void ParticleSimulation::Boom_InitBodies(uint numBodies)
{
	/*if (bodyData.nBodies == numBodies)
	{
		return;
	}*/

	// Free previous data
	SafeDeleteArray(bodyData.position);
	SafeDeleteArray(bodyData.velocity);
	
	// Allocate new data
	bodyData.position = new float[numBodies * 3];
	bodyData.velocity = new float[numBodies * 3];
	
	bodyData.nBodies = numBodies;

	float scale = clusterScale;
	float vscale =  scale * velocityScale;
	float inner =  2.5f * scale;
	float outer =  4.0f * scale;

	int p = 0, v = 0, t = 0;
	
	unsigned int i = 0;


	while (i < numBodies)
	{
		float x, y, z;
		x = rand() / (float)RAND_MAX * 2 - 1;
		y = rand() / (float)RAND_MAX * 2 - 1;
		z = rand() / (float)RAND_MAX * 2 - 1;



		D3DXVECTOR3 point(x, y, z);
		float len = D3DXVec3Length(&point);
		D3DXVec3Normalize(&point, &point);
		if (len > 1)
			continue;

		bodyData.position[p++] =0.0f;// point.x*(inner + (outer - inner) * rand() / (float)RAND_MAX);
		bodyData.position[p++] =0.0f;// point.y*(inner + (outer - inner) * rand() / (float)RAND_MAX);
		bodyData.position[p++] =0.0f;// point.z*(inner + (outer - inner) * rand() / (float)RAND_MAX);


		
		
		//g_BodyData.position[p++] =0.0f ;
	  //  g_BodyData.position[p++] =0.0f ;
		//float temp= Math::Clamp(point.z, 0, 1);
		//g_BodyData.position[p++] = temp;


		D3DXVECTOR3 axis(x, y, z);
		D3DXVec3Normalize(&axis, &axis);

		if (1 - D3DXVec3Dot(&point, &axis) < 1e-6)
		{
			axis.x = point.y;
			axis.y = point.x;
			D3DXVec3Normalize(&axis, &axis);
		}
		//if (point.y < 0) axis = scalevec(axis, -1);
		D3DXVECTOR3 vv(bodyData.position[3 * i], bodyData.position[3 * i + 1], bodyData.position[3 * i + 2]);
		D3DXVec3Cross(&vv, &vv, &axis);
		/*bodyData.velocity[v++] = vv.x * vscale;
		bodyData.velocity[v++] = vv.y * vscale;
		bodyData.velocity[v++] = vv.z * vscale;*/
		bodyData.velocity[v++] = rand() / (float)RAND_MAX * 2 - 1;
		bodyData.velocity[v++] = rand() / (float)RAND_MAX * 2 - 1;
		bodyData.velocity[v++] = rand() / (float)RAND_MAX * 2 - 1;


		i++;
	}
}

void ParticleSimulation::GunFire_InitBodies(uint numBodies)
{
	/*if (bodyData.nBodies == numBodies)
	{
		return;
	}*/

	// Free previous data
	SafeDeleteArray(bodyData.position);
	SafeDeleteArray(bodyData.velocity);

	
	// Allocate new data
	bodyData.position = new float[numBodies * 3];
	bodyData.velocity = new float[numBodies * 3];
	
	bodyData.nBodies = numBodies;
	float scale = clusterScale;
	float vscale = scale * velocityScale;
	float inner = 2.5f * scale;
	float outer = 4.0f * scale;
	int p = 0, v = 0;
	unsigned int i = 0;
	while (i < numBodies)
	{
		float x, y, z;
		x = rand() / (float)RAND_MAX * 2 - 1;
		y = rand() / (float)RAND_MAX * 2 - 1;
		z = rand() / (float)RAND_MAX * 2 - 1;



		D3DXVECTOR3 point(x, y, z);
		float len = D3DXVec3Length(&point);
		D3DXVec3Normalize(&point, &point);
		if (len > 1)
			continue;

		bodyData.position[p++] =0.f;// *(inner + (outer - inner) * rand() / (float)RAND_MAX);
		bodyData.position[p++] =0.f;// *(inner + (outer - inner) * rand() / (float)RAND_MAX);
		bodyData.position[p++] =0.f;// *(inner + (outer - inner) * rand() / (float)RAND_MAX);

		
		//D3DXVECTOR3 axis(x, y, z);
		//D3DXVec3Normalize(&axis, &axis);

		//if (1 - D3DXVec3Dot(&point, &axis) < 1e-6)
		//{
		//	axis.x = point.y;
		//	axis.y = point.x;
		//	D3DXVec3Normalize(&axis, &axis);
		//}
		//////if (point.y < 0) axis = scalevec(axis, -1);
		//D3DXVECTOR3 vv(bodyData.position[3 * i], bodyData.position[3 * i + 1], bodyData.position[3 * i + 2]);
		//D3DXVec3Cross(&vv, &vv, &axis);
		bodyData.velocity[v++] = (rand() / ((float)RAND_MAX * 2 - 1))*clusterScale;
		bodyData.velocity[v++] = (rand() / ((float)RAND_MAX * 2 - 1))*clusterScale;
		bodyData.velocity[v++] = Math::Abs((rand() / ((float)RAND_MAX * 2 - 1)))*velocityScale;
		
		i++;
	}
}

void ParticleSimulation::Blood_InitBodies(uint numBodies)
{
	// Free previous data
	SafeDeleteArray(bodyData.position);
	SafeDeleteArray(bodyData.velocity);
	
	// Allocate new data
	bodyData.position = new float[numBodies * 3];
	bodyData.velocity = new float[numBodies * 3];

	bodyData.nBodies = numBodies;

	
	float vscale = clusterScale * velocityScale;
	float inner = 2.5f * clusterScale;
	float outer = 4.0f * clusterScale;

	int p = 0, v = 0;
	unsigned int i = 0;


	while (i < numBodies)
	{
		float x, y, z;
		x = (rand() / (float)RAND_MAX * 2 - 1);
		y = (rand() / (float)RAND_MAX * 2 - 1);
		z = (rand() / (float)RAND_MAX * 2 - 1);



		D3DXVECTOR3 point(x, y, z);
		float len = D3DXVec3Length(&point);
		D3DXVec3Normalize(&point, &point);
		if (len > 1)
			continue;


		bodyData.position[p++] = 0.f;// *(inner + (outer - inner) * rand() / (float)RAND_MAX);
		bodyData.position[p++] = 0.f;// *(inner + (outer - inner) * rand() / (float)RAND_MAX);
		bodyData.position[p++] = 0.f;// *(inner + (outer - inner) * rand() / (float)RAND_MAX);


		//D3DXVECTOR3 axis(x, y, z);
		//D3DXVec3Normalize(&axis, &axis);

		//if (1 - D3DXVec3Dot(&point, &axis) < 1e-6)
		//{
		//	axis.x = point.y;
		//	axis.y = point.x;
		//	D3DXVec3Normalize(&axis, &axis);
		//}
		//////if (point.y < 0) axis = scalevec(axis, -1);
		//D3DXVECTOR3 vv(bodyData.position[3 * i], bodyData.position[3 * i + 1], bodyData.position[3 * i + 2]);
		//D3DXVec3Cross(&vv, &vv, &axis);
		bodyData.velocity[v++] = (rand() / ((float)RAND_MAX * 2 - 1))*clusterScale;
		bodyData.velocity[v++] = (rand() / ((float)RAND_MAX * 2 - 1))*clusterScale;
		bodyData.velocity[v++] = Math::Abs((rand() / ((float)RAND_MAX * 2 - 1)))*velocityScale;

		i++;
	}
}


