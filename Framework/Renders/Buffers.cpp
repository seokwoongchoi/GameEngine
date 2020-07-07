#include "Framework.h"
#include "Buffers.h"

ConstantBuffer::ConstantBuffer(void * data, uint dataSize,bool bImute)
	:data(data), dataSize(dataSize)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

	desc.ByteWidth = dataSize;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;


	desc.Usage = bImute ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = bImute ? 0 : D3D11_CPU_ACCESS_WRITE;


	Check(D3D::GetDevice()->CreateBuffer(&desc, NULL//초기화 데이터없음
		, &buffer));


}


ConstantBuffer::~ConstantBuffer()
{
	SafeRelease(buffer);
}

void ConstantBuffer::Apply()
{
	
	D3D::GetDC()->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, data, dataSize);
		//vRam의 어느 부분에 공간을 확보한다. 
		//byteWidth만큼 공간을 확보한다.
		//그 버퍼를 쓰기용도로 Map을 걸었다. 
		//그 주소의 공간이 subResource에 리턴된다.
		//pData가 그 공간의 시작주소를 가지고있다.
		//memcpy로 그공간에 데이터를 복사해서 넣었다.
	}
	D3D::GetDC()->Unmap(buffer, 0);//락안풀면 gpu에서 접근불가.
}


/////////////////////////////////////////////////////////////////////////////
VertexBuffer::VertexBuffer(void * data, uint count, uint stride, uint slot, bool bCpuWrite, bool bGpuWrite)
	:data(data),
	stride(stride),
	slot(slot),
	bCpuWrite(bCpuWrite),
	bGpuWrite(bGpuWrite),
	count(count)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

	desc.ByteWidth = stride * count;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	if (bCpuWrite == false && bGpuWrite == false)
	{
		desc.Usage = D3D11_USAGE_IMMUTABLE;
	}
	else if (bCpuWrite == true && bGpuWrite == false)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (bCpuWrite == false && bGpuWrite == true)
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
	}
	else
	{
		assert(false);
	}

	D3D11_SUBRESOURCE_DATA subResource = { 0 };//포인터 맴버를 가지고있지않으면
	//이런식으로 초기화가능
	subResource.pSysMem = data;

	Check(D3D::GetDevice()->CreateBuffer(&desc, data != NULL ? &subResource : NULL, &buffer));

}
VertexBuffer::~VertexBuffer()
{
	SafeRelease(buffer);
}

void VertexBuffer::Render()
{
	
		uint offset = 0;

		D3D::GetDC()->IASetVertexBuffers(slot, 1, &buffer, &stride, &offset);
	
}



/////////////////////////////////////////////////////////////////////////////
IndexBuffer::IndexBuffer(void * data, uint count)
	:data(data), count(count)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

	desc.ByteWidth = sizeof(uint) * count;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.Usage = D3D11_USAGE_IMMUTABLE;


	D3D11_SUBRESOURCE_DATA subResource = { 0 };//포인터 맴버를 가지고있지않으면
	//이런식으로 초기화가능
	subResource.pSysMem = data;

	Check(D3D::GetDevice()->CreateBuffer(&desc, &subResource, &buffer));

}


IndexBuffer::~IndexBuffer()
{
	SafeRelease(buffer);
}

void IndexBuffer::Render()
{
		uint offset = 0;

		D3D::GetDC()->IASetIndexBuffer(buffer, DXGI_FORMAT_R32_UINT, 0);
}


///////////////////////////////////////////////////////////////////////
CsResource::CsResource()
	:input(nullptr),srv(nullptr),output(nullptr),uav(nullptr),result(nullptr)
{
	
}

CsResource::~CsResource()
{
	SafeRelease(input);
	SafeRelease(srv);

	SafeRelease(output);
	SafeRelease(uav);

	SafeRelease(result);
}

void CsResource::Copy(void * data, uint size)
{
	D3D::GetDC()->CopyResource(result, output);

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(result, 0, D3D11_MAP_READ, 0, &subResource);
	{
		memcpy(data, subResource.pData, size);
	}
	D3D::GetDC()->Unmap(result, 0);
}

void CsResource::CreateBuffer()
{
	CreateInput();
	CreateSRV();

	CreateOutput();
	CreateUAV();

	CreateResult();
}
////////////////////////////////////////////////////////////////////////////////

RawBuffer::RawBuffer(void * inputData, UINT byteWidth)
	:CsResource(), inputData(inputData),byteWidth(byteWidth)
{
	CreateBuffer();
}

RawBuffer::~RawBuffer()
{
}

void RawBuffer::CreateInput()
{
	ID3D11Buffer* buffer = nullptr;
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	
	desc.ByteWidth = byteWidth;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	
	D3D11_SUBRESOURCE_DATA subResource = { 0 };
	subResource .pSysMem = inputData;
	D3D::GetDevice()->CreateBuffer(&desc, inputData != nullptr ? &subResource : nullptr, &buffer);

	input = (ID3D11Resource*)buffer;
}

void RawBuffer::CreateSRV()
{
	ID3D11Buffer* buffer = (ID3D11Buffer*)input;
	
	D3D11_BUFFER_DESC desc;
	buffer->GetDesc(&desc);
	
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;//r32=4byte;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.BufferEx.Flags=D3D11_BUFFEREX_SRV_FLAG_RAW;
	srvDesc.Buffer.NumElements = desc.ByteWidth / 4;
	Check(D3D::GetDevice()->CreateShaderResourceView(buffer, &srvDesc , &srv));
}

void RawBuffer::CreateOutput()
{
	ID3D11Buffer* buffer = nullptr;
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

	desc.ByteWidth = byteWidth;
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

	
	D3D::GetDevice()->CreateBuffer(&desc, nullptr, &buffer);

	output = (ID3D11Resource*)buffer;
}

void RawBuffer::CreateUAV()
{
	ID3D11Buffer* buffer = (ID3D11Buffer*)output;

	D3D11_BUFFER_DESC desc;
	buffer->GetDesc(&desc);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;//r32=4byte;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
	uavDesc.Buffer.NumElements = desc.ByteWidth / 4;
	Check(D3D::GetDevice()->CreateUnorderedAccessView(buffer, &uavDesc, &uav));
}

void RawBuffer::CreateResult()
{
	ID3D11Buffer* buffer;
	D3D11_BUFFER_DESC desc;
	((ID3D11Buffer*)output)->GetDesc(&desc);
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;
	Check(D3D::GetDevice()->CreateBuffer(&desc, nullptr, &buffer));
	result = (ID3D11Resource*)buffer;

}
//////////////////////////////////////////////////////////////////////////////////

StructuredBuffer::StructuredBuffer(void * inputData, UINT stride, UINT count, UINT outputStride, UINT outputCount)
	: CsResource()
	, inputData(inputData), stride(stride), count(count)
	, outputStride(outputStride), outputCount(outputCount)
{
	if (outputStride == 0)
		this->outputStride = stride;

	if (outputCount == 0)
		this->outputCount = count;

	CreateBuffer();
}

StructuredBuffer::~StructuredBuffer()
{
}

void StructuredBuffer::CreateInput()
{
	ID3D11Buffer* buffer = NULL;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

	desc.ByteWidth = InputByteWidth();
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = stride;

	D3D11_SUBRESOURCE_DATA subResource = { 0 };
	subResource.pSysMem = inputData;

	Check(D3D::GetDevice()->CreateBuffer(&desc, inputData != NULL ? &subResource : NULL, &buffer));

	input = (ID3D11Resource *)buffer;
}

void StructuredBuffer::CreateSRV()
{
	ID3D11Buffer* buffer = (ID3D11Buffer *)input;

	D3D11_BUFFER_DESC desc;
	buffer->GetDesc(&desc);


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.Buffer.NumElements = desc.ByteWidth / desc.StructureByteStride;

	Check(D3D::GetDevice()->CreateShaderResourceView(buffer, &srvDesc, &srv));
}

void StructuredBuffer::CreateOutput()
{
	ID3D11Buffer* buffer = NULL;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

	desc.ByteWidth = OutputByteWidth();
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = outputStride;

	Check(D3D::GetDevice()->CreateBuffer(&desc, NULL, &buffer));

	output = (ID3D11Resource *)buffer;
}

void StructuredBuffer::CreateUAV()
{
	ID3D11Buffer* buffer = (ID3D11Buffer *)output;

	D3D11_BUFFER_DESC desc;
	buffer->GetDesc(&desc);


	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = desc.ByteWidth / desc.StructureByteStride;

	Check(D3D::GetDevice()->CreateUnorderedAccessView(buffer, &uavDesc, &uav));
}

void StructuredBuffer::CreateResult()
{
	ID3D11Buffer* buffer;

	D3D11_BUFFER_DESC desc;
	((ID3D11Buffer *)output)->GetDesc(&desc);
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;

	Check(D3D::GetDevice()->CreateBuffer(&desc, NULL, &buffer));

	result = (ID3D11Buffer *)buffer;
}