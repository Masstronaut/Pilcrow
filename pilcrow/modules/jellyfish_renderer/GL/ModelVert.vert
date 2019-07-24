struct VS_INPUT
{
	float3 pos : SV_POSITION;
	float4 color : COLOR;
	float3 texcoord : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
	float3 normal : TEXCOORD1;
};

cbuffer WORLD_CONSTANT : register(b0)
{
	matrix modelViewProj;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
    output.texcoord = input.texcoord;
    
	output.position = mul(modelViewProj, float4(input.pos, 1.0));

    output.normal = input.normal;

	return output;
}
