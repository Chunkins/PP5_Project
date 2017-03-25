
struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float3 uv : UV;
};

textureCUBE env;
SamplerState envFilter;

float4 main(PixelShaderInput input) : SV_TARGET
{
	return env.Sample(envFilter, input.uv);
}


