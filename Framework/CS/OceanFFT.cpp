#include "Framework.h"
#include "OceanFFT.h"

OceanFFT::OceanFFT(uint slices)
{
	shader = new Shader(L"Deferred/fft_512x512_c2c.fxo");
	this->slices = slices;
	// Temp buffer
	D3D11_BUFFER_DESC buf_desc;
	buf_desc.ByteWidth = sizeof(float) * 2 * (512 * slices) * 512;
	buf_desc.Usage = D3D11_USAGE_DEFAULT;
	buf_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	buf_desc.CPUAccessFlags = 0;
	buf_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	buf_desc.StructureByteStride = sizeof(float) * 2;

	Check(D3D::GetDevice()->CreateBuffer(&buf_desc, NULL, &pBuffer_Tmp));
	

	// Temp undordered access view
	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
	uav_desc.Format = DXGI_FORMAT_UNKNOWN;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uav_desc.Buffer.FirstElement = 0;
	uav_desc.Buffer.NumElements = (512 * slices) * 512;
	uav_desc.Buffer.Flags = 0;

	Check(D3D::GetDevice()->CreateUnorderedAccessView(pBuffer_Tmp, &uav_desc, &pUAV_Tmp));
	
	// Temp shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	srv_desc.Format = DXGI_FORMAT_UNKNOWN;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srv_desc.Buffer.FirstElement = 0;
	srv_desc.Buffer.NumElements = (512 * slices) * 512;

	Check(D3D::GetDevice()->CreateShaderResourceView(pBuffer_Tmp, &srv_desc, &pSRV_Tmp));
	
	//create_cbuffers_512x512(slices);
	perframeBuffer = new ConstantBuffer(&perframeCB, sizeof(CB_6PerFrame));
	sPerframeBuffer = shader->AsConstantBuffer("CB_Perframe");
}

OceanFFT::~OceanFFT()
{
	SafeRelease(pSRV_Tmp);
	SafeRelease(pUAV_Tmp);
	SafeRelease(pBuffer_Tmp);
}
void OceanFFT::fft_512x512_c2c(ID3D11UnorderedAccessView * pUAV_Dst, ID3D11ShaderResourceView * pSRV_Dst, ID3D11ShaderResourceView * pSRV_Src)
{
	const UINT thread_count = slices * (512 * 512) / 8;
	UINT istride = 512 * 512 / 8;

	sPerframeBuffer->SetConstantBuffer(RadixCBuffers[0]->Buffer());
	radix008A(pUAV_Tmp, pSRV_Src, thread_count, istride);


	istride /= 8;
	sPerframeBuffer->SetConstantBuffer(RadixCBuffers[1]->Buffer());
	radix008A(pUAV_Dst, pSRV_Tmp, thread_count, istride);

	istride /= 8;
	sPerframeBuffer->SetConstantBuffer(RadixCBuffers[2]->Buffer());
	radix008A(pUAV_Tmp, pSRV_Dst, thread_count, istride);


	istride /= 8;
	sPerframeBuffer->SetConstantBuffer(RadixCBuffers[3]->Buffer());
	radix008A(pUAV_Dst, pSRV_Tmp, thread_count, istride);


	istride /= 8;
	sPerframeBuffer->SetConstantBuffer(RadixCBuffers[4]->Buffer());
	radix008A(pUAV_Tmp, pSRV_Dst, thread_count, istride);


	istride /= 8;
	sPerframeBuffer->SetConstantBuffer(RadixCBuffers[5]->Buffer());
	radix008A(pUAV_Dst, pSRV_Tmp, thread_count, istride);

}

void OceanFFT::create_cbuffers_512x512(UINT slices)
{
	// Create 6 cbuffers for 512x512 transform.
	// 512*512 = 8^6
	// Buffer 0
	const UINT thread_count = slices * (512 * 512) / 8;
	UINT ostride = 512 * 512 / 8;
	UINT istride = ostride;
	double phase_base = -TWO_PI / (512.0 * 512.0);

	for (int i = 0; i < 6; i++, istride /= 8, phase_base *= 8.0)
	{
		if (i == 3)
			ostride /= 512;
		CB_Descs[i] = { thread_count, ostride, istride, 512, (float)phase_base };
		if (i >= 3)
			CB_Descs[i].pstride = 1;
		RadixCBuffers[i] = new ConstantBuffer(&CB_Descs[i], sizeof(CB_Structure));
		assert(RadixCBuffers[i]->Buffer());
	}
}

void OceanFFT::radix008A(ID3D11UnorderedAccessView * pUAV_Dst, ID3D11ShaderResourceView * pSRV_Src, UINT thread_count, UINT istride)
{
	// Setup execution configuration
	UINT grid = thread_count / COHERENCY_GRANULARITY;


	shader->AsSRV("g_SrcData")->SetResource(pSRV_Src);
	// Output

	shader->AsUAV("g_DstData")->SetUnorderedAccessView(pUAV_Dst);

	if (istride > 1)
		shader->Dispatch(0, 0, grid, 1, 1);
	else
		shader->Dispatch(0, 1, grid, 1, 1);


}
