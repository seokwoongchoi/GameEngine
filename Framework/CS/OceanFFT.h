#pragma once
#define TWO_PI 6.283185307179586476925286766559
#define COHERENCY_GRANULARITY 128
class OceanFFT
{
public:
	OceanFFT( uint slices);
	~OceanFFT();

	void fft_512x512_c2c(ID3D11UnorderedAccessView* pUAV_Dst,
		ID3D11ShaderResourceView* pSRV_Dst,
		ID3D11ShaderResourceView* pSRV_Src);
	void create_cbuffers_512x512( UINT slices);
	void radix008A(ID3D11UnorderedAccessView* pUAV_Dst,
		ID3D11ShaderResourceView* pSRV_Src,
		UINT thread_count,
		UINT istride);

	

	

private:
    struct CB_6PerFrame
    {
    	UINT thread_count;
    	UINT ostride;
    	UINT istride;
    	UINT pstride;
    	float phase_base;
		Vector3 Padding;
    };

	struct CB_Structure
	{
		UINT thread_count;
		UINT ostride;
		UINT istride;
		UINT pstride;
		float phase_base;
		Vector3 Padding;
	};

	//CB_Structure cb_sturct[6];

	CB_Structure CB_Descs[6];
	ConstantBuffer* RadixCBuffers[6];

    CB_6PerFrame perframeCB;


	ConstantBuffer* perframeBuffer;
	ID3DX11EffectConstantBuffer* sPerframeBuffer;


private:
	class Shader* shader;
	// More than one array can be transformed at same time
	UINT slices;

	

	// Temporary buffers
	ID3D11Buffer* pBuffer_Tmp;
	ID3D11UnorderedAccessView* pUAV_Tmp;
	ID3D11ShaderResourceView* pSRV_Tmp;
	ID3D11Buffer* pRadix008A_CB[6];

};
