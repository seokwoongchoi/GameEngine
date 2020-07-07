#include "Framework.h"
#include "PerFrame.h"
#include "Viewer/Orbit.h"

PerFrame::PerFrame(Shader * shader)
	:shader(shader)
{
	buffer = new ConstantBuffer(&bufferDesc, sizeof(BufferDesc));
	sBuffer = shader->AsConstantBuffer("CB_PerFrame");

	
	/*spotLightBuffer = new ConstantBuffer(&spotLightDesc, sizeof(SpotLightDesc));
	sSpotLightBuffer = shader->AsConstantBuffer("CB_SpotLights");

	capsuleLightBuffer = new ConstantBuffer(&capsuleLightDesc, sizeof(CapsuleLightDesc));
	sCapsuleLightBuffer = shader->AsConstantBuffer("CB_CapsuleLights");*/
}

PerFrame::~PerFrame()
{
	SafeDelete(buffer);
	
	//SafeDelete(spotLightBuffer);
	//SafeDelete(capsuleLightBuffer);
}

void PerFrame::Update()
{
	
	bufferDesc.VP = Context::Get()->View()*Context::Get()->Projection();
	
	
		
	//planeNormals = Context::Get()->PlaneNormals();

	/*bufferDesc.FrustumNormals[0] = planeNormals[0];
	bufferDesc.FrustumNormals[1] = planeNormals[1];
	bufferDesc.FrustumNormals[2] = planeNormals[2];
	bufferDesc.FrustumNormals[3] = planeNormals[3];*/
}

void PerFrame::Render(Vector3 Pos)
{
	
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());
}
