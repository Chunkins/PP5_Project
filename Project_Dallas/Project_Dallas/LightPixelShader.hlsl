Texture2D shaderTexture;
Texture2D normalTex : register (t2);
SamplerState SampleType;
cbuffer LightBuffer
{
	float4 diffuseColor;
	float3 lightDirection;
	float padding;
	float4 lightType;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 normal : NORMAL;
	float4 tex : UV;
	float3 worldPos : POSITION;
};

float4 main(PixelInputType input) : SV_TARGET
{

	float4 textureColor;
	float3 lightDir;
	float3 coneDir = float3(lightDirection.x, -1.5f, -0.1f);
	float3 spotLightPosition = float3(-1, 5, -1);
	float3 pointLightPosition = float3(-3, 1, -1);	
	float lightIntensity;
	float lightRatio;
	float spotFactor;
	float surfaceRatio;
	float4 color;
	float4 color1;
	float coneRatio = 0.98;
	float alpha;
	float4 attenuation;

float4 normal = shaderTexture.Sample(SampleType, input.tex.xy);
input.normal = float4(normalize(input.normal.xyz + normal.xyz), 1);
textureColor = shaderTexture.Sample(SampleType, input.tex.xy);

float3 cameraPos = lightType.yzw;
float3 toCamera = normalize(cameraPos - input.worldPos);
float3 toLight = normalize(lightDirection - input.worldPos);
float3 vecterReflect = reflect(-toLight, input.normal);
float sp = clamp(dot(vecterReflect, toCamera),0, 1);
sp = pow(sp, 128);
float3 spec = diffuseColor * sp * 1;

//Directional Light
if (lightType.x == 1)
{
	alpha = textureColor.w;
	lightDir = -lightDirection;
	lightRatio = clamp(dot(normalize(lightDir), input.normal), 0, 1);
	color = lightRatio * diffuseColor * textureColor;
}

//Point Light
if (lightType.x == 2)
{
	alpha = textureColor.w;
	lightDir = normalize(lightDirection - input.worldPos);
	lightRatio = clamp(dot(input.normal, lightDir), 0, 1);
	color = diffuseColor * textureColor * lightRatio;
}

//Spot Light
if (lightType.x == 3)
{
	alpha = textureColor.w;
	lightDir = normalize(spotLightPosition - input.worldPos);
	surfaceRatio = clamp(dot(-lightDir, normalize(coneDir)), 0, 1);
	spotFactor = (surfaceRatio > coneRatio) ? 1 : 0;
	lightRatio = clamp(dot(normalize(lightDir), input.normal), 0, 1);
	color = spotFactor * lightRatio * diffuseColor * textureColor;
}

//Ambient Light
if (lightType.x == 4)
{
	alpha = textureColor.w;
	color = diffuseColor * textureColor * 0.8f;
}

return float4(saturate(color.xyz + spec), alpha);


//
//	float4 textureColor;
//
//
//// Sample the pixel color from the texture using the sampler at this texture coordinate location.
//textureColor = shaderTexture.Sample(SampleType, input.tex.xy);
//
//return textureColor;
}
