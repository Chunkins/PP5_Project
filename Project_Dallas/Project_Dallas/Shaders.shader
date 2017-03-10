Texture2D shaderTexture;
SamplerState SampleType;

cbuffer cbPerObject
{
	float4x4 WVP;
};

struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VOut VShader(float4 position : POSITION, float4 color : COLOR)
{
	VOut output;

	output.position = mul(position, WVP);
	output.color = color;

	return output;
}


float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
	float4 textureColor;


// Sample the pixel color from the texture using the sampler at this texture coordinate location.
textureColor = shaderTexture.Sample(SampleType, color.xy);

return textureColor;

}