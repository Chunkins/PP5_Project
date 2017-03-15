Texture2D shaderTexture;
SamplerState SampleType;

cbuffer cbPerObject
{
	float4x4 WVP;
};

cbuffer LightBuffer
{
	float4 diffuseColor;
	float3 lightDirection;
	float padding;
	float4 lightType;
};


struct VOut
{
	float4 position : SV_POSITION;
	float4 normal : NORMAL;
	float4 uv : UV;
};

VOut VShader(float4 position : POSITION, float4 normal : NORMAL, float4 uv : UV)
{
	VOut output;

	output.position = mul(position, WVP);
	output.normal = normal;
	output.uv = uv;

	return output;
}

float4 PShader(float4 position : SV_POSITION, float4 normal : NORMAL, float4 uv : UV) : SV_TARGET
{
	float4 textureColor;
	

// Sample the pixel color from the texture using the sampler at this texture coordinate location.
textureColor = shaderTexture.Sample(SampleType, uv.xy);

return textureColor;

}