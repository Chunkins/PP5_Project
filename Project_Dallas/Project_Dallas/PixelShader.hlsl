struct Input
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

float4 main(Input _pix) : SV_TARGET
{
	return _pix.color;
}