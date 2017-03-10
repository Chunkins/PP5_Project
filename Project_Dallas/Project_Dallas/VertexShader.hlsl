cbuffer cbPerObject
{
	float4x4 WVP;
};

//cbuffer viewProj : register(b1)
//{
//	
//}

struct Input
{
	float4 pos : POSITION;
	float4 normal : NORMAL;
	float4 uv : UV;
};

struct Output
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

Output main(Input _in)
{
	Output ret;
	float4 pos = _in.pos;
	pos = mul(pos, WVP);
	ret.pos = pos;
	ret.color = _in.normal;
	return ret;
}