struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
	float3 normal : TEXCOORD1;
};

SamplerState pointClampSampler : register(s0);

Texture2D<float4> diffuse : register(t0);

float4 main(VS_OUTPUT input) : SV_TARGET
{    
    return diffuse.Sample(pointClampSampler, input.texcoord);
}