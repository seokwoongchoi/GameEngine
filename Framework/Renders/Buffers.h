#pragma once
class ConstantBuffer
{
public:
	ConstantBuffer(void* data, uint dataSize,bool bImute=false);
	~ConstantBuffer();

	ConstantBuffer(const ConstantBuffer& rhs) = delete;
	ConstantBuffer& operator=(const ConstantBuffer& rhs) = delete;

	inline ID3D11Buffer* Buffer() { return buffer; }

	void Apply();

private:
	ID3D11Buffer* buffer;

	void* data;

	uint dataSize;
	D3D11_MAPPED_SUBRESOURCE subResource;

};
//////////////////////////////////////////////////////////////////////////
class VertexBuffer
{
public:
	VertexBuffer(void* data, uint count, uint stride, uint slot = 0, bool bCpuWrite = false, bool bGpuWrite = false);
	~VertexBuffer();

	VertexBuffer(const VertexBuffer& rhs) = delete;
	VertexBuffer& operator=(const VertexBuffer& rhs) = delete;

	uint Count() { return count; }
	uint Stride() { return stride; }
	inline ID3D11Buffer* Buffer() { return buffer; }
	void Render();
	

private:
	ID3D11Buffer* buffer;

	void* data;

	uint count;
	uint stride;
	uint slot;

	bool bCpuWrite;
	bool bGpuWrite;
};

////////////////////////////////////////////////////////////////////////////
class IndexBuffer
{
public:
	IndexBuffer(void* data, uint count);
	~IndexBuffer();
	IndexBuffer(const IndexBuffer& rhs) = delete;
	IndexBuffer& operator=(const IndexBuffer& rhs) = delete;
	uint Count() { return count; }

	ID3D11Buffer* Buffer() { return buffer; }
	void Render();
	

private:
	ID3D11Buffer* buffer;

	void* data;

	uint count;

};

////////////////////////////////////////////////////////////////////////////
class CsResource
{
public:
	CsResource();
	virtual ~CsResource();
	
	inline ID3D11ShaderResourceView* SRV() { return srv; }
	inline ID3D11UnorderedAccessView* UAV() { return uav; }

	void Copy(void* data, uint size);

protected:
	virtual void CreateInput() {}
	virtual void CreateSRV() {}

	virtual void CreateOutput() {}
	virtual void CreateUAV() {}

	virtual void CreateResult() {}

	void CreateBuffer();
	
protected:
	ID3D11Resource* input; //buffer와 texture의 상위 객체
	ID3D11ShaderResourceView* srv;

	ID3D11Resource* output;
	ID3D11UnorderedAccessView* uav; //어느곳이든 이걸로 우리가 원하는 엑세스를뽑아올수있다.
	
	ID3D11Resource* result;
	
};
//////////////////////////////////////////////////////////////////////////

class RawBuffer :public CsResource
{
public:
	RawBuffer(void* inputData, UINT byteWidth);
	~RawBuffer();

	

	UINT ByteWidth() { return byteWidth; }

private:
	void CreateInput() override;
	void CreateSRV() override;

	void CreateOutput() override;
	void CreateUAV() override;

	void CreateResult() override;

private:
	void* inputData;
	uint byteWidth;

	
};
////////////////////////////////////////////////////////////////////////////////
class StructuredBuffer : public CsResource
{
public:
	StructuredBuffer(void* inputData, UINT stride, UINT count, UINT outputStride = 0, UINT outputCount = 0);
	~StructuredBuffer();

	UINT InputByteWidth() { return stride * count; }
	UINT OutputByteWidth() { return outputStride * outputCount; }

private:
	void CreateInput() override;
	void CreateSRV() override;

	void CreateOutput() override;
	void CreateUAV() override;

	void CreateResult() override;

private:
	void* inputData;

	UINT stride;
	UINT count;

	UINT outputStride;
	UINT outputCount;
};