#pragma pack_matrix( row_major )
cbuffer cbPerObject: register(b0)
{
	float4x4 WVP;
};

cbuffer bones: register(b1) {
	float4x4 boneTransforms[40];
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	int4 boneCt : BONECOUNT;
	float4 pos : POSITION;
	float4 normal : NORMAL;
	float4 uv : UV;
	float4 blendWt : BLENDWEIGHT;
	int4 blendInd : BLENDINDICES;
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
	float3 norm = float3(0.0f, 0.0f, 0.0f);
	float lastWeight = 0.0f;
	input.normal = normalize(input.normal);
	if (input.boneCt.x > 4)
		input.boneCt.x = 4;
	if (input.boneCt.x > 0) {
		pos = float4(0.0f, 0.0f, 0.0f, 1.0f);
		int i = -1; while (++i < input.boneCt.x) {
			pos +=  mul( boneTransforms[input.blendInd[i]], input.pos)*input.blendWt[i].x;
			//norm += input.blendWt[i] * mul(input.normal, boneTransforms[input.blendInd[i]]);
		}
		pos.w = 1.f;
		norm = normalize(norm);
		norm = mul(norm, WVP);
	}
	pos = mul(pos, WVP);

	output.worldPos = pos;
	output.pos = pos;

	output.normal = input.normal;
	output.uv = input.uv;

	return output;
}
