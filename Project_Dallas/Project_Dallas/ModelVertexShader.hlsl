
cbuffer cbPerObject
{
	float4x4 WVP;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float4 pos : POSITION;
	float4 normal : NORMAL;
	float4 uv : UV;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 normal : NORMAL;
	float4 uv : UV;
	float3 worldPos : POSITION;

};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = input.pos;

	pos = mul(pos, WVP);
	output.worldPos = pos;
	output.pos = pos;

	output.normal = input.normal;
	output.uv = input.uv;

	return output;
}
