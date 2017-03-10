Texture2D shaderTexture;
SamplerState SampleType;

struct Input
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

float4 main(Input _pix) : SV_TARGET
{
//	float4 textureColor;
//
//
//// Sample the pixel color from the texture using the sampler at this texture coordinate location.
//textureColor = shaderTexture.Sample(SampleType, input.color.xy);
//
//return textureColor;
	//return float4(0.f,0.f,1.0,0.f);
	return _pix.color;
}